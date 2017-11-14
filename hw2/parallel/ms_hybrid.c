#define PNG_NO_SETJMP
#define DEBUG 0
#define TIME 0
#define HEIGHT_SIZE 20 
#define DATA_TAG 0
#define RESULT_TAG 1
#define TERMINATE_TAG 2
#define SOURCE_TAG 3

#include <assert.h>
#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <time.h>
#include <unistd.h>
#include <omp.h>

void write_png(const char* filename, const int width, const int height, const int* buffer) {
    FILE* fp = fopen(filename, "wb");
    assert(fp);
    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    assert(png_ptr);
    png_infop info_ptr = png_create_info_struct(png_ptr);
    assert(info_ptr);
    png_init_io(png_ptr, fp);
    png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png_ptr, info_ptr);
    size_t row_size = 3 * width * sizeof(png_byte);
    png_bytep row = (png_bytep)malloc(row_size);
    for (int y = 0; y < height; ++y) {
        memset(row, 0, row_size);
        for (int x = 0; x < width; ++x) {
            int p = buffer[(height - 1 - y) * width + x];
            row[x * 3] = ((p & 0xf) << 4);
        }
        png_write_row(png_ptr, row);
    }
    free(row);
    png_write_end(png_ptr, NULL);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);
}

int main(int argc, char** argv) {
    /* argument parsing */
    assert(argc == 9);
    int num_threads = strtol(argv[1], 0, 10);
    double left = strtod(argv[2], 0);
    double right = strtod(argv[3], 0);
    double lower = strtod(argv[4], 0);
    double upper = strtod(argv[5], 0);
    int width = strtol(argv[6], 0, 10);
    int height = strtol(argv[7], 0, 10);
    const char* filename = argv[8];
    float commTime=0;
    clock_t startT, endT;
    omp_set_num_threads(num_threads);

    /* MPI init */
    int commSize, rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &commSize);
    int row = 0, heightSize=0, mod;
    int *image, *buddyImage;
    int info[2];
    image = (int*)realloc(image, width*height*sizeof(int) );
    
    if (rank==0)
    {
        int count = 0,i;
        for (i=1; i<commSize; ++i)
        {
            #if (DEBUG)
            printf("init distributing %d start from %d\n", i , row);
            #endif
            MPI_Send(&row, 1, MPI_INT, i, DATA_TAG, MPI_COMM_WORLD);
            ++count;
            row+=HEIGHT_SIZE;
        }
        int recvHeight = 0;
        do{
            int buddyHeightSize=-1, flag;
            MPI_Request request;
            MPI_Status status;
            MPI_Recv(info, 2, MPI_INT, MPI_ANY_SOURCE, RESULT_TAG, MPI_COMM_WORLD, &status);
            #if(DEBUG)
            printf("buddyHeightSize=%d flag=%d tag=%d MPI_SOURCE=%d\n", info[1], flag, status.MPI_TAG, status.MPI_SOURCE);
            sleep(1);
            #endif
            #if(DEBUG)
            printf("%d recv result from %d buddyHeightSize=%d\n", row, status.MPI_SOURCE , info[1]);
            #endif
            buddyImage = (int*)malloc(width * info[1] * sizeof(int));
            MPI_Recv(buddyImage, info[1] * width, MPI_INT, status.MPI_SOURCE, RESULT_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            --count;
            if(row<height)
            {
                MPI_Send(&row, 1, MPI_INT, status.MPI_SOURCE, DATA_TAG, MPI_COMM_WORLD);
                row+=HEIGHT_SIZE;
                ++count;
            }
            else
                MPI_Send(&row, 1, MPI_INT, status.MPI_SOURCE, TERMINATE_TAG, MPI_COMM_WORLD);
            memcpy( image+info[0]*width, buddyImage, width*info[1]*sizeof(int));
        }while(count>0);
        #if(TIME)
        startT = clock();
        #endif
        write_png(filename, width, height, image);
        #if(TIME)
        endT = clock();
        float writeTime = (float)(endT - startT)/CLOCKS_PER_SEC;
        printf("write time = %f\n", writeTime);
        #endif
    }
    else
    {
        MPI_Status status;
        #if(TIME)
        startT = clock();
        #endif
        MPI_Recv(&row, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        #if(TIME)
        endT = clock();
        commTime  += (float)(endT - startT)/CLOCKS_PER_SEC;
        #endif

        while(status.MPI_TAG == DATA_TAG)
        {
            #if(DEBUG)
            printf("rank[%d] recv work start from %d\n", rank, row);
            #endif
            
            heightSize = (row+HEIGHT_SIZE<height) ? HEIGHT_SIZE : height-row;
            if(heightSize<0)
                heightSize=0;
            image = (int*)malloc(width*(heightSize)*sizeof(int) );
            #pragma omp parallel for schedule(dynamic)
            for (int counter = 0; counter < heightSize; ++counter) {
                double y0 = (row+counter) * ((upper - lower) / height) + lower;
                for (int i = 0; i < width; ++i) {
                    double x0 = i * ((right - left) / width) + left;

                    int repeats = 0;
                    double x = 0;
                    double y = 0;
                    double length_squared = 0;
                    while (repeats < 100000 && length_squared < 4) {
                        double temp = x * x - y * y + x0;
                        y = 2 * x * y + y0;
                        x = temp;
                        length_squared = x * x + y * y;
                        ++repeats;
                    }
                    image[counter * width + i] = repeats;
                    #if(DEBUG)
                    //printf("counter %d i %d thread %d\n", counter, i, omp_get_thread_num());
                    //sleep(1.5);
                    #endif
                }
            }
            info[0] = row;
            info[1] = heightSize;
            #if(DEBUG)
            printf("rank[%d] send result %d %d\n", rank, row, heightSize);
            #endif
            #if(TIME)
            startT = clock();
            #endif
            MPI_Send(info, 2, MPI_INT, 0, RESULT_TAG, MPI_COMM_WORLD);
            MPI_Send(image, heightSize * width, MPI_INT, 0, RESULT_TAG, MPI_COMM_WORLD);
            MPI_Recv(&row, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            #if(TIME)
            endT = clock();
            commTime += (float)(endT - startT)/CLOCKS_PER_SEC;
            #endif

        }

    }
    #if(TIME)
    float totalCommTime;
    float barrierTime=0, maxBarrierTime=0;
    startT = clock();
    MPI_Barrier(MPI_COMM_WORLD);
    endT = clock();
    barrierTime = (float)(endT - startT)/CLOCKS_PER_SEC;
    MPI_Reduce(&barrierTime, &maxBarrierTime, 1, MPI_FLOAT, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&commTime, &totalCommTime, 1, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);
    if(rank==0)
        printf("commtime = %f\nmaxBarrierTime = %f\n", totalCommTime/(commSize-1), maxBarrierTime);
    #endif
    
    free(image);
    MPI_Finalize();
}
