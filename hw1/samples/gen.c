#include<stdio.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<assert.h>
#include<time.h>

int main(int argc, char** argv)
{
    srand((unsigned int)time(NULL));
    assert(argc == 3);
    int n = atoi(argv[1]);
    float *output = (float*)malloc( sizeof(float) * (n+1) );
    for(int i=0;i<n;i++)
        output[i] = ((float)rand()/(float)(RAND_MAX)) * 1000000.0;
    char fileName[20];
    sprintf(fileName, "testcase%s", argv[2]);
    mode_t mode = S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH;
    int outputFD = open(fileName, O_WRONLY|O_CREAT|O_TRUNC, mode);
    write(outputFD, output, n*sizeof(float));
    close(outputFD);
}
