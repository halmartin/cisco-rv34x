#include "iptables-uci_common.h"
#include "ip6tables.h"

struct uci_aclRule {
	struct ucimap_section_data map;
	struct list_head list;

	char *name;
	int test;
	bool enable;
	char *target;
	int family;
	char *protocol;
	int protocol_number;
	int port_start;
	int port_end;
	int icmp_type;
	bool log;
	char *src;
	char *src_ip;
	int src_prefix;
	char *src_ip_end;
	char *dest;
	char *dest_ip;
	int dest_prefix;
	char *dest_ip_end;
	char *schedule;
};

struct modules {
	unsigned int basic_settings : 1;
	unsigned int access_rules : 1;
	unsigned int dmz : 1;
	unsigned int spoof_rules: 1;
};

struct ip6tc_handle *create6_handle(const char *tablename);

void insert_udp_flood6(char *interface,struct ip6tc_handle * handle);
void insert_syn_flood6(char *interface,struct ip6tc_handle * handle);
void insert_ping_of_death6(char *interface,struct ip6tc_handle * handle);
void insert_icmp_flood6(char *interface, struct ip6tc_handle * handle);
int insert_remote_management6(struct uci_context *ctx,struct ip6tc_handle *filter_handle);
void insert_lanvpn_http_rules6(int port,struct ip6tc_handle *handle);
void insert_lanvpn_https_rules6(int port,struct ip6tc_handle *handle);
int insert_lanvpn_management6(struct uci_context *ctx,struct ip6tc_handle *filter_handle);
void insert_block_wan_rules6(struct iface_data *iface,struct ip6tc_handle *handle);
int insert_block_wan6(struct uci_context *ctx,struct ip6tc_handle *filter_handle);
void insert_remote_management_forward_rules6(struct ip6tc_handle *filter_handle,
                                                   int remote_http,int remote_https,
						int port,char *remote_ipaddress,char *remote_end,
						struct iface_data *findIface);
void flush_basicsettings6(struct ip6tc_handle *filter_handle);

void print_acl(struct uci_aclRule *acl);
void print_usage(void);

void config_aclrule6(struct uci_context *ctx);
int config_basicsettings6(struct uci_context *ctx);


