#include <stdlib.h>
#include "csv.h"
#include "worker.h"

#define TEST_GROUP "TS"
#define TEST_MEMBER_COUNT 10

struct our_basket
{
	int id;
	int portno;
	char buff[32];
};
struct our_basket ob[TEST_MEMBER_COUNT];
struct guild *g_guild;
	
int our_worker(void* arg)
{
	struct our_basket sb;
	memcpy(&sb, arg, sizeof(struct our_basket));

	printf("### server_worker %d wakeup id:%d ###\n",
		sb.portno, sb.id);

	struct csvdata csv;
	csv_init(&csv, 8); // field count
	csv.path = "./test.csv";
	csv_load(&csv);

	printf("field:%ld tuples:%ld\n", csv.fields, csv.tuples);
	int f = 0;
	int t = 0;
	for(t = 0;t < csv.tuples-1;t++) {
		for(f = 0;f <= csv.fields;f++) {
			if (f != csv.fields)
				printf("%s", csv_getter(&csv, t, f));

			if (f < csv.fields - 1)
				printf(",");
		}
		printf("\n");
	}
	csv_detach(&csv);
	

	return 0;
}	
		
void call_worker()
{
	int i = 0;
	while (i < 100) {
		int workerid = reserve_worker(g_guild);
		printf("i:%d workerid:%d\n", i, workerid);
		
		memset(&ob[workerid], 0, sizeof(struct our_basket));
		ob[workerid].portno = 5000;
		ob[workerid].id = workerid;
		
		set_worker_basket(g_guild,
			workerid,
			(void*)&ob[workerid],
			sizeof(struct our_basket), our_worker);
		
		run_worker(g_guild, workerid);
		i++;
	}
	
	printf("sleeping....\n");
	while (0 != isbusy(g_guild)) {
		usleep(10);
	}

	guild_terminate(g_guild);
}

int main()
{
	create_society();
	g_guild = create_guild((char*)TEST_GROUP, TEST_MEMBER_COUNT); /* name, worker count */
	if (g_guild == NULL) {
		printf("create guild error.\n");
		goto error;
	}
	
	call_worker();

error:
	return 1;	
}



