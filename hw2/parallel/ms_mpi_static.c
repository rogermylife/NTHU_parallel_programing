#define PNG_NO_SETJMP
#define DEBUG 0

#include <assert.h>
#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <time.h>

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

    /* MPI init */
    int commSize, rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &commSize);
    int startHeight = 0, heightSize=0, mod;
    mod = height%commSize;
    heightSize = (mod > rank ) ? (height/commSize+1) : height/commSize;
    startHeight = height/commSize*rank;
    startHeight += (mod>rank) ? rank : mod;
    
    /* allocate memory for image */
    int* image = (int*)malloc(width * heightSize * sizeof(int));
    int *buddyImage;
    assert(image);

    /* mandelbrot set */
    for (int j = startHeight, counter = 0; counter < heightSize; ++counter, ++j) {
        double y0 = j * ((upper - lower) / height) + lower;
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
        }
    }
    # if(DEBUG)
    printf("rank[%d] starting merge\n", rank);
    # endif
    for(int i=2,padding=1; padding<=commSize; i*=2,padding*=2)
    {
        if(rank%i==0 && rank+padding<commSize)
        {
            int buddyHeightSize;
            MPI_Recv(&buddyHeightSize, 1, MPI_INT, rank+padding, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            buddyImage = (int*)malloc(width * buddyHeightSize * sizeof(int));
            MPI_Recv(buddyImage, buddyHeightSize * width, MPI_INT, rank+padding, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            image = (int*)realloc(image, width*(heightSize+buddyHeightSize)*sizeof(int) );
            memcpy( image+heightSize*width, buddyImage, width*buddyHeightSize*sizeof(int));
            heightSize += buddyHeightSize;
        }
        else if(rank % i == padding)
        {
            MPI_Send(&heightSize, 1, MPI_INT, rank-padding, 0, MPI_COMM_WORLD);
            MPI_Send(image, heightSize * width, MPI_INT, rank-padding, 0, MPI_COMM_WORLD);
        }
    }
    /* draw and cleanup */
    if(rank==0)
        write_png(filename, width, height, image);
    free(image);

    MPI_Finalize();
}
