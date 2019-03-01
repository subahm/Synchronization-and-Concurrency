#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include "uthread.h"
#include "uthread_mutex_cond.h"
#include <pthread.h>

#define NUM_ITERATIONS 1000

#ifdef VERBOSE
#define VERBOSE_PRINT(S, ...) printf (S, ##__VA_ARGS__);
#else
#define VERBOSE_PRINT(S, ...) ;
#endif

struct Agent {
  pthread_mutex_t mutex;
  pthread_cond_t  match;
  pthread_cond_t  paper;
  pthread_cond_t  tobacco;
  pthread_cond_t  smoke;
};

struct Agent* createAgent() {
  struct Agent* agent = malloc (sizeof (struct Agent));
  pthread_mutex_init(&agent->mutex, NULL);
  pthread_cond_init(&agent->paper, NULL);
  pthread_cond_init(&agent->match, NULL);
  pthread_cond_init(&agent->tobacco, NULL);
  pthread_cond_init(&agent->smoke, NULL);
  return agent;
}

//
// TODO
// You will probably need to add some procedures and struct etc.
//
int sum = 0;
pthread_cond_t match_and_paper;
pthread_cond_t paper_and_tobacco;
pthread_cond_t match_and_tobacco;

/**
 * You might find these declarations helpful.
 *   Note that Resource enum had values 1, 2 and 4 so you can combine resources;
 *   e.g., having a MATCH and PAPER is the value MATCH | PAPER == 1 | 2 == 3
 */
enum Resource            {    MATCH = 1, PAPER = 2,   TOBACCO = 4};
char* resource_name [] = {"", "match",   "paper", "", "tobacco"};

int signal_count [5];  // # of times resource signalled
int smoke_count  [5];  // # of times smoker with resource smoked

/**
 * This is the agent procedure.  It is complete and you shouldn't change it in
 * any material way.  You can re-write it if you like, but be sure that all it does
 * is choose 2 random reasources, signal their condition variables, and then wait
 * wait for a smoker to smoke.
 */
void* agent (void* av) {
  struct Agent* a = av;
  static const int choices[]         = {MATCH|PAPER, MATCH|TOBACCO, PAPER|TOBACCO};
  static const int matching_smoker[] = {TOBACCO,     PAPER,         MATCH};

  pthread_mutex_lock (&a->mutex);
    for (int i = 0; i < NUM_ITERATIONS; i++) {
      int r = random() % 3;
      signal_count [matching_smoker [r]] ++;
      int c = choices [r];
      if (c & MATCH) {
        pthread_cond_signal (&a->match);
      }
      if (c & PAPER) {
        pthread_cond_signal (&a->paper);
      }
      if (c & TOBACCO) {
        pthread_cond_signal (&a->tobacco);
      }
      pthread_cond_wait (&a->smoke, &a->mutex);
    }
  pthread_mutex_unlock (&a->mutex);
  return NULL;
}

void start_smoker(int s){
  switch(s){
    case MATCH + PAPER :
      pthread_cond_signal(&match_and_paper);
      sum = 0;
      break;
    case PAPER + TOBACCO:
      pthread_cond_signal(&paper_and_tobacco);
      sum = 0;
      break;
    case MATCH + TOBACCO:
      pthread_cond_signal(&match_and_tobacco);
      sum = 0;
      break;
    default:
      // do nothing
      break;
  }
}

void* match_smoker (void* av) {
	struct Agent* a = av;
	pthread_mutex_lock(&a->mutex);
	while(1) {
		pthread_cond_wait(&a->match, &a->mutex);
    sum = sum + MATCH;
    start_smoker(sum);
	}
	pthread_mutex_unlock(&a->mutex);
	return NULL;
}

void* paper_smoker (void* av) {
	struct Agent* a = av;
	pthread_mutex_lock(&a->mutex);
	while(1) {
		pthread_cond_wait(&a->paper, &a->mutex);
    sum = sum + PAPER;
    start_smoker(sum);
	}
	pthread_mutex_unlock(&a->mutex);
	return NULL;
}

void* tobacco_smoker (void* av) {
	struct Agent* a = av;
	pthread_mutex_lock(&a->mutex);
	while(1) {
		pthread_cond_wait(&a->tobacco, &a->mutex);
    sum = sum + TOBACCO;
    start_smoker(sum);
	}
	pthread_mutex_unlock(&a->mutex);
	return NULL;
}


void* match_listener (void* av) {
	struct Agent* a = av;
	pthread_mutex_lock(&a->mutex);
	while(1) {
		pthread_cond_wait(&paper_and_tobacco, &a->mutex);
		pthread_cond_signal(&a->smoke);
		smoke_count [MATCH]++;
	}
	pthread_mutex_unlock(&a->mutex);
	return NULL;
}

void* paper_listener (void* av) {
	struct Agent* a = av;
	pthread_mutex_lock(&a->mutex);
	while(1) {
		pthread_cond_wait(&match_and_tobacco, &a->mutex);
		pthread_cond_signal(&a->smoke);
		smoke_count [PAPER]++;
	}
	pthread_mutex_unlock(&a->mutex);
	return NULL;
}

void* tobacco_listener (void* av) {
	struct Agent* a = av;
	pthread_mutex_lock(&a->mutex);
	while(1) {
		pthread_cond_wait(&match_and_paper, &a->mutex);
		pthread_cond_signal(&a->smoke);
		smoke_count [TOBACCO]++;
	}
	pthread_mutex_unlock(&a->mutex);
	return NULL;
}

int main (int argc, char** argv) {
  pthread_t match;
  pthread_t paper;
  pthread_t tobacco;
  pthread_t match_var;
  pthread_t paper_var;
  pthread_t tobacco_var;
  pthread_t agent_var;

  struct Agent*  a = createAgent();

  pthread_cond_init(&match_and_paper, NULL);
  pthread_cond_init(&match_and_tobacco, NULL);
  pthread_cond_init(&paper_and_tobacco, NULL);

  pthread_create(&match, NULL, match_listener, a);
  pthread_create(&paper, NULL, paper_listener, a);
  pthread_create(&tobacco, NULL, tobacco_listener, a);

  pthread_create(&match_var, NULL, match_smoker, a);
  pthread_create(&paper_var, NULL, paper_smoker, a);
  pthread_create(&tobacco_var, NULL, tobacco_smoker, a);

  pthread_create(&agent_var, NULL, agent, a);

  pthread_join (agent_var, NULL);

  assert (signal_count [MATCH]   == smoke_count [MATCH]);
  assert (signal_count [PAPER]   == smoke_count [PAPER]);
  assert (signal_count [TOBACCO] == smoke_count [TOBACCO]);
  assert (smoke_count [MATCH] + smoke_count [PAPER] + smoke_count [TOBACCO] == NUM_ITERATIONS);
  printf ("Smoke counts: %d matches, %d paper, %d tobacco\n",
          smoke_count [MATCH], smoke_count [PAPER], smoke_count [TOBACCO]);
}
