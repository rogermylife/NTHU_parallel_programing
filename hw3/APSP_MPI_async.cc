#include <assert.h>
#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <time.h>
#include <vector>
#define TIME 0
#define DEBUG 0
#define BALL_TAG 11
#define BALL_ACK_TAG 12
#define DIS_TAG 13
#define DIS_ACK_TAG 14
#define BLACK 2
#define WHITE 3
#define ROUND 100
struct Job{
    int source;
    int dis;
    int neighbor;
    int round;
    MPI_Request *req;
};

unsigned int **map;
int numNode, numEdge;
MPI_Request sendReq[800][800];
int *minD;
int commSize, rank;
int *ans;
//std::vector<Job> jobs;
int color,source;
int flag, updated=0, allUpdated=0,cycle=0;
int ballHold=1, recvInited=1;
int updates[800];
int recvData[2], sendData[2];
#if TIME
double st1, et1, runST, runET, st2, et2;
double iot=0, cput=0, synct=0, commt=0;
double alliot=0, allcput=0, allsynct=0, allcommt=0;
#endif

#if DEBUG
FILE* logFile;
#endif

/*
void eraseJob(std::vector<Job>& jobs, int source, int dis, int neighbor)
{
    for (int i=0;i<jobs.size();++i)
    {
        if(jobs[i].source == source && jobs[i].dis >= dis && jobs[i].neighbor == neighbor)
        {
            free(jobs[i].req);
            jobs.erase(jobs.begin()+i);
            break;
        }
    }
}
*/

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
void broadcast(int start, int pred)
{
    for(int i=0;i<numNode;++i)
    {
        if(map[rank][i]==2147483647 || rank==i || start==i || pred==i)
            continue;
        //sendData[2] = {-1,-1};
        sendData[0] = start;
        sendData[1] = minD[start]+map[rank][i];
        #if DEBUG
        //if(i==49 && start==12)
        //printf("[rank %d] send to %d from %d dis %d\n", rank, i, start, sendData[1]);
        #endif
        //Job temp = { .source = start, .dis=sendData[1], .neighbor = i, .round = 0};
        //temp.req = (MPI_Request*)malloc(sizeof(MPI_Request));
        //eraseJob(jobs, start, -1, i);
        MPI_Isend(sendData, 2, MPI_INT, i, DIS_TAG, MPI_COMM_WORLD, &sendReq[start][i]);
        //MPI_Status sendStatus;
        MPI_Wait(&sendReq[start][i], MPI_STATUS_IGNORE);
        //jobs.push_back(temp);
    }
}

int main(int argc, char** argv) {
    /* argument parsing */
    char *inputFileName = argv[1];
    char *outputFileName = argv[2];
    int numThreads = strtol(argv[3], 0, 10);
    FILE* inputFile;
    FILE* outputFile;
    
    /* MPI init */
    printf("A");
    fflush(stdout);
    MPI_Init(&argc, &argv);
    inputFile = fopen (inputFileName,"r");
    #if TIME
    if(rank==0)
    {
        runST = MPI_Wtime();
    }
    #endif
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &commSize);
    if (rank==0)
    {
        outputFile = fopen (outputFileName,"w");
        printf("init done\n");
    }

    #if TIME
    st1 = MPI_Wtime();
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
    minD = (int*)malloc(sizeof(int)*numNode);
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
    }
    #if DEBUG
    char logFileName[10];
    sprintf(logFileName,"rank%d.log",rank);
    logFile = fopen (logFileName,"w");
    logFile = stdout;
    #endif

    #if TIME
    MPI_Barrier(MPI_COMM_WORLD);
    et1 = MPI_Wtime();
    iot += et1-st1;
    st1 = MPI_Wtime();
    #endif
    MPI_Request request;
    MPI_Status status;
    broadcast(rank, rank);
    #if TIME
    et1 = MPI_Wtime();
    commt += et1-st1;
    st1 = MPI_Wtime();
    #endif
    MPI_Barrier(MPI_COMM_WORLD);
    #if TIME
    et1 = MPI_Wtime();
    synct += et1-st1;
    #endif
    if( ballHold && rank==0)
    {
        ans = (int*) malloc(sizeof(int)*numNode*numNode);
        #if DEBUG
        printf("[rank %d] start RING!!!!!!\n", rank);
        #endif
        color = BLACK;
        int temp[2]={color, rank};
        #if TIME
        st1 = MPI_Wtime();
        #endif
        MPI_Send( temp, 2, MPI_INT, rank+1 , BALL_TAG, MPI_COMM_WORLD);
        #if TIME
        et1 = MPI_Wtime();
        commt += et1-st1;
        #endif
        ballHold=0;
    }
    while(1)
    {
        for (int i=0;i<numNode;++i)
            updates[i]=0;
        color = -1;
        source = -1;
        updated = 0;
        flag = 1;
        #if TIME
        st1 = MPI_Wtime();
        #endif
        if(recvInited)
        {
            MPI_Irecv(recvData, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &request);
            recvInited = 0;
        }
        MPI_Wait(&request, &status);
        #if TIME
        et1 = MPI_Wtime();
        commt += et1-st1;
        #endif
        while(1)
        {
            if(flag!=0)
            {
                if( status.MPI_SOURCE >=0)
                {
                    #if DEBUG
                    //printf("[rank %d] recv : %d,%d , slave : %d\n", rank, update[0], update[1], status.MPI_SOURCE);
                    #endif

                    if ( status.MPI_TAG == DIS_TAG )
                    {
                        #if TIME
                        st1 = MPI_Wtime();
                        #endif
                        //MPI_Isend(recvData, 2, MPI_INT, status.MPI_SOURCE, DIS_ACK_TAG, MPI_COMM_WORLD, &sendReq);
                        #if DEBUG
                        //if(rank==49 && update[0]==12)
                        fprintf(logFile,"%lf [rank %d] recv %d,%d from %d\n", MPI_Wtime(), rank, recvData[0], recvData[1], status.MPI_SOURCE);
                        #endif
                        if(recvData[1]<minD[recvData[0]])
                        {
                            //printf("[rank %d] update from %d %d->%d\n",rank, update[0], minD[update[0]], update[1]);
                            updated=1;
                            allUpdated = 1;
                            minD[recvData[0]] = recvData[1];
                            updates[recvData[0]] = 1;
                        }
                        #if TIME
                        et1 = MPI_Wtime();
                        cput += et1-st1;
                        #endif
                    }
                    /*
                    else if(status.MPI_TAG == DIS_ACK_TAG)
                    {
                        eraseJob(jobs, recvData[0], recvData[1], status.MPI_SOURCE);
                        #if DEBUG
                        //if(rank==49 && update[0]==12)
                        //fprintf(logFile,"%lf [rank %d] ack %d,%d from %d\n", MPI_Wtime(), rank, recvData[0], recvData[1], status.MPI_SOURCE);
                        #endif
                    }
                    */
                    else if(status.MPI_TAG == BALL_TAG)
                    {
                        #if TIME
                        st1 = MPI_Wtime();
                        #endif
                        color = recvData[0];
                        source = recvData[1];
                        #if DEBUG
                        //if(rank==49 && update[0]==12)
                        fprintf(logFile,"%lf [rank %d] RB %d,%d from %d\n", MPI_Wtime(), rank, recvData[0], recvData[1], status.MPI_SOURCE);
                        #endif
                        #if TIME
                        et1 = MPI_Wtime();
                        cput += et1-st1;
                        #endif
                    }
                    else 
                        printf("[rank %d] wrong tag (%d) !!!!\n", rank, status.MPI_TAG);
                    #if TIME
                    st1 = MPI_Wtime();
                    #endif
                    MPI_Irecv(recvData, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &request);
                    MPI_Test( &request, &flag, &status);
                    #if TIME
                    et1 = MPI_Wtime();
                    commt += et1-st1;
                    #endif
                }
                #if DEBUG
                else
                {
                    printf("[rank %d] recv %d,%d from %d source error\n", rank, recvData[0], recvData[1], status.MPI_SOURCE);
                }
                #endif
            }
            else
            {
                #if DEBUG
                printf("[rank %d] RF %d,%d from %d\n", rank, recvData[0], recvData[1], status.MPI_SOURCE);
                #endif
                break;
            }
        }
        #if DEBUG>1
        printf("[rank %d] updating\n", rank);
        #endif
        #if TIME
        st1 = MPI_Wtime();
        #endif
        for (int i=0;i<numNode;++i)
        {
            if( updates[i])
                broadcast(i,-1);
        }
        #if TIME
        et1 = MPI_Wtime();
        commt += et1-st1;
        #endif
        /*
        for(int i=0; i<jobs.size();++i)
        {
            updated = 1;
            allUpdated = 1;
            if(jobs[i].round >= ROUND)
            {
                int sendData[2]={jobs[i].source, jobs[i].dis};
                MPI_Isend(&sendData, 2, MPI_INT, jobs[i].neighbor, DIS_TAG, MPI_COMM_WORLD, jobs[i].req);
                jobs[i].round = 0;
            }
            else
            {
                ++jobs[i].round;
            }
        }
        */
        if( color != -1 && source != -1)
        {
            //printf("[rank %d] recv ball %d updated=%d allUpdated=%d\n",rank, color, updated, allUpdated);
            if( color == WHITE && allUpdated == 0)
            {
                ++cycle;
                int temp[2]={color,rank};
                #if TIME
                st1 = MPI_Wtime();
                #endif
                MPI_Send( temp, 2, MPI_INT, (rank==commSize-1) ? 0 : rank+1 , BALL_TAG, MPI_COMM_WORLD);
                #if TIME
                et1 = MPI_Wtime();
                commt += et1-st1;
                #endif
                if(cycle==2)
                {
                    break;
                }
            }
            else
            {
                allUpdated = 0;
                color = BLACK;
                if( rank==0 && updated == 0)
                {
                    allUpdated = 0;
                    color = WHITE;
                }
                int temp[2]={color, cycle};
                #if TIME
                st1 = MPI_Wtime();
                #endif
                MPI_Send( temp, 2, MPI_INT, (rank==commSize-1) ? 0 : rank+1 , BALL_TAG, MPI_COMM_WORLD);
                #if TIME
                et1 = MPI_Wtime();
                commt += et1-st1;
                #endif
            }
        }
        #if DEBUG>2
        if(rank==0)
            printf("WHHHHHHYYYYY bh=%d updated=%d size=%d\n", ballHold, updated, jobs.size());
        #endif
    }
    #if DEBUG
    //if(rank==49)
    {
        printf("[rank %d] final minD\n", rank);
        for(int i=0;i<numNode;++i)
            printf("-%d- ",minD[i]);
        printf("\n");
    }
    #endif
    #if TIME
    st1 = MPI_Wtime();
    #endif
    MPI_Gather (minD, numNode, MPI_INT, ans, numNode, MPI_INT, 0, MPI_COMM_WORLD);
    #if TIME
    et1 = MPI_Wtime();
    commt += et1-st1;
    #endif
    if(rank==0)
    {
        //printf("malloc ans");
        #if TIME
        st1 = MPI_Wtime();
        #endif
        printMap(outputFile);
        #if TIME
        et1 = MPI_Wtime();
        iot += et1-st1;
        #endif
        #if DEBUG
        printMap(stdout);
        #endif
    }

    #if TIME
    MPI_Reduce(&cput, &allcput, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&commt, &allcommt, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&iot, &alliot, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&synct, &allsynct, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    if(rank==0)
    {
        printf("iot time\t%lf\n", alliot);
        printf("cput time\t%lf\n", allcput/commSize);
        printf("commt time\t%lf\n", allcommt/commSize);
        printf("synct time\t%lf\n", allsynct);
        runET = MPI_Wtime();
        printf("Runtime = %lf\n", runET-runST);
    }
    #endif

    MPI_Finalize();
}
