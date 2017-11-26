#include <assert.h>
#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <time.h>
#define DEBUG 0

unsigned int **map;
int numNode, numEdge;
MPI_Request sendReq;
unsigned int *minD;
int commSize, rank;
int *ans;
int neighbor[2000],numNeighbor=0;

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
    
    /* MPI init */
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &commSize);
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
    
    int flag, updated=0, allUpdated=1;
    MPI_Request request;
    MPI_Status status;
    unsigned int dis;
    for(int v=0;v<numNode;++v)
    {
        allUpdated = 1;
        while(allUpdated)
        {
            updated = 0;
            unsigned int sendData = 0;
            for (int i=0;i<numNeighbor;++i)
            {
                sendData = minD[v]+map[rank][neighbor[i]];
                MPI_Isend( &sendData, 1, MPI_UNSIGNED, neighbor[i], 0, MPI_COMM_WORLD, &sendReq);
            }
            for (int i=0;i<numNeighbor;++i)
            {
                MPI_Recv(&dis, 1, MPI_UNSIGNED, neighbor[i], 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                #if DEBUG
                //if(rank==49 && update[0]==12)
                printf("[rank %d] recv %u from %d v=%d minD[v]=%u \n", rank, dis, neighbor[i], v, minD[v]);
                #endif

                if(dis<minD[v])
                {
                    updated = 1;
                    minD[v] = dis;
                }
            }
            MPI_Allreduce(&updated, &allUpdated, 1, MPI_INT, MPI_BOR, MPI_COMM_WORLD);
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
    {
        ans = (int*) malloc(sizeof(int)*numNode*numNode);
        MPI_Gather (minD, numNode, MPI_INT, ans, numNode, MPI_INT, 0, MPI_COMM_WORLD);
        printMap(outputFile);
        #if DEBUG
        printMap(stdout);
        #endif
    }
    else
        MPI_Gather (minD, numNode, MPI_INT, ans, numNode, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Finalize();
}
