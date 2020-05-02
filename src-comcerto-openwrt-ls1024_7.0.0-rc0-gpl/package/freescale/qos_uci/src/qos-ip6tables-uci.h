#include "qos-iptables-uci_common.h"
#include "ip6tables.h"

struct qos_flow {

	char proto[16];
	char icmptype[16];
	char sport[16];
	char dport[16];
	char sip[64];
	char dip[64];
	char dscp[16];
	char rcvinterface[32];
	int mark_dscp;
	int ip_version;
	char queue[8];
};

struct queue_config {
	unsigned int queue1 : 1;
	unsigned int queue2 : 1;
	unsigned int queue3 : 1;
	unsigned int queue4 : 1;
	unsigned int queue5 : 1;
	unsigned int queue6 : 1;
	unsigned int queue7 : 1;
};

struct ip6tc_handle *create6_handle(const char *tablename);
void tokenize_and_docommand(char *cmdBuff,char *table, struct ip6tc_handle *handle);
int strreplace(char *outputbuff,char * original_buff,char *search_string,char * replace_with);
int get_queues_configured(struct queue_config *queueconf, char *policy, char *class_name, struct uci_context *ctx);
int add_rule_to_chain(char *class_name, char *flow_name, struct uci_context *ctx, struct ip6tc_handle *handle);

void print_usage(void);
void print_flowEntry(struct qos_flow *flow);
