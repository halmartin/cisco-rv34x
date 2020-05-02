#include<stdio.h>
#include<stdlib.h>	

const unsigned short int __mon_yday[2][13] =
{     
    /* Normal years.  */
    { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
    /* Leap years.  */
    { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }
};

# define __isleap(year) \
    ((year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0))
#define MIN 60
#define HOUR 3600

struct dst {
    int  start_year;
    int  start_month;
    int  start_day;
    int  start_hour;
    int  start_min;
    int  start_week;
    int  end_year;
    int  end_month;
    int  end_day;
    int  end_hour;
    int  end_min;
    int  end_week;
    int  offset;
} dst_t = {0} ;

int get_year_of_day_from_reccuring (int * y,int * m,int * w,int *week_day) {
    int week;
    int day;
    int year;
    int month;
    int days_before_month=0;
    unsigned int i;
    int d, m1, yy0, yy1, yy2, dow;
    time_t t=0;
    year=*y;
    month=*m;
    week=*w;
    day=*week_day;

    const unsigned short int *myday =&__mon_yday[__isleap (year)][month];
    t += myday[-1];
    /* Use Zeller's Congruence to get day-of-week of first day of month. */
    m1 = (month + 9) % 12 + 1;
    yy0 = (month <= 2) ? (year - 1) : year;
    yy1 = yy0 / 100;
    yy2 = yy0 % 100;
    dow = ((26 * m1 - 2) / 10 + 1 + yy2 + yy2 / 4 + yy1 / 4 - 2 * yy1) % 7;
    if (dow < 0)
	dow += 7;

    /* DOW is the day-of-week of the first day of the month.  Get the
       day-of-month (zero-origin) of the first DOW day of the month.  */
    d = day - dow;
    if (d < 0)
	d += 7;
    for (i = 1; i < week; ++i)
    {
	if (d + 7 >= (int) myday[0] - myday[-1])
	    break;
	d += 7;
    }

    /* D is the day-of-month (zero-origin) of the day we want.  */
    t += d;
    *week_day=t+1;

    /* Convert year of day to month of day*/
    myday = &__mon_yday[__isleap (year)][0];
    days_before_month = myday[month-1];
    *week_day-=days_before_month;
}

void cal_julian (struct dst * dst_ptr) {
	int start_total=0;
	int end_total=0;

	const unsigned short int *startmonth =&__mon_yday[__isleap (dst_ptr->start_year)][dst_ptr->start_month];
	const unsigned short int *endmonth =&__mon_yday[__isleap (dst_ptr->end_year)][dst_ptr->end_month];

	int startday=startmonth[-1] + dst_ptr->start_day;
	int endday=endmonth[-1] + dst_ptr->end_day;
	int extraday = 24 * HOUR;

	if (((startday - endday) == 1) || ((endday - startday) == 1) || (startday == endday)) {
		start_total = ((dst_ptr->start_hour) * HOUR) + ((dst_ptr->start_min) * MIN);
		end_total = ((dst_ptr->end_hour) * HOUR) + ((dst_ptr->end_min) * MIN);

		if (endday > startday)
			end_total += extraday;
		else if (startday > endday)
			start_total += extraday;

		if (end_total >= start_total) {
			if (dst_ptr->offset >=0)
				start_total += ((dst_ptr->offset) * MIN);

			if ((start_total + 60) > end_total)
				exit (1);
		}
		else {
			if (dst_ptr->offset < 0) {
				dst_ptr->offset = -dst_ptr->offset;
				end_total += ((dst_ptr->offset) * MIN);
			}
			if ((end_total + 60) > start_total)
				exit (1);
		}
	}
}


void cal_recurring (struct dst * dst_ptr) {
	int start_total=0;
	int end_total=0;
    	
	/* First get the year of day*/
    	get_year_of_day_from_reccuring (&dst_ptr->start_year, &dst_ptr->start_month, &dst_ptr->start_week, &dst_ptr->start_day);
    	get_year_of_day_from_reccuring (&dst_ptr->end_year, &dst_ptr->end_month, &dst_ptr->end_week, &dst_ptr->end_day);

	const unsigned short int *startmonth =&__mon_yday[__isleap (dst_ptr->start_year)][dst_ptr->start_month];
	const unsigned short int *endmonth =&__mon_yday[__isleap (dst_ptr->end_year)][dst_ptr->end_month];
	
	int startday=startmonth[-1] + dst_ptr->start_day;
        int endday=endmonth[-1] + dst_ptr->end_day;
	int extraday = 24 * HOUR;
	
	if (((startday - endday) == 1) || ((endday - startday) == 1) || (startday == endday)) {
		start_total = ((dst_ptr->start_hour) * HOUR) + ((dst_ptr->start_min) * MIN);
		end_total = ((dst_ptr->end_hour) * HOUR) + ((dst_ptr->end_min) * MIN);
	
		if (endday > startday)
			end_total += extraday;
		else if (startday > endday)
			start_total += extraday;

		if (end_total >= start_total) {
			if (dst_ptr->offset >=0)
				start_total += ((dst_ptr->offset) * MIN);

			if ((start_total + 60) > end_total)
				exit (1);
		}
		else {
			if (dst_ptr->offset < 0) {
				dst_ptr->offset = -dst_ptr->offset;
				end_total += ((dst_ptr->offset) * MIN);
			}
			if ((end_total + 60) > start_total)
				exit (1);
		}
	}
}


int main (int argc, char **argv)
{
    struct dst * dst_ptr = &dst_t;

    if ((argc >= 2) && (strcmp(argv[1], "Julian") == 0) && (argc != 12)) {
	fprintf (stdout,"Usage: /usr/bin/check_dsttime Julian <year> <start_month> <start_day> <start_hour> <start_min> <end_month> <end_day> <end_hour> <end_min> <offset> \n");
	exit (1);
    } else if ((argc >= 2) && (strcmp(argv[1], "Recurring") == 0) && (argc != 14)) {
	fprintf (stdout,"Usage: /usr/bin/check_dsttime Recurring <year> <start_month> <start_week> <start_day> <start_hour> <start_min> <end_month> <end_week> <end_day> <end_hour> <end_min> <offset>\n");
	exit (1);
    } else if (argc < 12) {
   	fprintf (stdout,"Invalid Usage. Please execute \"/usr/bin/check_dsttime Julian|Recurring --help\" for all options.\n");
	exit (1);
    }

    if ((strcmp(argv[1], "Julian") == 0) && (argc == 12))  {
	dst_ptr->start_year=atoi(argv[2]);
	dst_ptr->end_year=dst_ptr->start_year;
	dst_ptr->start_month=atoi(argv[3]);
	dst_ptr->start_day=atoi(argv[4]);
	dst_ptr->start_hour=atoi(argv[5]);
	dst_ptr->start_min=atoi(argv[6]);
	dst_ptr->end_month=atoi(argv[7]);
	dst_ptr->end_day=atoi(argv[8]);
	dst_ptr->end_hour=atoi(argv[9]);
	dst_ptr->end_min=atoi(argv[10]);
	dst_ptr->offset=atoi(argv[11]);

    } else if ((strcmp(argv[1], "Recurring") == 0) && (argc == 14)) {
	dst_ptr->start_year=atoi(argv[2]);
	dst_ptr->end_year=dst_ptr->start_year;
	dst_ptr->start_month=atoi(argv[3]);
	dst_ptr->start_week=atoi(argv[4]);
	dst_ptr->start_day=atoi(argv[5]);
	dst_ptr->start_hour=atoi(argv[6]);
	dst_ptr->start_min=atoi(argv[7]);
	dst_ptr->end_month=atoi(argv[8]);
	dst_ptr->end_week=atoi(argv[9]);
	dst_ptr->end_day=atoi(argv[10]);
	dst_ptr->end_hour=atoi(argv[11]);
	dst_ptr->end_min=atoi(argv[12]);
	dst_ptr->offset=atoi(argv[13]);
    } else {
	fprintf (stdout,"Invalid Usage. Please execute \"/usr/bin/check_dsttime Julian|Recurring --help\" for all options.\n");
	exit (1);
	}

    if (strcmp(argv[1], "Julian") == 0)
	cal_julian (dst_ptr);
    else if (strcmp(argv[1], "Recurring") == 0)
	cal_recurring (dst_ptr);

    exit (0);
}
