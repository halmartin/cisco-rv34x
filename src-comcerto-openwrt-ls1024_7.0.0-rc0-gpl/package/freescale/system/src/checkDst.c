#include<stdio.h>
#include<stdlib.h>	

int main (int argc, char **argv)
{
int cur_month=atoi(argv[1]);
int cur_day=atoi(argv[2]);
int cur_hour=atoi(argv[3]);
int cur_min=atoi(argv[4]);
int start_month=atoi(argv[5]);
int start_day=atoi(argv[6]);
int start_hour=atoi(argv[7]);
int start_min=atoi(argv[8]);
int end_month=atoi(argv[9]);
int end_day=atoi(argv[10]);
int end_hour=atoi(argv[11]);
int end_min=atoi(argv[12]);
int year=atoi(argv[13]);

unsigned int total_curr=0;
unsigned int total_start=0;
unsigned int total_end=0;

total_start = start_month << 24 | start_day << 16 | start_hour << 8 | start_min;
total_end = end_month << 24 | end_day << 16 | end_hour << 8 | end_min;
total_curr = cur_month << 24 | cur_day << 16 | cur_hour << 8 | cur_min;

if (total_start < total_end) {
	if ((total_curr >= total_start) && (total_curr < total_end))
		exit (1);
	else
		exit (0);
} else {
	if ((total_curr < total_end) || (total_curr >= total_start))
		exit (1);
	else
		exit (0);
}
return 0;
}
