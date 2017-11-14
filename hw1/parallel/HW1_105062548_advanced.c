#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<algorithm>
#include<mpi.h>
#include<time.h>
#define BLOCK_SIZE 1500000
bool myfunction (float i,float j) { return (j-i>(float)0.0); }
float* mergeFloats(float *a, int an, float* b, int bn)
{
    float* mergedFloats = (float*)malloc( (an+bn+1)*sizeof(float) );
    int index=0;
    for(int i=0,j=0;i<an||j<bn;)
    {
        //printf("hehe an %d, bn %d, %f, %f, i=%d j=%d\n", an, bn, a[i], b[j], i, j);
        if((j==bn&&i<an) || (i<an&&a[i]<b[j]))
            mergedFloats[index++] = a[i++];
        else
            mergedFloats[index++] = b[j++];
    }
    return mergedFloats;
}
int main(int argc, char **argv)
{
    assert(argc==4);
    int n = atoi(argv[1]);
    char *inputName = argv[2];
    char *outputName = argv[3];
    float *floats, *buddyFloats, *mergedFloats;
    float IOT=0, CPUT=0, CommT=0, tT;
    float allIOT=0, allCPUT=0, allCommT=0;
    clock_t startT, endT;
    int commSize, floatsSize, rank, rankUsedNum, mod, seek;
    MPI_Init(&argc, &argv);
    MPI_File inputFH, outputFH;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &commSize);
    mod = n%commSize;
    if( n > commSize*BLOCK_SIZE)
    {
        rankUsedNum = commSize;
        floatsSize = ( mod > rank) ? (n/commSize+1) : n/commSize;
        seek = n/commSize*rank;
        seek+= (mod>rank)? rank : mod;
    }
    else
    {
        int bsmod = n%BLOCK_SIZE;
        rankUsedNum = (bsmod != 0) ? n/BLOCK_SIZE+1: n/BLOCK_SIZE;
        if(bsmod==0)
            floatsSize = BLOCK_SIZE;
        else if(rank==0)
            floatsSize=bsmod;
        else if(n/BLOCK_SIZE>=rank)
            floatsSize=BLOCK_SIZE;
        else
            floatsSize=0;

        if(bsmod==0)
            seek=(rank==0 || rankUsedNum <= rank)? 0 : BLOCK_SIZE*(rank)+bsmod;
        else
            seek=(rank==0 || rankUsedNum <= rank)? 0 : BLOCK_SIZE*(rank-1)+bsmod;
    }
    //printf("rank[%d] seek %d\n",rank ,seek);
    floats = (float*)malloc((floatsSize+1)*sizeof(float));
    startT = clock();
    MPI_File_open(MPI_COMM_WORLD, inputName, MPI_MODE_RDONLY, MPI_INFO_NULL, &inputFH);
    MPI_File_open(MPI_COMM_WORLD, outputName, MPI_MODE_WRONLY|MPI_MODE_CREATE, MPI_INFO_NULL, &outputFH);
    MPI_File_seek(inputFH, seek*sizeof(float), MPI_SEEK_SET);//debug
    //printf("seek %d\n", (n/2)*rank*sizeof(float));//debug
    MPI_File_read(inputFH, floats, floatsSize, MPI_FLOAT, MPI_STATUS_IGNORE);
    endT = clock();
    IOT += (float)(endT - startT)/CLOCKS_PER_SEC;

    if(rank<rankUsedNum)
    {
        startT = clock();
        std::sort(floats, floats+floatsSize, myfunction);
        mergedFloats = floats;
        endT = clock();
        CPUT += (float)(endT - startT)/CLOCKS_PER_SEC;
    }
    if(rank<rankUsedNum)
    for(int i=2,padding=1;padding<=rankUsedNum;i*=2,padding*=2)
    {
        if(rank%i==0 && rank+padding<rankUsedNum)
        {
            //printf("rank[%d] start recv\n",rank);
            int buddyFloatsSize;
            startT = clock();
            MPI_Recv(&buddyFloatsSize, 1, MPI_INT, rank+padding, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            buddyFloats = (float*)malloc( (buddyFloatsSize+1)*sizeof(float) );
            MPI_Recv(buddyFloats, buddyFloatsSize, MPI_FLOAT, rank+padding, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            endT = clock();
            CommT += (float)(endT - startT)/CLOCKS_PER_SEC;
            startT = clock();
            mergedFloats = mergeFloats(floats, floatsSize, buddyFloats, buddyFloatsSize);
            free(floats);
            free(buddyFloats);
            floats = mergedFloats;
            floatsSize +=buddyFloatsSize;
            endT = clock();
            CPUT += (float)(endT - startT)/CLOCKS_PER_SEC;
            //printf("rank[%d] %d\n",rank, floatsSize);
        }
        else if(rank%i==padding)
        {
            //printf("rank[%d] start sending\n", rank);
            startT = clock();
            MPI_Send(&floatsSize, 1, MPI_INT, rank-padding, 0, MPI_COMM_WORLD);
            MPI_Send(floats, floatsSize, MPI_FLOAT, rank-padding, 0, MPI_COMM_WORLD);
            endT = clock();
            CommT += (float)(endT - startT)/CLOCKS_PER_SEC;
        }
    }
    if(rank==0)
    {
        startT = clock();
        if(n!=0)
            MPI_File_write(outputFH, mergedFloats, floatsSize, MPI_FLOAT, MPI_STATUS_IGNORE);
        endT = clock();
        IOT += (float)(endT - startT)/CLOCKS_PER_SEC;
        //for(int i=0;i<100;i++)
        //    printf("%f\n",mergedFloats[i]);
    }
    MPI_Reduce(&CPUT, &allCPUT, 1, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&CommT, &allCommT, 1, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&IOT, &allIOT, 1, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);
    if(rank == 0)
    {
        printf("cpu time\t= %.16f\n",allCPUT/commSize);
        printf("comm time\t= %.16f\n",allCommT/commSize);
        printf("IO time\t=%.16f\n",allIOT/commSize);
    }

    MPI_File_close(&inputFH);
    MPI_File_close(&outputFH);
    MPI_Finalize();
}
