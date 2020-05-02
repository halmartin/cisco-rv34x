/*
 * iptables-uci.c - sample code for the ucimap/libuci library
 * 
 */
#include "iptables-uci.h"

#include "xtables.h"
#include "libiptc/libiptc.h"
#include "iptables-multi.h"

struct list_head allIfaces;
struct list_head allowurlRules;
struct list_head allowkeywordRules;
struct list_head blockurlRules;
struct list_head blockkeywordRules;

/* global new argv and argc */
extern char *mynewargv[255];
extern int mynewargc;
extern int enable_debug;
//static char * tablename="filter";
//static char * nat_tablename="nat";

void print_allowURLRule(struct uci_allowurlRule *au)
{
	int a=0;

	printf("Details of argc and argv passed to do_command are:\r\n");
	for (a = 0; a < mynewargc; a++)
		printf("argv[%u]: %s\t", a, mynewargv[a]);
	printf("\r\n");
	if(au)
		printf("Allow URL record '%s' is\n"
			"	domain_name:	%s\n"
			"	schedule:	%s\n",
			au->name,
			au->domain_name,
			au->schedule);
}

void print_blockURLRule(struct uci_blockurlRule *bu)
{
	int a=0;

	printf("Details of argc and argv passed to do_command are:\r\n");
	for (a = 0; a < mynewargc; a++)
		printf("argv[%u]: %s\t", a, mynewargv[a]);
	printf("\r\n");
	if(bu)
		printf("Block URL record '%s' is\n"
			"	domain_name:	%s\n"
			"	schedule:	%s\n",
			bu->name,
			bu->domain_name,
			bu->schedule);
}

void print_allowKeywordRule(struct uci_allowkeywordRule *ak)
{
	int a=0;

	printf("Details of argc and argv passed to do_command are:\r\n");
	for (a = 0; a < mynewargc; a++)
		printf("argv[%u]: %s\t", a, mynewargv[a]);
	printf("\r\n");
	if(ak)
		printf("Allow Keyword record '%s' is\n"
			"	keyword:	%s\n"
			"	schedule:	%s\n",
			ak->name,
			ak->keyword,
			ak->schedule);
}

void print_blockKeywordRule(struct uci_blockkeywordRule *bk)
{
	int a=0;

	printf("Details of argc and argv passed to do_command are:\r\n");
	for (a = 0; a < mynewargc; a++)
		printf("argv[%u]: %s\t", a, mynewargv[a]);
	printf("\r\n");
	if(bk)
		printf("Block Keyword record '%s' is\n"
			"	keyword:	%s\n"
			"	schedule:	%s\n",
			bk->name,
			bk->keyword,
			bk->schedule);
}

int config_webfeatures(struct uci_context *ctx, struct iptc_handle *handle)
{
	struct uci_element *e = NULL;
	struct uci_ptr ptr;
	char *tmpptr=strdup("firewall.webfeatures");
	int ret;
	int java=0,cookies=0,activex=0,httpproxy=0;
	char * tablename="filter";

	if (uci_lookup_ptr(ctx, &ptr, tmpptr, true) != UCI_OK) {
		printf("SKC: Error in getting info about:%s\r\n",tmpptr);
		return 1;
	}

	uci_foreach_element(&(ptr.s)->options, e) {
		struct uci_option *o = uci_to_option(e);
		if(strcmp(o->e.name,"java")==0)
		{	
			java=atoi(o->v.string);
			if(java)
			{
				mynewargc = 0;
				add_argv("newTestApp");
				add_argv("-t");
				add_argv("filter");
				add_argv("-A");
				add_argv("content_filter");

				add_argv("-m");
				add_argv("webstr");

				add_argv("--content");
				add_argv("1");
				add_argv("--jump");
				add_argv("DROP");
				ret = do_command(mynewargc, mynewargv, &tablename, &handle);
				if (!ret) {
					printf("do_command failed.string:%s\n",iptc_strerror(errno));
				}
				free_argv();
			}
		}
		else if(strcmp(o->e.name,"cookies")==0)
		{
			cookies=atoi(o->v.string);
			if(cookies)
			{
				mynewargc = 0;
				add_argv("newTestApp");
				add_argv("-t");
				add_argv("filter");
				add_argv("-A");
				add_argv("content_filter");
	
				add_argv("-m");
				add_argv("webstr");
	
				add_argv("--content");
				add_argv("4");
				add_argv("--jump");
				add_argv("ACCEPT");
				ret = do_command(mynewargc, mynewargv, &tablename, &handle);
				if (!ret) {
					printf("do_command failed.string:%s\n",iptc_strerror(errno));
				}
				free_argv();
			}
		}
		else if(strcmp(o->e.name,"activex")==0)
		{
			activex=atoi(o->v.string);
			if(activex)
			{
				mynewargc = 0;
				add_argv("newTestApp");
				add_argv("-t");
				add_argv("filter");
				add_argv("-A");
				add_argv("content_filter");
	
				add_argv("-m");
				add_argv("webstr");
	
				add_argv("--content");
				add_argv("2");
				add_argv("--jump");
				add_argv("DROP");
				ret = do_command(mynewargc, mynewargv, &tablename, &handle);
				if (!ret) {
					printf("do_command failed.string:%s\n",iptc_strerror(errno));
				}
				free_argv();
	
			}
		}
		else if(strcmp(o->e.name,"httpproxy")==0)
		{
			httpproxy=atoi(o->v.string);
			if(httpproxy)
			{
				mynewargc = 0;
				add_argv("newTestApp");
				add_argv("-t");
				add_argv("filter");
				add_argv("-A");
				add_argv("content_filter");
	
				add_argv("-m");
				add_argv("webstr");
	
				add_argv("--content");
				add_argv("8");
				add_argv("--jump");
				add_argv("REJECT");
				ret = do_command(mynewargc, mynewargv, &tablename, &handle);
				if (!ret) {
					printf("do_command failed.string:%s\n",iptc_strerror(errno));
				}
				free_argv();
			}
		}
	}

	if(java || cookies || activex || httpproxy)
	{
		system("cmm -c set port_dpi enable");
		system("cmm -c set port_dpi delete 80");
		system("cmm -c set port_dpi add 80");
	}

	free(tmpptr);
	return 0;
}

int config_trusted(struct uci_context *ctx, struct iptc_handle *handle)
{
	struct uci_element *e = NULL;
	struct uci_ptr ptr;
	struct uci_ptr ptr1;
	char *tmpptr=strdup("firewall.@trustedDomain[0].domain_name");
	char *tmpptr1=strdup("firewall.webfeatures.exception");
	int exception=0;
	int ret;
	char * tablename="filter";


	if (uci_lookup_ptr(ctx, &ptr1, tmpptr1, true) != UCI_OK) {
		printf("Error in getting info about:%s\r\n",tmpptr);
		return 1;
	}
	exception=atoi(ptr1.o->v.string);

	if (uci_lookup_ptr(ctx, &ptr, tmpptr, true) != UCI_OK) {
		printf("Error in getting info about:%s\r\n",tmpptr);
		return 1;
	}

	if(!(ptr.flags & UCI_LOOKUP_COMPLETE))
  		return 1;
	
	struct uci_option *o = ptr.o;
	uci_foreach_element(&o->v.list, e) {
		mynewargc=0; 
		if(exception == 0)
		{
			add_argv("iptables-uci");
			add_argv("-t");
			add_argv("filter");
			add_argv("-A");
			add_argv("content_filter");
			add_argv("-m");
			add_argv("webstr");
			add_argv("--url");
			add_argv(e->name);
			add_argv("--jump");
			add_argv("ACCEPT");
		}
		else
		{
			add_argv("iptables-uci");
			add_argv("-t");
			add_argv("filter");
			add_argv("-I");
			add_argv("content_filter");
			add_argv("--match");
			add_argv("string");
			add_argv("--algo");
			add_argv("bm");
			add_argv("--string");
			add_argv(e->name);
			add_argv("--jump");
			add_argv("ACCEPT");
		}
		ret = do_command(mynewargc, mynewargv, &tablename, &handle);
		if (!ret) {
			printf("do_command failed.string:%s\n",iptc_strerror(errno));
		}

		free_argv();
	}

	free(tmpptr);
	free(tmpptr1); 
	return 0;
}

static int insert_blockurlrule(struct uci_blockurlRule *url, struct uci_context *ctx, struct iptc_handle *handle)
{
	int ret;
	struct uci_element *e = NULL;
	struct uci_ptr ptr;
	char cmdbuf[256]={'\0'};
	char * tablename="filter";
	mynewargc = 0;

	add_argv("iptables-uci");
	add_argv("-t");
	add_argv("filter");
	add_argv("-A");
	add_argv("content_filter");

	add_argv("-m");
	add_argv("webstr");
	add_argv("--jump");
	add_argv("drop_log");

	add_argv("--url");
	add_argv(url->domain_name);

	//sprintf(cmdbuf,"echo %s > /proc/sys/net/ipv4/blockurl_last",url->domain_name);
	sprintf(cmdbuf,"echo %s > /proc/sys/kernel/blockurl_last",url->domain_name);
	system(cmdbuf);

//For Schedule
	if(strlen(url->schedule) && strcmp(url->schedule,"Always")!=0)
	{
		char tuple[32]="";
		bool dayConfig=0;
		char dayString[32]="";

		sprintf(tuple,"schedule.%s",url->schedule);

		if (uci_lookup_ptr(ctx, &ptr, tuple, true) != UCI_OK) {
			printf("Error in getting info about schedule with string:%s\r\n",tuple);
			//cli_perror();
			return 1;
		}

		add_argv("--match");
		add_argv("time");
        add_argv("--kerneltz");

		//e = ptr.last;
		uci_foreach_element(&(ptr.s)->options, e) {
			struct uci_option *o = uci_to_option(e);

			if(strcmp(o->e.name,"start_time")==0)
			{
				add_argv("--timestart");
				add_argv(o->v.string);
			}
			else if (strcmp(o->e.name,"end_time")==0)
			{
				add_argv("--timestop");
				add_argv(o->v.string);
			}
			else if (strcmp(o->e.name,"sun")==0)
			{//During parsing Sun will come first.
				if(strcmp(o->v.string,"1")==0)
				{
					if(dayConfig)
						strcat(dayString,",Su");
					else
					{
						sprintf(dayString,"Su");
						dayConfig=1;
					}
				}
			}
			else if (strcmp(o->e.name,"mon")==0)
			{
				if(strcmp(o->v.string,"1")==0)
				{
					if(dayConfig)
						strcat(dayString,",Mo");
					else
					{
						sprintf(dayString,"Mo");
						dayConfig=1;
					}
				}
			}
			else if (strcmp(o->e.name,"tue")==0)
			{
				if(strcmp(o->v.string,"1")==0)
				{
					if(dayConfig)
						strcat(dayString,",Tu");
					else
					{
						sprintf(dayString,"Tu");
						dayConfig=1;
					}
				}
			}
			else if (strcmp(o->e.name,"wed")==0)
			{
				if(strcmp(o->v.string,"1")==0)
				{
					if(dayConfig)
						strcat(dayString,",We");
					else
					{
						sprintf(dayString,"We");
						dayConfig=1;
					}
				}
			}
			else if (strcmp(o->e.name,"thu")==0)
			{
				if(strcmp(o->v.string,"1")==0)
				{
					if(dayConfig)
						strcat(dayString,",Th");
					else
					{
						sprintf(dayString,"Th");
						dayConfig=1;
					}
				}
			}
			else if (strcmp(o->e.name,"fri")==0)
			{
				if(strcmp(o->v.string,"1")==0)
				{
					if(dayConfig)
						strcat(dayString,",Fr");
					else
					{
						sprintf(dayString,"Fr");
						dayConfig=1;
					}
				}
			}
			else if (strcmp(o->e.name,"sat")==0)
			{
				if(strcmp(o->v.string,"1")==0)
				{
					if(dayConfig)
						strcat(dayString,",Sa");
					else
					{
						sprintf(dayString,"Sa");
						dayConfig=1;
					}
				}
			}
		}

		if(dayConfig)
		{
			add_argv("--weekdays");
			add_argv(dayString);
		}
	
		if (ptr.p)
			uci_unload(ctx, ptr.p);
	}
//END:For Schedule

	if (enable_debug)
		print_blockURLRule(url);

	ret = do_command(mynewargc, mynewargv, &tablename, &handle);
	if (!ret) {
		printf("do_command failed.string:%s\n",iptc_strerror(errno));
	}

	free_argv();
	return 0;
}

static int insert_blockkeywordrule(struct uci_blockkeywordRule *key, struct uci_context *ctx, struct iptc_handle *handle)
{
	int ret;
	struct uci_element *e = NULL;
	struct uci_ptr ptr;
	char cmdbuf[256]={'\0'};
	char * tablename="filter";

	mynewargc = 0;

	add_argv("iptables-uci");
	add_argv("-t");
	add_argv("filter");
	add_argv("-A");
	add_argv("content_filter");

	add_argv("-m");
	add_argv("webstr");
	add_argv("--jump");
	add_argv("drop_log");

	add_argv("--url");
	add_argv(key->keyword);

	//sprintf(cmdbuf,"echo %s > /proc/sys/net/ipv4/blockurl_last",key->keyword);
	sprintf(cmdbuf,"echo %s > /proc/sys/kernel/blockurl_last",key->keyword);
	system(cmdbuf);

//For Schedule
	if(strlen(key->schedule) && strcmp(key->schedule,"Always")!=0)
	{
		char tuple[32]="";
		bool dayConfig=0;
		char dayString[32]="";

		sprintf(tuple,"schedule.%s",key->schedule);

		if (uci_lookup_ptr(ctx, &ptr, tuple, true) != UCI_OK) {
			printf("Error in getting info about schedule with string:%s\r\n",tuple);
			//cli_perror();
			return 1;
		}

		add_argv("--match");
		add_argv("time");
        add_argv("--kerneltz");

		//e = ptr.last;
		uci_foreach_element(&(ptr.s)->options, e) {
			struct uci_option *o = uci_to_option(e);

			if(strcmp(o->e.name,"start_time")==0)
			{
				add_argv("--timestart");
				add_argv(o->v.string);
			}
			else if (strcmp(o->e.name,"end_time")==0)
			{
				add_argv("--timestop");
				add_argv(o->v.string);
			}
			else if (strcmp(o->e.name,"sun")==0)
			{//During parsing Sun will come first.
				if(strcmp(o->v.string,"1")==0)
				{
					if(dayConfig)
						strcat(dayString,",Su");
					else
					{
						sprintf(dayString,"Su");
						dayConfig=1;
					}
				}
			}
			else if (strcmp(o->e.name,"mon")==0)
			{
				if(strcmp(o->v.string,"1")==0)
				{
					if(dayConfig)
						strcat(dayString,",Mo");
					else
					{
						sprintf(dayString,"Mo");
						dayConfig=1;
					}
				}
			}
			else if (strcmp(o->e.name,"tue")==0)
			{
				if(strcmp(o->v.string,"1")==0)
				{
					if(dayConfig)
						strcat(dayString,",Tu");
					else
					{
						sprintf(dayString,"Tu");
						dayConfig=1;
					}
				}
			}
			else if (strcmp(o->e.name,"wed")==0)
			{
				if(strcmp(o->v.string,"1")==0)
				{
					if(dayConfig)
						strcat(dayString,",We");
					else
					{
						sprintf(dayString,"We");
						dayConfig=1;
					}
				}
			}
			else if (strcmp(o->e.name,"thu")==0)
			{
				if(strcmp(o->v.string,"1")==0)
				{
					if(dayConfig)
						strcat(dayString,",Th");
					else
					{
						sprintf(dayString,"Th");
						dayConfig=1;
					}
				}
			}
			else if (strcmp(o->e.name,"fri")==0)
			{
				if(strcmp(o->v.string,"1")==0)
				{
					if(dayConfig)
						strcat(dayString,",Fr");
					else
					{
						sprintf(dayString,"Fr");
						dayConfig=1;
					}
				}
			}
			else if (strcmp(o->e.name,"sat")==0)
			{
				if(strcmp(o->v.string,"1")==0)
				{
					if(dayConfig)
						strcat(dayString,",Sa");
					else
					{
						sprintf(dayString,"Sa");
						dayConfig=1;
					}
				}
			}
		}

		if(dayConfig)
		{
			add_argv("--weekdays");
			add_argv(dayString);
		}
	
		if (ptr.p)
			uci_unload(ctx, ptr.p);
	}
//END:For Schedule

	if (enable_debug)
		print_blockKeywordRule(key);

	ret = do_command(mynewargc, mynewargv, &tablename, &handle);
	if (!ret) {
		printf("do_command failed.string:%s\n",iptc_strerror(errno));
	}

	free_argv();
	return 0;
}

int insert_allowurlstringrule(struct uci_allowurlRule *url, struct uci_context *ctx, struct iptc_handle *handle)
{
    int ret;
    struct uci_element *e = NULL;
    struct uci_ptr ptr;
    char * tablename="filter";
    mynewargc = 0;

    add_argv("newTestApp");
    add_argv("-t");
    add_argv("filter");
    add_argv("-A");
    add_argv("content_filter");

    add_argv("--match");
    add_argv("string");
    add_argv("--algo");
    add_argv("bm");
    add_argv("--jump");
    add_argv("accept_log");

    add_argv("--string");
    add_argv(url->domain_name);

    //For Schedule
    if(strlen(url->schedule) && strcmp(url->schedule,"Always")!=0)
    {
        char tuple[32]="";
        bool dayConfig=0;
        char dayString[32]="";

        sprintf(tuple,"schedule.%s",url->schedule);

        if (uci_lookup_ptr(ctx, &ptr, tuple, true) != UCI_OK) {
            printf("Error in getting info about schedule with string:%s\r\n",tuple);
            //cli_perror();
            return 1;
        }

        add_argv("--match");
        add_argv("time");
        add_argv("--kerneltz");

        uci_foreach_element(&(ptr.s)->options, e) {
            struct uci_option *o = uci_to_option(e);

            if(strcmp(o->e.name,"start_time")==0)
            {
                add_argv("--timestart");
                add_argv(o->v.string);
            }
            else if (strcmp(o->e.name,"end_time")==0)
            {
                add_argv("--timestop");
                add_argv(o->v.string);
            }
            else if (strcmp(o->e.name,"sun")==0)
            {//During parsing Sun will come first.
                if(strcmp(o->v.string,"1")==0)
                {
                    if(dayConfig)
                        strcat(dayString,",Su");
                    else
                    {
                        sprintf(dayString,"Su");
                        dayConfig=1;
                    }
                }
            }
            else if (strcmp(o->e.name,"mon")==0)
            {
                if(strcmp(o->v.string,"1")==0)
                {
                    if(dayConfig)
                        strcat(dayString,",Mo");
                    else
                    {
                        sprintf(dayString,"Mo");
                        dayConfig=1;
                    }
                }
            }
            else if (strcmp(o->e.name,"tue")==0)
            {
                if(strcmp(o->v.string,"1")==0)
                {
                    if(dayConfig)
                        strcat(dayString,",Tu");
                    else
                    {
                        sprintf(dayString,"Tu");
                        dayConfig=1;
                    }
                }
            }
            else if (strcmp(o->e.name,"wed")==0)
            {
                if(strcmp(o->v.string,"1")==0)
                {
                    if(dayConfig)
                        strcat(dayString,",We");
                    else
                    {
                        sprintf(dayString,"We");
                        dayConfig=1;
                    }
                }
            }
            else if (strcmp(o->e.name,"thu")==0)
            {
                if(strcmp(o->v.string,"1")==0)
                {
                    if(dayConfig)
                        strcat(dayString,",Th");
                    else
                    {
                        sprintf(dayString,"Th");
                        dayConfig=1;
                    }
                }
            }
            else if (strcmp(o->e.name,"fri")==0)
            {
                if(strcmp(o->v.string,"1")==0)
                {
                    if(dayConfig)
                        strcat(dayString,",Fr");
                    else
                    {
                        sprintf(dayString,"Fr");
                        dayConfig=1;
                    }
                }
            }
            else if (strcmp(o->e.name,"sat")==0)
            {
                if(strcmp(o->v.string,"1")==0)
                {
                    if(dayConfig)
                        strcat(dayString,",Sa");
                    else
                    {
                        sprintf(dayString,"Sa");
                        dayConfig=1;
                    }
                }
            }
        }

        if(dayConfig)
        {
            add_argv("--weekdays");
            add_argv(dayString);
        }

        if (ptr.p)
            uci_unload(ctx, ptr.p);
    }
    //END:For Schedule
    //
    if (enable_debug)
        print_allowURLRule(url);

    ret = do_command(mynewargc, mynewargv, &tablename, &handle);
    if (!ret) {
        printf("do_command failed.string:%s\n",iptc_strerror(errno));
    }

    free_argv();
    return 0;


}

int insert_allowurlrule(struct uci_allowurlRule *url, struct uci_context *ctx, struct iptc_handle *handle)
{
	int ret;
	struct uci_element *e = NULL;
	struct uci_ptr ptr;
	char * tablename="filter";
	mynewargc = 0;

	add_argv("newTestApp");
	add_argv("-t");
	add_argv("filter");
	add_argv("-A");
	add_argv("content_filter");

	add_argv("-m");
	add_argv("webstr");
	add_argv("--jump");
	add_argv("accept_log");

	add_argv("--url");
	add_argv(url->domain_name);

//For Schedule
	if(strlen(url->schedule) && strcmp(url->schedule,"Always")!=0)
	{
		char tuple[32]="";
		bool dayConfig=0;
		char dayString[32]="";

		sprintf(tuple,"schedule.%s",url->schedule);

		if (uci_lookup_ptr(ctx, &ptr, tuple, true) != UCI_OK) {
			printf("Error in getting info about schedule with string:%s\r\n",tuple);
			//cli_perror();
			return 1;
		}

		add_argv("--match");
		add_argv("time");
        add_argv("--kerneltz");

		//e = ptr.last;
		uci_foreach_element(&(ptr.s)->options, e) {
			struct uci_option *o = uci_to_option(e);

			if(strcmp(o->e.name,"start_time")==0)
			{
				add_argv("--timestart");
				add_argv(o->v.string);
			}
			else if (strcmp(o->e.name,"end_time")==0)
			{
				add_argv("--timestop");
				add_argv(o->v.string);
			}
			else if (strcmp(o->e.name,"sun")==0)
			{//During parsing Sun will come first.
				if(strcmp(o->v.string,"1")==0)
				{
					if(dayConfig)
						strcat(dayString,",Su");
					else
					{
						sprintf(dayString,"Su");
						dayConfig=1;
					}
				}
			}
			else if (strcmp(o->e.name,"mon")==0)
			{
				if(strcmp(o->v.string,"1")==0)
				{
					if(dayConfig)
						strcat(dayString,",Mo");
					else
					{
						sprintf(dayString,"Mo");
						dayConfig=1;
					}
				}
			}
			else if (strcmp(o->e.name,"tue")==0)
			{
				if(strcmp(o->v.string,"1")==0)
				{
					if(dayConfig)
						strcat(dayString,",Tu");
					else
					{
						sprintf(dayString,"Tu");
						dayConfig=1;
					}
				}
			}
			else if (strcmp(o->e.name,"wed")==0)
			{
				if(strcmp(o->v.string,"1")==0)
				{
					if(dayConfig)
						strcat(dayString,",We");
					else
					{
						sprintf(dayString,"We");
						dayConfig=1;
					}
				}
			}
			else if (strcmp(o->e.name,"thu")==0)
			{
				if(strcmp(o->v.string,"1")==0)
				{
					if(dayConfig)
						strcat(dayString,",Th");
					else
					{
						sprintf(dayString,"Th");
						dayConfig=1;
					}
				}
			}
			else if (strcmp(o->e.name,"fri")==0)
			{
				if(strcmp(o->v.string,"1")==0)
				{
					if(dayConfig)
						strcat(dayString,",Fr");
					else
					{
						sprintf(dayString,"Fr");
						dayConfig=1;
					}
				}
			}
			else if (strcmp(o->e.name,"sat")==0)
			{
				if(strcmp(o->v.string,"1")==0)
				{
					if(dayConfig)
						strcat(dayString,",Sa");
					else
					{
						sprintf(dayString,"Sa");
						dayConfig=1;
					}
				}
			}
		}

		if(dayConfig)
		{
			add_argv("--weekdays");
			add_argv(dayString);
		}
	
		if (ptr.p)
			uci_unload(ctx, ptr.p);
	}
//END:For Schedule

	if (enable_debug)
		print_allowURLRule(url);

	ret = do_command(mynewargc, mynewargv, &tablename, &handle);
	if (!ret) {
		printf("do_command failed.string:%s\n",iptc_strerror(errno));
	}

	free_argv();
	return 0;
}

int insert_allowkeywordstringrule(struct uci_allowkeywordRule *key, struct uci_context *ctx, struct iptc_handle *handle)
{
    int ret;
    struct uci_element *e = NULL;
    struct uci_ptr ptr;
    char * tablename="filter";
    mynewargc = 0;

    add_argv("newTestApp");
    add_argv("-t");
    add_argv("filter");
    add_argv("-A");
    add_argv("content_filter");

    add_argv("--match");
    add_argv("string");
    add_argv("--algo");
    add_argv("bm");
    add_argv("--jump");
    add_argv("accept_log");

    add_argv("--string");
    add_argv(key->keyword);

    if(strlen(key->schedule) && strcmp(key->schedule,"Always")!=0)
    {
        char tuple[32]="";
        bool dayConfig=0;
        char dayString[16]="";

        sprintf(tuple,"schedule.%s",key->schedule);

        if (uci_lookup_ptr(ctx, &ptr, tuple, true) != UCI_OK) {
            printf("Error in getting info about schedule with string:%s\r\n",tuple);
            //cli_perror();
            return 1;
        }

        add_argv("--match");
        add_argv("time");
        add_argv("--kerneltz");

        //e = ptr.last;
        uci_foreach_element(&(ptr.s)->options, e) {
            struct uci_option *o = uci_to_option(e);

            if(strcmp(o->e.name,"start_time")==0)
            {
                add_argv("--timestart");
                add_argv(o->v.string);
            }
            else if (strcmp(o->e.name,"end_time")==0)
            {
                add_argv("--timestop");
                add_argv(o->v.string);
            }
            else if (strcmp(o->e.name,"sun")==0)
            {//During parsing Sun will come first.

                if(strcmp(o->v.string,"1")==0)
                {
                    if(dayConfig)
                        strcat(dayString,",Su");
                    else
                    {
                        sprintf(dayString,"Su");
                        dayConfig=1;
                    }
                }
            }
            else if (strcmp(o->e.name,"mon")==0)
            {
                if(strcmp(o->v.string,"1")==0)
                {
                    if(dayConfig)
                        strcat(dayString,",Mo");
                    else
                    {
                        sprintf(dayString,"Mo");
                        dayConfig=1;
                    }
                }
            }
            else if (strcmp(o->e.name,"tue")==0)
            {
                if(strcmp(o->v.string,"1")==0)
                {
                    if(dayConfig)
                        strcat(dayString,",Tu");
                    else
                    {
                        sprintf(dayString,"Tu");
                        dayConfig=1;
                    }
                }
            }
            else if (strcmp(o->e.name,"wed")==0)
            {
                if(strcmp(o->v.string,"1")==0)
                {
                    if(dayConfig)
                        strcat(dayString,",We");
                    else
                    {
                        sprintf(dayString,"We");
                        dayConfig=1;
                    }
                }
            }
            else if (strcmp(o->e.name,"thu")==0)
            {
                if(strcmp(o->v.string,"1")==0)
                {
                    if(dayConfig)
                        strcat(dayString,",Th");
                    else
                    {
                        sprintf(dayString,"Th");
                        dayConfig=1;
                    }
                }
            }
            else if (strcmp(o->e.name,"fri")==0)
            {
                if(strcmp(o->v.string,"1")==0)
                {
                    if(dayConfig)
                        strcat(dayString,",Fr");
                    else
                    {
                        sprintf(dayString,"Fr");
                        dayConfig=1;
                    }
                }
            }
            else if (strcmp(o->e.name,"sat")==0)
            {
                if(strcmp(o->v.string,"1")==0)
                {
                    if(dayConfig)
                        strcat(dayString,",Sa");
                    else
                    {
                        sprintf(dayString,"Sa");
                        dayConfig=1;
                    }
                }
            }
        }

        if(dayConfig)
        {
            add_argv("--weekdays");
            add_argv(dayString);
        }

        if (ptr.p)
            uci_unload(ctx, ptr.p);
    }
    //END:For Schedule
    if (enable_debug)
        print_allowKeywordRule(key);

    ret = do_command(mynewargc, mynewargv, &tablename, &handle);
    if (!ret) {
        printf("do_command failed.string:%s\n",iptc_strerror(errno));
    }

    free_argv();
    return 0;


}

int insert_allowkeywordrule(struct uci_allowkeywordRule *key, struct uci_context *ctx, struct iptc_handle *handle)
{
	int ret;
	struct uci_element *e = NULL;
	struct uci_ptr ptr;
	char * tablename="filter";
	mynewargc = 0;

	add_argv("newTestApp");
	add_argv("-t");
	add_argv("filter");
	add_argv("-A");
	add_argv("content_filter");

	add_argv("-m");
	add_argv("webstr");
	add_argv("--jump");
	add_argv("accept_log");

	add_argv("--host");
	add_argv(key->keyword);

//For Schedule
	if(strlen(key->schedule) && strcmp(key->schedule,"Always")!=0)
	{
		char tuple[32]="";
		bool dayConfig=0;
		char dayString[16]="";

		sprintf(tuple,"schedule.%s",key->schedule);

		if (uci_lookup_ptr(ctx, &ptr, tuple, true) != UCI_OK) {
			printf("Error in getting info about schedule with string:%s\r\n",tuple);
			//cli_perror();
			return 1;
		}

		add_argv("--match");
		add_argv("time");
        add_argv("--kerneltz");

		//e = ptr.last;
		uci_foreach_element(&(ptr.s)->options, e) {
			struct uci_option *o = uci_to_option(e);

			if(strcmp(o->e.name,"start_time")==0)
			{
				add_argv("--timestart");
				add_argv(o->v.string);
			}
			else if (strcmp(o->e.name,"end_time")==0)
			{
				add_argv("--timestop");
				add_argv(o->v.string);
			}
			else if (strcmp(o->e.name,"sun")==0)
			{//During parsing Sun will come first.
				if(strcmp(o->v.string,"1")==0)
				{
					if(dayConfig)
						strcat(dayString,",Su");
					else
					{
						sprintf(dayString,"Su");
						dayConfig=1;
					}
				}
			}
			else if (strcmp(o->e.name,"mon")==0)
			{
				if(strcmp(o->v.string,"1")==0)
				{
					if(dayConfig)
						strcat(dayString,",Mo");
					else
					{
						sprintf(dayString,"Mo");
						dayConfig=1;
					}
				}
			}
			else if (strcmp(o->e.name,"tue")==0)
			{
				if(strcmp(o->v.string,"1")==0)
				{
					if(dayConfig)
						strcat(dayString,",Tu");
					else
					{
						sprintf(dayString,"Tu");
						dayConfig=1;
					}
				}
			}
			else if (strcmp(o->e.name,"wed")==0)
			{
				if(strcmp(o->v.string,"1")==0)
				{
					if(dayConfig)
						strcat(dayString,",We");
					else
					{
						sprintf(dayString,"We");
						dayConfig=1;
					}
				}
			}
			else if (strcmp(o->e.name,"thu")==0)
			{
				if(strcmp(o->v.string,"1")==0)
				{
					if(dayConfig)
						strcat(dayString,",Th");
					else
					{
						sprintf(dayString,"Th");
						dayConfig=1;
					}
				}
			}
			else if (strcmp(o->e.name,"fri")==0)
			{
				if(strcmp(o->v.string,"1")==0)
				{
					if(dayConfig)
						strcat(dayString,",Fr");
					else
					{
						sprintf(dayString,"Fr");
						dayConfig=1;
					}
				}
			}
			else if (strcmp(o->e.name,"sat")==0)
			{
				if(strcmp(o->v.string,"1")==0)
				{
					if(dayConfig)
						strcat(dayString,",Sa");
					else
					{
						sprintf(dayString,"Sa");
						dayConfig=1;
					}
				}
			}
		}

		if(dayConfig)
		{
			add_argv("--weekdays");
			add_argv(dayString);
		}
	
		if (ptr.p)
			uci_unload(ctx, ptr.p);
	}
//END:For Schedule

	if (enable_debug)
		print_allowKeywordRule(key);

	ret = do_command(mynewargc, mynewargv, &tablename, &handle);
	if (!ret) {
		printf("do_command failed.string:%s\n",iptc_strerror(errno));
	}

	free_argv();
	return 0;
}

int insert_blockrule(struct iptc_handle *handle)
{

    int ret;
    char * tablename="filter";

    mynewargc=0;

    add_argv("newTestApp");
    add_argv("-t");
    add_argv("filter");
    add_argv("-A");
    add_argv("content_filter");

    add_argv("--match");
    add_argv("string");
    add_argv("--algo");
    add_argv("bm");
    add_argv("--string");
    add_argv("*");
    add_argv("--jump");
    add_argv("DROP");


    ret = do_command(mynewargc, mynewargv, &tablename, &handle);
    if (!ret) {
        printf("do_command failed.string:%s\n",iptc_strerror(errno));
    }

    free_argv();
    return 0;


}

int config_contentfilter(struct uci_context *ctx, struct iptc_handle *handle)
{
	struct  list_head *p;
	struct uci_blockurlRule *blockurl;
	struct uci_blockkeywordRule *blockkeyword;
	struct uci_allowurlRule *allowurl;
	struct uci_allowkeywordRule *allowkeyword;
	int contentfilter=0,allowurl_status=0,blockurl_status=0;
	struct uci_ptr ptr;
	struct uci_element *e = NULL;
	char *tmpptr=strdup("firewall.content_filter");

	if (uci_lookup_ptr(ctx, &ptr, tmpptr, true) != UCI_OK) {
		printf("Error in getting info about:%s\r\n",tmpptr);
		return 1;
	}

	uci_foreach_element(&(ptr.s)->options, e) {
		struct uci_option *o = uci_to_option(e);

		if(strcmp(o->e.name,"status")==0)
			contentfilter=atoi(o->v.string);
		else if(strcmp(o->e.name,"allowurl_status")==0)
			allowurl_status=atoi(o->v.string);
		else if(strcmp(o->e.name,"blockurl_status")==0)
			blockurl_status=atoi(o->v.string);
	}

	if((contentfilter == 1) && (blockurl_status == 1))
	{
		system("cmm -c set port_dpi enable");
		system("cmm -c set port_dpi delete 80");
		system("cmm -c set port_dpi add 80");

		system("echo 10 > /proc/sys/kernel/contentfilter_type");
		//system("echo 10 > /proc/sys/net/ipv4/contentfilter_type");

		list_for_each(p, &blockurlRules) {
			blockurl = list_entry(p, struct uci_blockurlRule, list);

			insert_blockurlrule(blockurl, ctx, handle);
		}

		list_for_each(p, &blockkeywordRules) {
			blockkeyword = list_entry(p, struct uci_blockkeywordRule, list);

			insert_blockkeywordrule(blockkeyword, ctx, handle);
		}
	}
	else if((contentfilter==1) && (allowurl_status == 1))
	{
		system("cmm -c set port_dpi enable");
		system("cmm -c set port_dpi delete 80");
		system("cmm -c set port_dpi add 80");

		//system("echo 20 > /proc/sys/net/ipv4/contentfilter_type");
		system("echo 20 > /proc/sys/kernel/contentfilter_type");

		list_for_each(p, &allowurlRules) {
			allowurl = list_entry(p, struct uci_allowurlRule, list);

			insert_allowurlrule(allowurl, ctx, handle);
			insert_allowurlstringrule(allowurl, ctx, handle);
		}

		list_for_each(p, &allowkeywordRules) {
			allowkeyword = list_entry(p, struct uci_allowkeywordRule, list);

			insert_allowkeywordrule(allowkeyword, ctx, handle);
			insert_allowkeywordstringrule(allowkeyword, ctx, handle);
		}
		insert_blockrule(handle);
	}
	return 0;
}

void config_allWebContentFiltering(struct uci_context *ctx)
{
	struct iptc_handle *handle = NULL;
	int y=0;

	handle = create_handle("filter");

	if(iptc_is_chain(strdup("content_filter"), handle))
	{
		if(!iptc_flush_entries(strdup("content_filter"),handle))
			printf("Failed in flushing rules of content_filter in iptables\r\n");
		system("cmm -c set port_dpi delete 80");
	}
	else
		printf("No chain detected with that name for IPv4\r\n");

	config_webfeatures(ctx, handle);
	config_trusted(ctx, handle);
	config_contentfilter(ctx, handle);

	y = iptc_commit(handle);
	if (!y)
	{
		//printf("Error commiting data for IPv4: %s\n", iptc_strerror(errno));
		if(errno == 11) //Identifyed that resource unavailable error's errno is 11. So we retry again after a sec and then giveup.
		{
			sleep(1);
			y = iptc_commit(handle);
			if (!y)
				printf("Error commiting data: %s errno:%d in the context of allWebCntntFltr even after retry.\n",
					iptc_strerror(errno),errno);
		}
		else
			printf("Error commiting data: %s errno:%d in the context of allWebCntntFltr.\n",
				iptc_strerror(errno),errno);
		//return -1;
	}
	iptc_free(handle);
}
