#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<time.h>
#define DEBUG 0
#define TIME 0
struct Info{
    int start,end,k;
}infos[2000];

unsigned int **map;
int numNode, numEdge;
struct timespec st1, st2, et1, et2, st3, et3;
double cput=0.0, synct=0.0, iot=0.0;

void printMap(FILE* file)
{
    for(int i=0;i<numNode;++i)
    {
        for(int j=0;j<numNode;++j)
        {
            unsigned int temp = map[i][j];
            if(temp==2147483647)
                temp = 0;
            fprintf(file, "%u ", temp);
        }
        fprintf(file,"\n");
    }
}

void* compute(void* vinfo)
{
    Info *info = ((Info*)vinfo);
    int k = info->k;
    for(int i=info->start; i<=info->end; ++i)
    {
        for(int j=0;j<numNode;++j)
        {
            if(i==j||map[i][k]==2147483647||map[k][j]==2147483647)
                continue;
            if(map[i][j] > map[i][k] + map[k][j])
                map[i][j] = map[i][k] + map[k][j];
        }
    }
}

int main(int argc, char **argv)
{
    char *inputFileName = argv[1];
    char *outputFileName = argv[2];
    int numThreads = strtol(argv[3], 0, 10);  
    FILE* inputFile = fopen (inputFileName,"r");
    FILE* outputFile = fopen (outputFileName,"w");
    pthread_t *threads = (pthread_t*)malloc(numThreads * sizeof(pthread_t));

    #if TIME
    clock_gettime(CLOCK_MONOTONIC, &st1);
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
    int blockSize = numNode/(numThreads);
    int mod = numNode%(numThreads);
    //init map
    for(int k=0;k<numEdge;++k)
    {
        int i, j, w;
        fscanf(inputFile, "%d %d %d", &i, &j, &w);
        map[i][j] = w;
        map[j][i] = w;
    }
    #if TIME
    clock_gettime(CLOCK_MONOTONIC, &et1);
    iot += (et1.tv_sec - st1.tv_sec);
    iot += (et1.tv_nsec - st1.tv_nsec) / 1000000000.0;
    #endif

    #if TIME
    clock_gettime(CLOCK_MONOTONIC, &st1);
    //printf("st %f\n",(float)clock());
    #endif
    for( int k=0;k<numNode;++k)
    {
        int i;
        for(i=0;i<numThreads;++i)
        {
            infos[i].start = blockSize*i;
            infos[i].start += (mod>i) ? i : mod;
            infos[i].end = (mod > i ) ? (blockSize+1) : blockSize;
            infos[i].end += infos[i].start-1;
            infos[i].k = k;
            #if DEBUG
            if(k<2)
                printf("compute %d %d %d %d \n", i, infos[i].start, infos[i].end, infos[i].k);
            #endif
            if(i<numThreads-1)
                pthread_create( &threads[i], NULL, compute, &infos[i]);
        }
        compute(&infos[i-1]);
        #if TIME
        clock_gettime(CLOCK_MONOTONIC, &st2);
        #endif
        
        for(int i=0;i<numThreads-1;++i)
        {
            #if TIME
            clock_gettime(CLOCK_MONOTONIC, &st3);
            #endif
            pthread_join(threads[i], NULL);
            #if TIME
            clock_gettime(CLOCK_MONOTONIC, &et3);
            double wt = (et3.tv_sec - st3.tv_sec);
            wt += (et3.tv_nsec - st3.tv_nsec) / 1000000000.0;
            printf("wait %d %lf sec\n", i, wt);
            #endif
        }
        #if TIME
        clock_gettime(CLOCK_MONOTONIC, &et2);
        synct += (et2.tv_sec - st2.tv_sec);
        synct += (et2.tv_nsec - st2.tv_nsec) / 1000000000.0;
        #endif
    }
    #if TIME
    clock_gettime(CLOCK_MONOTONIC, &et1);
    cput += (et1.tv_sec - st1.tv_sec);
    cput += (et1.tv_nsec - st1.tv_nsec) / 1000000000.0;
    //printf("%f %f\n",(float)et1 ,(float)st1);
    cput -=synct;
    #endif

    #if TIME
    clock_gettime(CLOCK_MONOTONIC, &st1);
    #endif
    printMap(outputFile);
    #if TIME
    clock_gettime(CLOCK_MONOTONIC, &et1);
    iot += (et1.tv_sec - st1.tv_sec);
    iot += (et1.tv_nsec - st1.tv_nsec) / 1000000000.0;
    printf("cpu\t%lf\nsync\t%lf\nio\t%lf\n", cput, synct, iot);
    #endif
}
