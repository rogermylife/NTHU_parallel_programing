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
    int inputFD = open(inputName, O_RDONLY);
    read(inputFD, input, n*sizeof(float));
    close(inputFD);
   
    for( int i=0;i<n;++i)
        printf("number[%d] %.100f\n", i, input[i]);
    

}
