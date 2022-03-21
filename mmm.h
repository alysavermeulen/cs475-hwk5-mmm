#ifndef MMM_H_
#define MMM_H_

extern int size; // size of the square matrix (e.g. 1000 for 1000 x 1000 matrix)
extern double **m1; // first input matrix
extern double **m2; // second input matrix
extern double **output; // output matrix

/** struct to hold parameters to mmm_par() **/
typedef struct thread_args
{
  int tid;    // the given thread id (0, 1, ...)
  int begin; // index of first output row to compute
  int end;   // index of last output row to compute
} thread_args;

void mmm_init(int s);
void mmm_reset(double **matrix);
void mmm_freeup();
void mmm_seq();
void *mmm_par(void *args);
double mmm_verify(double **output1, double **output2);

#endif /* MMM_H_ */
