#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<algorithm>
#include<mpi.h>

int getBuddyFloatsSize(int rank, int n)
{
    if(n&1==1)
        return n/2+1;
    return n/2;
}

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
    float *floats, *buddyFloats;
    int commSize, floatsSize, rank;
    MPI_Init(&argc, &argv);
    MPI_File inputFH, outputFH;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &commSize);
    floatsSize = (rank==1 && n%2==1) ? n/2+1: n/2;
    floats = (float*)malloc((floatsSize+1)*sizeof(float));

    MPI_File_open(MPI_COMM_WORLD, inputName, MPI_MODE_RDONLY, MPI_INFO_NULL, &inputFH);
    MPI_File_open(MPI_COMM_WORLD, outputName, MPI_MODE_WRONLY|MPI_MODE_CREATE, MPI_INFO_NULL, &outputFH);
    MPI_File_seek(inputFH, (n/2)*rank*sizeof(float), MPI_SEEK_SET);//debug
    //printf("seek %d\n", (n/2)*rank*sizeof(float));//debug
    MPI_File_read(inputFH, floats, floatsSize, MPI_FLOAT, MPI_STATUS_IGNORE);
    if(rank<=1)
    {
        std::sort(floats, floats+floatsSize);
    }
    if(rank==0)
    {
        //for(int i=0;i<floatsSize;i++)
        //    printf("0number[%d] %f\n", i, floats[i]);
        int buddyFloatsSize = getBuddyFloatsSize(1, n);
        buddyFloats = (float*)malloc( (buddyFloatsSize+1)*sizeof(float) );
        MPI_Recv(buddyFloats, buddyFloatsSize, MPI_FLOAT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        /*
        for(int i=0;i<buddyFloatsSize;i++)
            printf("number[%d] %f\n", i, buddyFloats[i]);
        */
    }
    else if(rank==1)
    {
        /*
        for(int i=0;i<floatsSize;i++)
            printf("snumber[%d] %f\n", i, floats[i]);
        */
        MPI_Send(floats, floatsSize, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
    }
    if(rank==0)
    {
        float *mergedFloats = mergeFloats(floats, floatsSize, buddyFloats, getBuddyFloatsSize(1, n));
        MPI_File_write(outputFH, mergedFloats, floatsSize+getBuddyFloatsSize(1, n), MPI_FLOAT, MPI_STATUS_IGNORE);
    }
    MPI_File_close(&inputFH);
    MPI_File_close(&outputFH);
    MPI_Finalize();
}
