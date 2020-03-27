#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <math.h>
typedef struct DataSet{
	long long PartialSum;
	long long Cnt;
} DataSetStruct, *pDataSetStruct;

void* PiFunc(void *DataSet);

int main(int argc, char *argv[])
{
	char *LoopCntEnd,*ThreadCntEnd;
	long long ThreadCnt,TotalLoopCnt,Hits=0,LoopCnt;
	double Pi;
	pDataSetStruct DateSetTemp;
	pthread_t *Threads;

	LoopCnt = strtoll(argv[2],&LoopCntEnd,10);	
	ThreadCnt = strtoll(argv[1],&ThreadCntEnd,10);
	TotalLoopCnt = LoopCnt/ThreadCnt;
	DateSetTemp = (DataSetStruct*)malloc(sizeof(DataSetStruct)*ThreadCnt);
	Threads = (pthread_t *)malloc(sizeof(pthread_t)*ThreadCnt);
	for(long long i=0 ; i<ThreadCnt ; i++){
		DateSetTemp[i].Cnt = TotalLoopCnt;
		pthread_create(&Threads[i],NULL,PiFunc,(void*)&DateSetTemp[i]);
	}
	for(long long i=0;i<ThreadCnt;i++){pthread_join(Threads[i], NULL);}
	for(long long i=0;i<ThreadCnt;i++){Hits=Hits+DateSetTemp[i].PartialSum;}
	Pi = (4.0f*Hits/LoopCnt);
	free(DateSetTemp);
	free(Threads);
	printf("%f\n", Pi );
	return 0;
}

void* PiFunc(void *DataSet){
	unsigned int Seed;
	double XPointTemp, YPointTemp;
	long long PartialSum=0;
	pDataSetStruct DataSetTemp;
	
	DataSetTemp = (DataSetStruct *)DataSet;
	Seed =  (unsigned int)time(NULL);

	for(int i=0;i<DataSetTemp->Cnt;i++){
		XPointTemp = ((double)rand_r(&Seed)/(RAND_MAX/2.0f))-1;
		YPointTemp = ((double)rand_r(&Seed)/(RAND_MAX/2.0f))-1;
		if( (XPointTemp*XPointTemp)+(YPointTemp*YPointTemp)<=1.0 ){PartialSum++;}
	}
	DataSetTemp->PartialSum = PartialSum;	
	return NULL;
}
