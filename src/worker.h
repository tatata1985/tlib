/*
* Copyright (c) 2018 tatata1985
* All rights reserved.
*/

#ifndef ___WORKER_COMMON_CODE___
#define ___WORKER_COMMON_CODE___
#ifdef __X86_64
	#include <semaphore.h> /* sem_t */
#else
	#include "my_semaphore.h" /* sem_t */
#endif

struct stworker
{
	int id;
	int available;// 0:reserved or working now, 1:available
	char *bascket_buff;
	int bascket_buff_size;
	sem_t sem_me;
	int (*mycallback)(void*);
	sem_t *queue;
	int *dissolution;
#ifdef __X86_64

#else
	TaskHandle_t thandle;
#endif
};

struct guild
{
	char name[31];
	int workercount;
	int dissolution;
	sem_t queue;
	struct stworker *worker;
};

void create_society();
void collapse_society();

struct guild *create_guild(char*, int);
struct guild *create_guild_delux(char*, int, int*);
void guild_terminate(struct guild *);

int reserve_worker(struct guild*);
void cancel_reserve_worker(struct guild*, int);

int set_worker_basket(struct guild*, int, void*, int, int (*)(void*));
void run_worker(struct guild*, int);
int isbusy(struct guild*);

#endif /* ___WORKER_COMMON_CODE___ */

