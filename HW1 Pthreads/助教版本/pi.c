#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#define RANDMAX 2147483646

int tossN = 100000000;

void *func(void *arg)
{
    double *f = (double *)malloc(sizeof(double));
    int seed = *((int *)arg);
    int total = 0;
    unsigned int x = seed, y;
    int s = 0;
    for (int i = 0; i < tossN; i++)
    {
        x = (22695477 * seed + 1) & 0x7FFFFFFF;
        y = (2133350137 * seed + 22695478) & 0x7FFFFFFF;
        seed = y;
        total += (int)(((double)x * x + (double)y * y) / ((double)RANDMAX * RANDMAX));
    }
    *f = 4 * (total / (double)tossN);
    return (double *)f;
}

int main(int argv, char **argc)
{
    int coreN = 1;
    if (argv > 1)
    {
        coreN = atoi(argc[1]);
        if (argv == 3)
        {
            tossN = atoi(argc[2]);
        }
    }
    tossN = tossN / coreN;
    int arg[coreN];
    pthread_t th[coreN];
    srand(time(NULL));
    for (int i = 0; i < coreN; i++)
    {
        arg[i] = rand();
        pthread_create(&th[i], NULL, func, &arg[i]);
    }
    double f[coreN];
    for (int i = 0; i < coreN; i++)
    {
        double *joinedf;
        pthread_join(th[i], (void **)&joinedf);
        f[i] = *joinedf;
        free(joinedf);
    }
    double total = 0;
    for (int i = 0; i < coreN; i++)
        total += f[i];
    printf("%f\n", 4 - total / coreN);
    return 0;
}
