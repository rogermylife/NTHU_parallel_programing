#include <assert.h>
#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <time.h>
#include <vector>
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
MPI_Request sendReq;
int *minD;
int commSize, rank;
int *ans;
std::vector<Job> jobs;
int color,source;
int flag, updated=0, allUpdated=0,cycle=0;
int ballHold=1, recvInited=1;
int updates[2000];
int recvData[2];
#if DEBUG
FILE* logFile;
#endif

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
        int sendData[2] = {-1,-1};
        sendData[0] = start;
        sendData[1] = minD[start]+map[rank][i];
        #if DEBUG
        //if(i==49 && start==12)
        //printf("[rank %d] send to %d from %d dis %d\n", rank, i, start, sendData[1]);
        #endif
        Job temp = { .source = start, .dis=sendData[1], .neighbor = i, .round = 0};
        temp.req = (MPI_Request*)malloc(sizeof(MPI_Request));
        eraseJob(jobs, start, -1, i);
        MPI_Isend(sendData, 2, MPI_INT, i, DIS_TAG, MPI_COMM_WORLD, temp.req);
        MPI_Status sendStatus;
        //MPI_Wait(temp.req, &sendStatus);
        //jobs.push_back(temp);
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
    minD = (int*)malloc(sizeof(int)*numNode);
    for(int i=0;i<numNode;++i)
        minD[i] = 2147483647;
    
    //init map
    for(int k=0;k<numEdge;++k)
    {
        int i, j, w;
        fscanf(inputFile, "%d %d %d", &i, &j, &w);
        map[i][j] = w;
        map[j][i] = w;
    }
    /* MPI init */
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &commSize);
    minD[rank] = 0;
    #if DEBUG
    char logFileName[10];
    sprintf(logFileName,"rank%d.log",rank);
    logFile = fopen (logFileName,"w");
    logFile = stdout;
    #endif
    
    MPI_Request request;
    MPI_Status status;
    broadcast(rank, rank);
    MPI_Barrier(MPI_COMM_WORLD);
    if( ballHold && rank==0)
    {
        #if DEBUG
        printf("[rank %d] start RING!!!!!!\n", rank);
        #endif
        color = BLACK;
        int temp[2]={color, rank};
        MPI_Send( temp, 2, MPI_INT, rank+1 , BALL_TAG, MPI_COMM_WORLD);
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
        if(recvInited)
        {
            MPI_Irecv(recvData, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &request);
            recvInited = 0;
        }
        MPI_Wait(&request, &status);
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
                        //MPI_Isend(recvData, 2, MPI_INT, status.MPI_SOURCE, DIS_ACK_TAG, MPI_COMM_WORLD, &sendReq);
                        #if DEBUG
                        //if(rank==49 && update[0]==12)
                        //fprintf(logFile,"%lf [rank %d] recv %d,%d from %d\n", MPI_Wtime(), rank, recvData[0], recvData[1], status.MPI_SOURCE);
                        #endif
                        if(recvData[1]<minD[recvData[0]])
                        {
                            //printf("[rank %d] update from %d %d->%d\n",rank, update[0], minD[update[0]], update[1]);
                            updated=1;
                            allUpdated = 1;
                            minD[recvData[0]] = recvData[1];
                            updates[recvData[0]] = 1;
                        }
                    }
                    else if(status.MPI_TAG == DIS_ACK_TAG)
                    {
                        eraseJob(jobs, recvData[0], recvData[1], status.MPI_SOURCE);
                        #if DEBUG
                        //if(rank==49 && update[0]==12)
                        //fprintf(logFile,"%lf [rank %d] ack %d,%d from %d\n", MPI_Wtime(), rank, recvData[0], recvData[1], status.MPI_SOURCE);
                        #endif
                    }
                    else if(status.MPI_TAG == BALL_TAG)
                    {
                        color = recvData[0];
                        source = recvData[1];
                        #if DEBUG
                        //if(rank==49 && update[0]==12)
                        fprintf(logFile,"%lf [rank %d] RB %d,%d from %d\n", MPI_Wtime(), rank, recvData[0], recvData[1], status.MPI_SOURCE);
                        #endif
                    }
                    else 
                        printf("[rank %d] wrong tag (%d) !!!!\n", rank, status.MPI_TAG);
                    MPI_Irecv(recvData, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &request);
                    MPI_Test( &request, &flag, &status);
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
        
        for (int i=0;i<numNode;++i)
        {
            if( updates[i])
                broadcast(i,-1);
        }
        
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
            printf("[rank %d] recv ball %d updated=%d allUpdated=%d\n",rank, color, updated, allUpdated);
            if( color == WHITE && allUpdated == 0)
            {
                ++cycle;
                int temp[2]={color,rank};
                MPI_Send( temp, 2, MPI_INT, (rank==commSize-1) ? 0 : rank+1 , BALL_TAG, MPI_COMM_WORLD);
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
                MPI_Send( temp, 2, MPI_INT, (rank==commSize-1) ? 0 : rank+1 , BALL_TAG, MPI_COMM_WORLD);
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
