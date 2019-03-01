#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "uthread.h"
#include "uthread_sem.h"
#include <pthread.h>
#include <semaphore.h>

#define MAX_ITEMS 10
const int NUM_ITERATIONS = 200;
const int NUM_CONSUMERS  = 2;
const int NUM_PRODUCERS  = 2;

int histogram [MAX_ITEMS+1]; // histogram [i] == # of times list stored i items

int items = 0;
sem_t mutex;
sem_t condc;
sem_t condp;

#define NUM_THREADS 4

void* producer (void* v) {
  for (int i=0; i<NUM_ITERATIONS; i++) {
    // TODO
    while (items >= MAX_ITEMS){
      sem_wait(&condc);
		  sem_wait(&mutex);
    }
    //assert(items < MAX_ITEMS);
		items++;
		histogram[items]++;
		sem_post(&mutex);
		sem_post(&condp);
  }
  return NULL;
}

void* consumer (void* v) {
  for (int i=0; i<NUM_ITERATIONS; i++) {
    // TODO
    while(items <= 0){
      sem_wait(&condp);
		  sem_wait(&mutex);
    }
    //assert(items > 0);
		items--;
		histogram[items]++;
		sem_post(&mutex);

		sem_post(&condc);
  }
  return NULL;
}

int main (int argc, char** argv) {

  // TODO: Create Threads and Join
  sem_init(&mutex, 0, 1);
	sem_init(&condp, 0, 0);
	sem_init(&condc, 0, MAX_ITEMS);
  pthread_t t[NUM_THREADS];

  int i;
  for(i=0; i<NUM_THREADS; i++){
    if(i%2==0)
      pthread_create(&t[i], NULL, producer, NULL);
    else
      pthread_create(&t[i], NULL, consumer, NULL);
  }
  for(i=0; i<NUM_THREADS; i++){
    pthread_join(t[i], NULL);
  }

  printf ("items value histogram:\n");
  int sum=0;
  for (int i = 0; i <= MAX_ITEMS; i++) {
    printf ("  items=%d, %d times\n", i, histogram [i]);
    sum += histogram [i];
  }
  assert (sum == sizeof (t) / sizeof (sem_t) * NUM_ITERATIONS);
}
