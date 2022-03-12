#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <math.h>
#include "rtclock.h"
#include "mmm.h"

/* globals (anything here would be shared with all threads) */

int main(int argc, char *argv[])
{
	double clockstart, clockend;
	int size = 0, i, j;
	if (argc < 3 || argc > 4 || (strcmp(argv[1], "P") != 0 && strcmp(argv[1], "S") != 0)){
		printf("Usage: ./mmm <mode> [num threads] <size>\n");
		exit(0);
	}
	else if (strcmp(argv[1], "P") == 0 && argc == 3){
		printf("Error: parallel mode requires both [num threads] and <size>\n");
		exit(0);
	}
	else if (strcmp(argv[1], "S") == 0 && argc == 4){
		printf("Error: sequential mode does not require [num threads]\n");
		exit(0);
	}
	else if (strcmp(argv[1], "P") == 0 && (atoi(argv[2]) < 1 || atoi(argv[2]) > atoi(argv[3]))){
		printf("Error: number of threads should be between 1 and <size>\n");
		exit(0);
	}

	// display user choice
	printf("========\n");
	if (strcmp(argv[1], "P") == 0){
		printf("mode: parallel\n");
		printf("thread count: %d\n", atoi(argv[2]));
		size = atoi(argv[3]);
	}
	else{
		printf("mode: sequential\n");
		printf("thread count: 1\n");
		size = atoi(argv[2]);
	}
	printf("size: %d\n", size);
	printf("========\n");

	mmm_init(size);

	float t = 0; // temp variable for smoothing time taken results
	// run sequential n+1 times (n=3)
	for (j = 0; j < 4; j++){
		clockstart = rtclock(); // start clocking
		mmm_seq();
		clockend = rtclock(); // stop clocking
		if (j > 0){
			t += (clockend - clockstart);
		}
	}
	float seqtime = t / 3;
	printf("Sequential time: %.6f sec\n", seqtime);
	t = 0;

	// if requested, run parallel n+1 times (n=3)
	if (strcmp(argv[1], "P") == 0){
		// save sequential output matrix
		double **seqmatrix = (double **)malloc(size * sizeof(double *));
		for (j = 0; j < size; j++){
			seqmatrix[j] = (double *)malloc(size * sizeof(double));
		}
		for (i = 0; i < size; i++){
			for (j = 0; j < size; j++){
				seqmatrix[i][j] = output[i][j];
			}
		}

		mmm_reset(output);

		int numThreads = atoi(argv[2]);
		for (j = 0; j < 4; j++){
			clockstart = rtclock(); // start clocking

			// allocate space to hold threads
			pthread_t threads[numThreads];
			// prepare threads
			int i;
			thread_args args[numThreads];
			int tasksPerThread=floor((size+numThreads-1)/numThreads);
			for (i = 0; i < numThreads; i++){
				// prepare arguments for thread
				args[i].tid = i;
				args[i].begin = i * tasksPerThread;
				args[i].end = (i + 1) * tasksPerThread - 1;
				if (i == numThreads - 1){
					args[numThreads-1].end = size-1;
				}
				// launch thread
				pthread_create(&threads[i], NULL, mmm_par, &args[i]);
			}
			// wait for threads to finish
			for (i = 0; i < numThreads; i++){
				pthread_join(threads[i], NULL);
			}	

			clockend = rtclock(); // stop clocking
			if (j > 0){
				t += (clockend - clockstart);
			}
		}
		float partime = t / 3;
		printf("Parallel Time: %.6f sec\n", partime);
		float speedup = seqtime / partime;
		printf("Speedup: %.6f\n", speedup);
		double diff = mmm_verify(seqmatrix, output);
		printf("Verifying... largest error between parallel and sequential matrix: %.6f\n", diff);

		for (j = 0; j < size; j++){
			free(seqmatrix[j]);
		}
		free(seqmatrix);
	}

	// free memory
	mmm_freeup();

	return 0;
}
