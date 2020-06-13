/*
* Copyright (c) 2018 tatata1985
* All rights reserved.
*/
#include <pthread.h> /* pthread_create */
#include <unistd.h> /* sleep */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "worker.h"
sem_t sem_wakeup;

void create_society()
{
	sem_init(&sem_wakeup, 0, 0);
}

void collapse_society()
{
	sem_destroy(&sem_wakeup);
}

void *worker_thread(void* arg)
{
	struct stworker *me = (struct stworker*)arg;
	me->available = 0;
	me->bascket_buff_size = 512;
	me->bascket_buff = (char*)malloc(
		me->bascket_buff_size);

	sem_init(&me->sem_me, 0, 0);

	/* ready */
sem_post(&sem_wakeup);
	for (;;) {
sem_wait(&(*me->queue));
		me->available = 1;// available
sem_post(&(*me->queue));

		/* Line up(the queue */
sem_wait(&me->sem_me);
		if (*me->dissolution != 0)
			goto out;
		me->mycallback((void*)me->bascket_buff);
	}
out:
	free(me->bascket_buff);
	sem_destroy(&me->sem_me);
	return NULL;
}

struct guild *create_guild(char* name, int workercount)
{
	struct guild *gl;
	int i = 0;

	if (workercount < 1 || workercount > 100)
		goto errout;

	gl = (struct guild*)malloc(sizeof(struct guild));
	if (gl == NULL)
		goto errout;

	gl->workercount = workercount;
	gl->dissolution = 0;

	sem_init(&gl->queue, 0, 0);
sem_post(&gl->queue);
	gl->worker = (struct stworker*)malloc(sizeof(struct stworker) * gl->workercount);
	while (i < gl->workercount) {
		pthread_t tmptid;
		gl->worker[i].queue = &gl->queue;
		gl->worker[i].id = i;
		gl->worker[i].dissolution = &gl->dissolution;
#ifdef __X86_64
		if (pthread_create(&tmptid,
			NULL,
			worker_thread,
			(void*)&gl->worker[i]) == -1)
			goto errout;
#else
		char taskname[16];
		sprintf(taskname, "%s_%s_%d", name, "work", i);
		xTaskCreatePinnedToCore(
			worker_thread,
			taskname,
			28192,
			(void*)&gl->worker[i],
			1,
			&gl->worker[i].thandle,
			1);
#endif
sem_wait(&sem_wakeup);
		i++;
	}
strcpy(gl->name, name);

	return gl;
errout:
	printf("worker error out\n");
	return NULL;
}

struct guild *create_guild_delux(char* name, int workercount, int* stacksize)
{
	struct guild *gl;
	int i = 0;

	if (workercount < 1 || workercount > 100)
		goto errout;

	gl = (struct guild*)malloc(sizeof(struct guild));
	if (gl == NULL)
		goto errout;

	gl->workercount = workercount;
	gl->dissolution = 0;

	sem_init(&gl->queue, 0, 0);
	
sem_post(&gl->queue);

	gl->worker = (struct stworker*)malloc(sizeof(struct stworker) * gl->workercount);
	while (i < gl->workercount) {
		pthread_t tmptid;
		gl->worker[i].queue = &gl->queue;
		gl->worker[i].id = i;
		gl->worker[i].dissolution = &gl->dissolution;

#ifdef __X86_64
		if (pthread_create(&tmptid,
			NULL,
			worker_thread,
			(void*)&gl->worker[i]) == -1)
			goto errout;
#else
		char taskname[16];
		sprintf(taskname, "%s_%s_%d", name, "work", i);
		xTaskCreatePinnedToCore(
			worker_thread,
			taskname,
			//28192,
			stacksize[i],
			(void*)&gl->worker[i],
			1,
			&gl->worker[i].thandle,
			1);
#endif

sem_wait(&sem_wakeup);
		i++;
	}
strcpy(gl->name, name);

	return gl;
errout:
	printf("worker error out\n");
	return NULL;
}

void guild_terminate(struct guild * gl)
{
	gl->dissolution = 1;

	int i = 0;
	while (i < gl->workercount) {
		sem_post(&gl->worker[i].sem_me);
		i++;
	}

	sleep(2);

	free(gl->worker);
	sem_destroy(&gl->queue);
}

void run_worker(struct guild * gl, int id)
{
	sem_post(&gl->worker[id].sem_me);
}

void cancel_reserve_worker(struct guild * gl, int id)
{
sem_wait(&gl->queue);
	gl->worker[id].available = 1;/* available */
sem_post(&gl->queue);
	return;
}

int isbusy(struct guild * gl)
{
sem_wait(&gl->queue);
	int busy = 0;
	int i = 0;
	while (i < gl->workercount) {
		if (gl->worker[i].available != 1) {
			/* If any thread is working */
			busy = 1;
			break;
		}
		i++;
	}
sem_post(&gl->queue);	
	return busy;
}

int reserve_worker(struct guild * gl)
{
	int i = 0;
	int retrycount = 0;
waitloop:
	i = 0;

sem_wait(&gl->queue);

	while (i < gl->workercount) {
		if (gl->worker[i].available == 1)
			break;
		i++;
	}

	if (i >= gl->workercount) {
sem_post(&gl->queue);
		retrycount++;
		if (retrycount > 3) {
			//	printf("order worker fail here1 %d\n", i);
			//	return -1;
		}
		usleep(1);
		goto waitloop;
	}

	gl->worker[i].available = 0;// reserve
sem_post(&gl->queue);

	return i;
}

int set_worker_basket(struct guild * gl, int id, void* basket, int bascketsize, int (*mycallback)(void*))
{
	if (gl->worker[id].bascket_buff_size < bascketsize) {
		char* tmp = (char*)realloc(
			gl->worker[id].bascket_buff, bascketsize);
		if (tmp == NULL) {
			goto errorout;
		} else {
			gl->worker[id].bascket_buff = tmp;
			gl->worker[id].bascket_buff_size = bascketsize;
		}
	}

	memcpy(gl->worker[id].bascket_buff, basket, bascketsize);
	gl->worker[id].mycallback = mycallback;

	return 1;
errorout:
	return -1;
}

