#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <unistd.h>

#define RUNS 100000
#define SAMPLES 100000

int main()
{
	union datum
	{
		char str [4];
		unsigned int num;
	}shortSample [RUNS];

	union data
	{
		char str [8];
		unsigned long long num;
	}longSample [RUNS];

	srand(time(NULL));

//	shortStr shortSample [RUNS];
//	longStr longSample [RUNS];
	int i;
	for(i = 0; i < RUNS; i++)
	{
		shortSample[i].num = rand();
		shortSample[i].num >>= 8;
		unsigned long long temp = (unsigned long long)rand();
		temp <<=32;
		longSample[i].num = temp + ((unsigned long long)rand());
		longSample[i].num >>= 8;
	}

/*	shortSample[0].num = 0x00414141;
	shortSample[1].num = 0x004f4259;
	longSample[0].num = 0x0041414141414141;
	longSample[1].num = 0x0048524953544121;

	for(i = 0; i < RUNS; i++)
	{
		printf("Short String: %s and its int: %i \n",shortSample[i].str,shortSample[i].num);
		printf("long String: %s and its int: %lli \n",longSample[i].str,longSample[i].num);
	}
*/
	double sstr = 0;
	double snum = 0;
	double lstr = 0;
	double lnum = 0;
	clock_t start,end;
	clock_t as,ae;
	as = clock();
	pid_t whois = 0;

	int j;

	whois = fork();
	if(whois == 0)
	{
		start = clock();
		for(j = 0; j < SAMPLES; j++)
		{
			for(i = 0; i < RUNS; i++)
				strcmp(shortSample[i].str,shortSample[i].str);
			end = clock();
		}
		sstr += ((double)end - start)/CLOCKS_PER_SEC;
		printf("Short String time: 	%f\n",sstr);
		return(0);
		
	}
	whois = fork();
	if(whois == 0)
	{
		start = clock();
		for(j = 0; j < SAMPLES; j++)
		{
			for(i = 0; i < RUNS; i++)
				strcmp(longSample[i].str,longSample[i].str);
			end = clock();
		}
		lstr += ((double)end - start)/CLOCKS_PER_SEC;
		printf("Long String time: 	%f\n",lstr);
		return(0);
	}
	whois = fork();
	if(whois == 0)
	{
		start = clock();
		for(j = 0; j < SAMPLES; j++)
		{
			for(i = 0; i < RUNS; i++)
				shortSample[i].num == shortSample[i].num;
			end = clock();
		}
		snum += ((double)end - start)/CLOCKS_PER_SEC;
		printf("Short Int time: 	%f\n",snum);
		return(0);
	}
	whois = fork();
	if(whois == 0)
	{	
		start = clock();
		for(j = 0; j < SAMPLES; j++)
		{
			for(i = 0; i < RUNS; i++)
				longSample[i].num == longSample[i].num;
			end = clock();
		}
		lnum += ((double)end - start)/CLOCKS_PER_SEC;
		printf("Long Int time: 		%f\n",lnum);
		return(0);
	}
	ae = clock();
	
	double all = ((double)ae-as)/CLOCKS_PER_SEC;
	printf("And the results: \n");
//	printf("Short String time: 	%f\n",sstr);
//	printf("Short Int time: 	%f\n",snum);
//	printf("Speed up: 		%f% \n",(sstr/snum));
//	printf("Long String time: 	%f\n",lstr);
//	printf("Long Int time: 		%f\n",lnum);
//	printf("Speed up: 		%f% \n",(lstr/lnum));
	printf("Overall time: 		%f\n",all);
	printf("woohoo!\n");
}
