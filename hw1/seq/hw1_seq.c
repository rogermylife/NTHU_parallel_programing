#include<stdio.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<algorithm>
int main(int argc, char* argv[])
{
    int n = atoi(argv[1]);
    float *input=(float*)malloc((n+1)*sizeof(float));
    char* inputName = argv[2];
    char* outputName = argv[3];
    int inputFD = open(inputName, O_RDONLY);
    mode_t mode = S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH;
    int outputFD = open(outputName, O_WRONLY|O_CREAT|O_TRUNC, mode);
    read(inputFD, input, n*sizeof(float));
    close(inputFD);
   /* 
    for( int i=0;i<n;++i)
        printf("number[%d] %f\n", i, input[i]);
    */
    std::sort(input, input+n);
     
    for( int i=0;i<100;++i)
        printf("number[%d] %f\n", i, input[i]);
    
    write(outputFD, input, n*sizeof(float));
    close(outputFD);

}
