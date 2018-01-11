#include <assert.h>
#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <time.h>
#define DEBUG 0
#define TIME 0
unsigned int **map;
int numNode, numEdge;
MPI_Request sendReq;
unsigned int *minD;
int commSize, rank;
int *ans;
int neighbor[2000],numNeighbor=0;
#if TIME
double commt=0, cput=0, iot=0, synct=0, sendt=0, recvt=0;
double allcommt=0, allcput=0, alliot=0, allsynct=0, allsendt=0, allrecvt=0;
double st1, st2, et1, et2, runST, runET;
int round = 0;
#endif

void printMap(FILE* file)
{
    int bound = numNode*numNode;
    for(int i=0;i<bound;++i)
    {
        int temp = ans[i];
        if(temp==2147483647)
            temp = 0;
        fprintf(file, "%d ", temp);
        if((i+1)%numNode==0)
            fprintf(file, "\n");
    }
}

int main(int argc, char** argv) {
    /* argument parsing */
    char *inputFileName = argv[1];
    char *outputFileName = argv[2];
    int numThreads = strtol(argv[3], 0, 10);
    FILE* inputFile = fopen (inputFileName,"r");
    FILE* outputFile = fopen (outputFileName,"w");
    
    /* MPI init */
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &commSize);

    #if TIME
    st1 = MPI_Wtime();
    runST = MPI_Wtime();
    #endif
    
    //malloc map
    fscanf(inputFile, "%d %d", &numNode, &numEdge);
    map = (unsigned int**)malloc(numNode*sizeof(unsigned int*));
    for(int i=0;i<numNode;++i)
    {
        map[i] = (unsigned int*)malloc(numNode*sizeof(unsigned int));
        for(int j=0;j<numNode;++j)
            map[i][j]=2147483647;
    }
    minD = (unsigned int*)malloc(sizeof(unsigned int)*numNode);
    for(int i=0;i<numNode;++i)
        minD[i] = 2147483647;
    minD[rank] = 0;
    
    
    //init map
    for(int k=0;k<numEdge;++k)
    {
        int i, j, w;
        fscanf(inputFile, "%d %d %d", &i, &j, &w);
        map[i][j] = w;
        map[j][i] = w;
        if(i==rank)
            neighbor[numNeighbor++] = j;
        else if (j==rank)
            neighbor[numNeighbor++] = i;
    }
    
    #if TIME
    et1 = MPI_Wtime();
    iot += et1-st1;
    #endif

    int flag, updated=0, allUpdated=1;
    MPI_Request *request = (MPI_Request*)malloc(numNeighbor * sizeof(MPI_Request));
    MPI_Status *status =(MPI_Status*)malloc(numNeighbor * sizeof(MPI_Status));
    unsigned int dis[800];
    for(int v=0;v<numNode;++v)
    {
        allUpdated = 1;
        while(allUpdated)
        {
            updated = 0;
            for( int i=0;i<numNeighbor;++i)
                MPI_Irecv(&dis[i], 1, MPI_UNSIGNED, neighbor[i], MPI_ANY_TAG, MPI_COMM_WORLD, &request[i]);

            unsigned int sendData = 0;
            for (int i=0;i<numNeighbor;++i)
            {
                sendData = minD[v]+map[rank][neighbor[i]];
                #if TIME
                ++round;
                st1 = MPI_Wtime();
                #endif
                MPI_Isend( &sendData, 1, MPI_UNSIGNED, neighbor[i], 0, MPI_COMM_WORLD, &sendReq);
                MPI_Wait(&sendReq, MPI_STATUS_IGNORE);
                #if TIME
                et1 = MPI_Wtime();
                commt += et1-st1;
                sendt += et1-st1;
                #endif
            }
            #if TIME
            st1 = MPI_Wtime();
            #endif
            MPI_Waitall(numNeighbor, request, status);
            #if TIME
            et1 = MPI_Wtime();
            commt += et1-st1;
            recvt += et1-st1;
            #endif
            for (int i=0;i<numNeighbor;++i)
            {
                /*
                #if TIME
                st1 = MPI_Wtime();
                #endif
                MPI_Recv(&dis, 1, MPI_UNSIGNED, neighbor[i], 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                #if TIME
                et1 = MPI_Wtime();
                commt += et1-st1;
                recvt += et1-st1;
                #endif
                */
                #if DEBUG
                //if(rank==49 && update[0]==12)
                printf("[rank %d] recv %u from %d v=%d minD[v]=%u \n", rank, dis, neighbor[i], v, minD[v]);
                #endif

                #if TIME
                st1 = MPI_Wtime();
                #endif

                if(dis[i]<minD[v])
                {
                    updated = 1;
                    minD[v] = dis[i];
                }
                #if TIME
                et1 = MPI_Wtime();
                cput += et1-st1;
                #endif
            }
            #if TIME
            st1 = MPI_Wtime();
            #endif
            MPI_Allreduce(&updated, &allUpdated, 1, MPI_INT, MPI_BOR, MPI_COMM_WORLD);
            #if TIME
            et1 = MPI_Wtime();
            synct += et1-st1;
            #endif
        }
    }
    
    #if DEBUG
    if(1)
    {
        printf("[rank %d] final minD\n", rank);
        for(int i=0;i<numNode;++i)
            printf("-%d- ",minD[i]);
        printf("\n");
    }
    #endif
    
    if(rank==0)
        ans = (int*) malloc(sizeof(int)*numNode*numNode);

    #if TIME
    st1 = MPI_Wtime();
    #endif
    MPI_Gather (minD, numNode, MPI_INT, ans, numNode, MPI_INT, 0, MPI_COMM_WORLD);
    #if TIME
    et1 = MPI_Wtime();
    commt += et1-st1;
    st1 = MPI_Wtime();
    #endif
    if(rank==0)
    {
        printMap(outputFile);
        #if DEBUG
        printMap(stdout);
        #endif
    }
    #if TIME
    et1 = MPI_Wtime();
    iot += et1-st1;
    MPI_Reduce(&cput, &allcput, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&commt, &allcommt, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&sendt, &allsendt, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&recvt, &allrecvt, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&iot, &alliot, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&synct, &allsynct, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    if(rank==0)
    {
        printf("iot time\t%lf\n", alliot);
        printf("cput time\t%lf\n", allcput/commSize);
        printf("commt time\t%lf\n", allcommt/commSize);
        printf("send time\t%lf\n", allsendt);
        printf("recv time\t%lf\n", allrecvt);
        printf("synct time\t%lf\n", allsynct);
        printf("round time\t%d\n", round);
        runET = MPI_Wtime();
        printf("Runtime = %lf\n", runET-runST);
    }

    #endif

    MPI_Finalize();
}
