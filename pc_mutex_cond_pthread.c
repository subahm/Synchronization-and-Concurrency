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

/* MY CODE ****************************************************************/
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t free_slot = PTHREAD_COND_INITIALIZER;
pthread_cond_t full_slot = PTHREAD_COND_INITIALIZER;
/* MY CODE ****************************************************************/

void* producer (void* v) {
  for (int i=0; i<NUM_ITERATIONS; i++) {
    // TODO
    /* MY CODE ****************************************************************/
    assert(0 <= items && items <= MAX_ITEMS);
    		/* get lock */
    		pthread_mutex_lock(&lock);

    		/* check if we need to wait for consumer to consume */
    		while(items == MAX_ITEMS){
    			producer_wait_count++;
    			pthread_cond_wait(&free_slot, &lock);
    		}

    		/* produce */
    		items++;
    		/* update histogram */
    		histogram[items]++;

    		/* signal */
    		pthread_cond_signal(&full_slot);

    		/* unlock */
    		pthread_mutex_unlock(&lock);
    /* MY CODE ****************************************************************/
  }
  return NULL;
}

void* consumer (void* v) {
  for (int i=0; i<NUM_ITERATIONS; i++) {
    // TODO
    /* MY CODE ****************************************************************/
    assert(0 <= items && items <= MAX_ITEMS);

		/* get lock */
		pthread_mutex_lock(&lock);

		/* check if we need to wait for producer to produce */
		while(items == 0){
			consumer_wait_count++;
			pthread_cond_wait(&full_slot, &lock);
		}

		/* consume */
		items--;
		/* update histogram */
		histogram[items]++;

		/* signal */
		pthread_cond_signal(&free_slot);
		/* unlock */
		pthread_mutex_unlock(&lock);
    /* MY CODE ****************************************************************/
  }
  return NULL;
}

int main (int argc, char** argv) {

  // TODO: Create Threads and Join
  /* MY CODE ****************************************************************/
  /* storage for threads */
  pthread_t t2[NUM_CONSUMERS + NUM_PRODUCERS];
  /* create threads */
  pthread_create(&t2[0], NULL, &consumer, NULL);
  pthread_create(&t2[1], NULL, &consumer, NULL);
  pthread_create(&t2[2], NULL, &producer, NULL);
  pthread_create(&t2[3], NULL, &producer, NULL);
  /* join threads */
  for(int i = 0;i < (NUM_CONSUMERS + NUM_PRODUCERS);i++){
  	pthread_join(t2[i], NULL);
  }
  /* MY CODE ****************************************************************/
  printf ("producer_wait_count=%d\nconsumer_wait_count=%d\n", producer_wait_count, consumer_wait_count);
  printf ("items value histogram:\n");
  int sum=0;
  for (int i = 0; i <= MAX_ITEMS; i++) {
    printf ("  items=%d, %d times\n", i, histogram [i]);
    sum += histogram [i];
  }
  assert (sum == sizeof (t2) / sizeof (pthread_t) * NUM_ITERATIONS);
}
