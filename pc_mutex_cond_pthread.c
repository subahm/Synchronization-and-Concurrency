#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "uthread.h"
#include "uthread_mutex_cond.h"
#include "spinlock.h"
#include <pthread.h>

#define MAX_ITEMS 10
const int NUM_ITERATIONS = 200;
const int NUM_CONSUMERS  = 2;
const int NUM_PRODUCERS  = 2;

int producer_wait_count;     // # of times producer had to wait
int consumer_wait_count;     // # of times consumer had to wait
int histogram [MAX_ITEMS+1]; // histogram [i] == # of times list stored i items

int items = 0;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condc = PTHREAD_COND_INITIALIZER;
pthread_cond_t condp = PTHREAD_COND_INITIALIZER;

void* producer (void* v) {
  for (int i=0; i<NUM_ITERATIONS; i++) {
    // TODO
    assert(0 <= items && items <= MAX_ITEMS);
    pthread_mutex_lock(&lock);
    while(items == MAX_ITEMS){
      producer_wait_count++;
    	pthread_cond_wait(&condc, &lock);
    }
    items++;
    histogram[items]++;
    pthread_cond_signal(&condp);
    pthread_mutex_unlock(&lock);
    assert(0 <= items && items <= MAX_ITEMS);
  }
  return NULL;
}

void* consumer (void* v) {
  for (int i=0; i<NUM_ITERATIONS; i++) {
    // TODO
    assert(0 <= items && items <= MAX_ITEMS);
		pthread_mutex_lock(&lock);
		while(items == 0){
			consumer_wait_count++;
			pthread_cond_wait(&condp, &lock);
		}
		items--;
		histogram[items]++;
		pthread_cond_signal(&condc);
		pthread_mutex_unlock(&lock);
    assert(0 <= items && items <= MAX_ITEMS);
  }
  return NULL;
}

int main (int argc, char** argv) {

  // TODO: Create Threads and Join
  pthread_t t[NUM_CONSUMERS + NUM_PRODUCERS];
  pthread_create(&t[0], NULL, &consumer, NULL);
  pthread_create(&t[1], NULL, &consumer, NULL);
  pthread_create(&t[2], NULL, &producer, NULL);
  pthread_create(&t[3], NULL, &producer, NULL);

  for(int i = 0;i < (NUM_CONSUMERS + NUM_PRODUCERS);i++){
  	pthread_join(t[i], NULL);
  }

  printf ("producer_wait_count=%d\nconsumer_wait_count=%d\n", producer_wait_count, consumer_wait_count);
  printf ("items value histogram:\n");
  int sum=0;
  for (int i = 0; i <= MAX_ITEMS; i++) {
    printf ("  items=%d, %d times\n", i, histogram [i]);
    sum += histogram [i];
  }
  assert (sum == sizeof (t) / sizeof (pthread_t) * NUM_ITERATIONS);
}
