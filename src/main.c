#include <stdlib.h>
#include "csv.h"

int main()
{
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
	return 1;	
}



