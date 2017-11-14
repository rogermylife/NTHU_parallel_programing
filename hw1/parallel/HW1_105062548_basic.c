#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<algorithm>
#include<mpi.h>
#include<time.h>
int main(int argc, char **argv)
{
    assert(argc==4);
    int n = atoi(argv[1]);
    char *inputName = argv[2];
    char *outputName = argv[3];
    float *floats, *buddyFloats, *mergedFloats;
    int commSize, floatsSize, rank, rankUsedNum, mod, seek, index;
    int isChanged=0, isAllChanged=0;
    float CPUT=0, CommT=0, IOT=0, tT=0;
    float allCPUT=0, allCommT=0, allIOT=0;
    clock_t startT,endT;
    MPI_Init(&argc, &argv);
    MPI_File inputFH, outputFH;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &commSize);
    mod = n%commSize;
    rankUsedNum = commSize;
    floatsSize = ( mod > rank) ? (n/commSize+1) : n/commSize;
    seek = n/commSize*rank;
    seek+= (mod>rank)? rank : mod;
    //printf("rank[%d] seek %d\n",rank ,seek);
    floats = (float*)malloc((floatsSize+1)*sizeof(float));
    startT = clock();
    MPI_File_open(MPI_COMM_WORLD, inputName, MPI_MODE_RDONLY, MPI_INFO_NULL, &inputFH);
    MPI_File_open(MPI_COMM_WORLD, outputName, MPI_MODE_WRONLY|MPI_MODE_CREATE, MPI_INFO_NULL, &outputFH);
    MPI_File_set_size(outputFH, 0);
    MPI_File_seek(inputFH, seek*sizeof(float), MPI_SEEK_SET);//debug
    //printf("seek %d\n", (n/2)*rank*sizeof(float));//debug
    MPI_File_read(inputFH, floats, floatsSize, MPI_FLOAT, MPI_STATUS_IGNORE);
    endT = clock();
    IOT += (float)(endT - startT)/CLOCKS_PER_SEC;
    int run=0,i;
    MPI_Request req;
    do 
    {
        isChanged = 0, isAllChanged = 0;
        //printf("rank[%d] round %d\n", rank, run++);
        startT = clock();
        if(floatsSize==0)
            goto END;
        i = ((seek&1)==0)? 0:1;
        for(;i<floatsSize-1;i+=2)
            if(floats[i]>floats[i+1])
                std::swap(floats[i], floats[i+1]),isChanged=1;
        endT = clock();
        CPUT += (float)(endT - startT)/CLOCKS_PER_SEC;
        
        startT = clock();
        index = seek+floatsSize-1;
        if((index&1)==0 && index+1<n)
            MPI_Isend(&floats[floatsSize-1], 1, MPI_FLOAT, rank+1, 0, MPI_COMM_WORLD, &req);
        index = seek;
        if((index&1)==1)
        {
            float recv;
            MPI_Recv(&recv, 1, MPI_FLOAT, rank-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            //printf("rank[%d] recv %f from %d\n", rank, recv, rank-1);
            MPI_Isend(&floats[0], 1, MPI_FLOAT, rank-1, 0, MPI_COMM_WORLD, &req);
            if(recv>floats[0])
                floats[0] = recv,isChanged = 1;
        }
        index = seek+floatsSize-1;
        if((index&1)==0 && index+1<n)
        {
            float recv;
            MPI_Recv(&recv, 1, MPI_FLOAT, rank+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            //printf("rank[%d] recv %f from %d\n", rank, recv, rank+1);
            if(floats[floatsSize-1]>recv)
                floats[floatsSize-1] = recv,isChanged=1;
        }
        endT = clock();
        CommT += (float)(endT - startT)/CLOCKS_PER_SEC;
        //printf("rank[%d] done half\n", rank);

        startT = clock();
        i = (seek&1==1)? 0:1;
        for(;i<floatsSize-1;i+=2)
            if(floats[i]>floats[i+1])
                std::swap(floats[i], floats[i+1]),isChanged=1;
        endT = clock();
        CPUT += (float)(endT - startT)/CLOCKS_PER_SEC;

        startT = clock();


        index = seek+floatsSize-1;
        if((index&1)==1 && index+1<n)
        {
            //printf("rank[%d] send\n", rank);
            MPI_Isend(&floats[floatsSize-1], 1, MPI_FLOAT, rank+1, 0, MPI_COMM_WORLD, &req);
        }
        index = seek;
        //printf("rank[%d] fuckthis index=%d ans=%d\n", rank, index, (index&1));
        if((index&1)==0 && seek>0)
        {
            float recv;
            MPI_Recv(&recv, 1, MPI_FLOAT, rank-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            //printf("rank[%d] recv %f from %d\n", rank, recv, rank-1);
            MPI_Isend(&floats[0], 1, MPI_FLOAT, rank-1, 0, MPI_COMM_WORLD, &req);
            if(recv>floats[0])
                floats[0] = recv,isChanged = 1;
        }
        index = seek+floatsSize-1;
        if((index&1)==1 && index+1<n)
        {
            float recv;
            MPI_Recv(&recv, 1, MPI_FLOAT, rank+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            //printf("rank[%d] recv %f from %d\n", rank, recv, rank+1);
            if(floats[floatsSize-1]>recv)
                floats[floatsSize-1] = recv,isChanged=1;
        }
        endT = clock();
        CommT += (float)(endT - startT)/CLOCKS_PER_SEC;
        //printf("rank[%d] done trun\n",rank);
        if((run&1024)==1024)
        {
            isAllChanged=1;
            continue;
        }
        
    END:
        startT = clock();
        MPI_Reduce(&isChanged, &isAllChanged, 1, MPI_INT, MPI_BOR, 0, MPI_COMM_WORLD);
        MPI_Bcast(&isAllChanged, 1, MPI_INT, 0, MPI_COMM_WORLD);
        endT = clock();
        CommT += (float)(endT - startT)/CLOCKS_PER_SEC;
    }while(isAllChanged==1);

    startT = clock();
    MPI_File_seek(outputFH, seek*sizeof(float), MPI_SEEK_SET);
    MPI_File_write(outputFH, floats, floatsSize, MPI_FLOAT, MPI_STATUS_IGNORE);
    MPI_File_close(&inputFH);
    MPI_File_close(&outputFH);
    endT = clock();
    IOT += (float)(endT - startT)/CLOCKS_PER_SEC;
    MPI_Reduce(&CPUT, &allCPUT, 1, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&CommT, &allCommT, 1, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&IOT, &allIOT, 1, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);
    if(rank == 0)
    {
        printf("cpu time\t= %.16f\n",allCPUT/commSize);
        printf("comm time\t= %.16f\n",allCommT/commSize);
        printf("IO time\t=%.16f\n",allIOT/commSize);
    }
    MPI_Finalize();
}
