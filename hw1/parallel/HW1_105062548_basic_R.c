#include<stdio.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<mpi.h>
#include<assert.h>
#include<time.h>
int getRankPlus1(int rank, int size)
{
    if (rank+1 == size)
        return 0;
    return rank+1;
}

int getRankMinus1(int rank, int size)
{
    if (rank-1 == -1)
        return size-1;
    return rank-1;
}

int main(int argc, char** argv)
{
    assert(argc==4);
    int n = atoi(argv[1]);
    char *inputName = argv[2];
    char *outputName = argv[3];
    float *floats=NULL, *allFloats=NULL;
    int commSize, floatsSize, rank;
    float iotime=0, computetime=0, commtime=0;
    clock_t start_time, end_time;
    MPI_Init(&argc, &argv);
    MPI_File inputFH, outputFH;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &commSize);
    floatsSize = (n%commSize > rank) ? (n/commSize+1) : n/commSize;
    floats = (float*)malloc((floatsSize+1)*sizeof(float));
    //printf("rank %d\n", rank);
    start_time = clock();
    MPI_File_open(MPI_COMM_WORLD, inputName, MPI_MODE_RDONLY, MPI_INFO_NULL, &inputFH);
    MPI_File_open(MPI_COMM_WORLD, outputName, MPI_MODE_WRONLY|MPI_MODE_CREATE, MPI_INFO_NULL, &outputFH);
    MPI_File_seek(inputFH, rank*sizeof(float), MPI_SEEK_CUR);
    for (int i=0;i<floatsSize;++i)
    {
        MPI_File_read(inputFH, &floats[i], 1, MPI_FLOAT, MPI_STATUS_IGNORE);
        //printf("===%d===\n", (commSize-1)*sizeof(float));
        MPI_File_seek(inputFH, (commSize-1)*sizeof(float), MPI_SEEK_CUR);
        
    }
    end_time = clock();
    iotime+=(float)(end_time - start_time)/CLOCKS_PER_SEC;
    /*
    if (rank==0)
    {
        allFloats = (float*)malloc( (n+1) * sizeof(float));
        MPI_File_seek(inputFH, 0, MPI_SEEK_SET);
        MPI_File_read(inputFH, allFloats, n, MPI_FLOAT, MPI_STATUS_IGNORE);
        
        printf("init ");
        for (int i=0;i<n;i++)
            printf(" %f", allFloats[i]);
        printf("\n");
        
        //MPI_Scatter(allFloats, 2, MPI_FLOAT, floats, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
    }
    */
    //MPI_Barrier(MPI_COMM_WORLD);
    //else
        //MPI_Scatter(allFloats, 2, MPI_FLOAT, floats, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
    //printf("rank[%d] ", rank);
    //for(int i=0;i<floatsSize;i++)
    //    printf("rank [%d]_%d %f\n", rank, i, floats[i]);
    MPI_Barrier(MPI_COMM_WORLD);
    start_time = clock();
    int isChanged = 0, isAllChanged = 0;
    int time=0;
    do
    {
        if (rank%2==0)
        {
            isChanged = 0;
            for(int i=0;i<floatsSize;++i)
            {
                if(rank+i*commSize+1<n)
                {
                    float temp;
                    //printf("rank[%d] sending %f to %d\n", rank, floats[i], getRankPlus1(rank, commSize));
                    MPI_Send(floats+i, 1, MPI_FLOAT, getRankPlus1(rank, commSize), 0, MPI_COMM_WORLD);
                    MPI_Recv(&temp, 1, MPI_FLOAT, getRankPlus1(rank, commSize), 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    //printf("rank[%d] recv %f from %d\n", rank, temp, getRankPlus1(rank, commSize));
                    if (floats[i]>temp)
                    {
                        //printf("rank[%d] temp %f origin %f\n", rank, temp, floats[i]);
                        floats[i] = temp, isChanged=1;

                    }
                }
            }
            //MPI_Reduce(&isChanged, &isAllChanged, 1, MPI_INT, MPI_BOR, 0, MPI_COMM_WORLD);
            MPI_Barrier(MPI_COMM_WORLD);
            for (int i=(rank==0)?1:0 ;i<floatsSize;++i)
            {
                float temp;
                MPI_Recv(&temp, 1, MPI_FLOAT, getRankMinus1(rank, commSize), 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                //printf("rank[%d] recv %f from %d\n", rank, temp, getRankMinus1(rank, commSize));
                //printf("rank[%d] sending %f to %d\n", rank, floats[i], getRankMinus1(rank, commSize));
                MPI_Send(floats+i, 1, MPI_FLOAT, getRankMinus1(rank, commSize), 0, MPI_COMM_WORLD);
                if(temp>floats[i])
                    floats[i]=temp, isChanged=1;
            }
        }
        else
        {
            isChanged = 0;
            for(int i=0;i<floatsSize;++i)
            {
                float temp;
                MPI_Recv(&temp, 1, MPI_FLOAT, getRankMinus1(rank, commSize), 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                //printf("rank[%d] recv %f from %d\n", rank, temp, getRankMinus1(rank, commSize));
                //printf("rank[%d] sending %f to %d\n", rank, floats[i], getRankMinus1(rank, commSize));
                MPI_Send(floats+i, 1, MPI_FLOAT, getRankMinus1(rank, commSize), 0, MPI_COMM_WORLD);
                if (temp>floats[i])
                    floats[i] = temp, isChanged=1;
            }
            //MPI_Reduce(&isChanged, &isAllChanged, 1, MPI_INT, MPI_BOR, 0, MPI_COMM_WORLD);
            MPI_Barrier(MPI_COMM_WORLD);
            for(int i=0;i<floatsSize;++i)
            {
                if(rank+i*commSize+1<n)
                {
                    float temp;
                    //printf("rank[%d] sending %f to %d\n", rank, floats[i], getRankPlus1(rank, commSize));
                    MPI_Send(floats+i, 1, MPI_FLOAT, getRankPlus1(rank, commSize), 0, MPI_COMM_WORLD);
                    MPI_Recv(&temp, 1, MPI_FLOAT, getRankPlus1(rank, commSize), 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    //printf("rank[%d] recv %f from %d\n", rank, temp, getRankPlus1(rank, commSize));
                    if(floats[i]>temp)
                    {
                        //printf("rank[%d] temp %f origin %f\n", rank, temp, floats[i]);
                        floats[i] = temp, isChanged = 1;
                    }
                }
            }
        }
        //printf("rank[%d] done\n", rank);
        MPI_Reduce(&isChanged, &isAllChanged, 1, MPI_INT, MPI_BOR, 0, MPI_COMM_WORLD);
        /*
        if(rank==0)
            printf("isAllChanged %d time %d\n", isAllChanged, ++time);
        */
        MPI_Bcast(&isAllChanged, 1, MPI_INT, 0, MPI_COMM_WORLD);
    }while(isAllChanged);
    end_time = clock();
    computetime = (float)(end_time - start_time)/CLOCKS_PER_SEC;
    MPI_File_seek(outputFH, rank*sizeof(float), MPI_SEEK_CUR);
    for (int i=0;i<floatsSize;++i)
    {
        MPI_File_write(outputFH, &floats[i], 1, MPI_FLOAT, MPI_STATUS_IGNORE);
        MPI_File_seek(outputFH, (commSize-1)*sizeof(float), MPI_SEEK_CUR);
    }


    MPI_File_close(&inputFH);
    MPI_File_close(&outputFH);
    MPI_Finalize();

}
