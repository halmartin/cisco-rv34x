#include "iptables-uci_common.h"
#include "iptables.h"

struct uci_dmzhost {
	struct ucimap_section_data map;
	struct list_head list;

	char *name;
	int test;

	bool status;
	char *ipaddr;
};

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
	int src_subnet;
	char *src_ip_end;
	char *dest;
	char *dest_ip;
	int dest_subnet;
	char *dest_ip_end;
	char *schedule;
};

struct uci_ifaceNAT {
	struct ucimap_section_data map;
	struct list_head list;

	char *name;
	int test; //This is just for us. No use with this variable. We can ignore later on.
	bool enable;
	char *interfacename;
};

struct uci_staticNAT {
	struct ucimap_section_data map;
 	struct list_head list;

	char *name;
	int test; //This is just for us. No use with this variable. We can ignore later on.
	bool status;
	char *privateip;
	char *publicip;
	int range_length;
	char *protocol;
	int port_start;
	int port_end;
	char *interface;
    int icmp_type;
    int protocol_number;
};

struct uci_portForward {
	struct ucimap_section_data map;
 	struct list_head list;

	char *name;
	int test; //This is just for us. No use with this variable. We can ignore later on.
	bool status;
	char *protocol;
	int ext_port_start;
	int ext_port_end;
	int int_port_start;
	int int_port_end;
	char *interface_name;
	char *dest_ip;
};

struct uci_portTriggerRule {
	struct ucimap_section_data map;
 	struct list_head list;

	char *name;
	int test; //This is just for us. No use with this variable. We can ignore later on.
	bool status;
	char *interface;
	char *protocol;
	int trigger_port_start;
	int trigger_port_end;
	int int_port_start;
	int int_port_end;
	char *int_protocol;
};

struct uci_blockurlRule {
	struct ucimap_section_data map;
	struct list_head list;

	const char *name;
	int test;

	char * domain_name;
	char *schedule;
};

struct uci_blockkeywordRule {
	struct ucimap_section_data map;
	struct list_head list;

	const char *name;
	int test;

	char * keyword;
	char *schedule;
};

struct uci_allowurlRule {
	struct ucimap_section_data map;
	struct list_head list;

	const char *name;
	int test;

	char * domain_name;
	char *schedule;
};

struct uci_allowkeywordRule {
	struct ucimap_section_data map;
	struct list_head list;

	const char *name;
	int test;

	char * keyword;
	char *schedule;
};

struct uci_staticnatRule {
    
    struct ucimap_section_data map;
    struct list_head list;
    
    const char *name;
    int test;
    bool status;
    char *interface;
    char *privateip;
    char *publicip;
    char *protocol;
    int range_length;
    int port_start;
    int port_end;
    int icmp_type;
    int protocol_number;
};

struct uci_policynatRule {
    struct ucimap_section_data map;
    struct list_head list;

    const char *name;
    int test;

    bool status;
    char *src_interface;
    char *dest_interface;
    char *orig_src_ip;
    char *trans_src_ip;
    char *orig_dest_ip;
    char *trans_dest_ip;
    char *protocol;
    int orig_port_start;
    int orig_port_end;
    int trans_port_start;
    int trans_port_end;

};

struct modules {
	unsigned int basic_settings : 1;
	unsigned int access_rules : 1;
	unsigned int static_nat : 1;
	unsigned int iface_nat : 1;
	unsigned int port_forwarding : 1;
	unsigned int dmz : 1;
	unsigned int port_triggering : 1;
	unsigned int spoof_rules: 1;
	unsigned int allWebContentFilter: 1;
    unsigned int policynat: 1;

	unsigned int fileID;
};

struct iptc_handle *create_handle(const char *tablename);
int config_basicsettings(struct uci_context *ctx);
void insert_lanvpn_http_rules(int port,struct iptc_handle *handle);
void insert_lanvpn_https_rules(int port,struct iptc_handle *handle);
int insert_lanvpn_management(struct uci_context *ctx,struct iptc_handle *filter_handle,struct iptc_handle *nat_handle);
void insert_block_wan_rules(struct iface_data *iface,struct iptc_handle *handle);
int insert_block_wan(struct uci_context *ctx,struct iptc_handle *filter_handle);
void insert_remote_management_forward_rules(struct iptc_handle *filter_handle,
                                           int remote_http,int remote_https,int port,char *remote_ipaddress,char *remote_end,struct iface_data *findIface);
void insert_remote_management_dnat_rules(struct iptc_handle *nat_handle,
                                                    int remote_http,int remote_https,int port,char *remote_ipaddress,char *remote_end,struct iface_data *findIface);
int insert_remote_management(struct uci_context *ctx,struct iptc_handle *filter_handle,struct iptc_handle *nat_handle);

void add_publicip(char *publicip,char *interface);
char * incrementIpAddr (char * ipAddr);
void tokenize_and_docommand(char *cmdBuff,char *table, struct iptc_handle *handle);
int strreplace(char *outputbuff,char * original_buff,char *search_string,char * replace_with);

void print_acl(struct uci_aclRule *acl);
void print_ifaceNAT(struct uci_ifaceNAT *ifNAT);
void print_portForward(struct uci_portForward *pf);
void print_portTrigger(struct uci_portTriggerRule *pt);
void print_usage(void);

int config_contentfilter(struct uci_context *ctx, struct iptc_handle *handle);
int config_webfeatures(struct uci_context *ctx, struct iptc_handle *handle);
int config_trusted(struct uci_context *ctx, struct iptc_handle *handle);
void config_allWebContentFiltering(struct uci_context *ctx);
void config_aclrule(struct uci_context *ctx);
void config_ifaceNAT(struct uci_context *ctx);
void config_portForwarding(struct uci_context *ctx);
void config_portTriggering(struct uci_context *ctx);
void config_dmzhost(struct uci_context *ctx);
void config_staticNAT(struct uci_context *ctx);
void config_spoof_rules(struct uci_context *ctx);
int config_staticnat(void);
void config_policynat(void);

int staticnat_reflection_in_dnat(struct uci_staticnatRule *staticnat,struct iptc_handle *nat_handle,char *publicip,char *privateip);
int staticnat_reflection_in(struct uci_staticnatRule *staticnat,struct iptc_handle *nat_handle,char *publicip,char *privateip);
int staticnat_reflection_out(struct uci_staticnatRule *staticnat,struct iptc_handle *nat_handle,char *publicip,char *privateip);

void flush_staticnat(struct iptc_handle *nat_handle,struct iptc_handle *filter_handle);
void flush_basicsettings(struct iptc_handle *filter_handle,struct iptc_handle *nat_handle);

void print_allowURLRule(struct uci_allowurlRule *au);
void print_blockURLRule(struct uci_blockurlRule *bu);
void print_allowKeywordRule(struct uci_allowkeywordRule *ak);
void print_blockKeywordRule(struct uci_blockkeywordRule *bk);
void print_staticnat(struct uci_staticnatRule *stNAT);
void print_basicsettings(void);

void insert_udp_flood(char *interface,struct iptc_handle * handle);
void insert_syn_flood(char *interface,struct iptc_handle * handle);
void insert_ping_of_death(char *interface,struct iptc_handle * handle);
void insert_icmp_flood(char *interface, struct iptc_handle * handle);
int insert_allowurlrule(struct uci_allowurlRule *url, struct uci_context *ctx, struct iptc_handle *handle);
int insert_allowkeywordrule(struct uci_allowkeywordRule *key, struct uci_context *ctx, struct iptc_handle *handle);
int insert_allowurlstringrule(struct uci_allowurlRule *url, struct uci_context *ctx, struct iptc_handle *handle);
int insert_allowkeywordstringrule(struct uci_allowkeywordRule *key, struct uci_context *ctx, struct iptc_handle *handle);
int insert_spoof_rules(struct uci_context *ctx, struct iptc_handle *handle);
int insert_blockrule(struct iptc_handle *handle);
int insert_policynat_forward(struct uci_policynatRule *pn,struct uci_context *ctx,struct iptc_handle *handle_filter);
int insert_policynat_dnat(struct uci_policynatRule *pn,struct uci_context *ctx,struct iptc_handle *handle_nat,char *orig_range_ip,char *trans_orig_ip,char *subnet_start,char *subnet_end,int prefix);
int insert_policynat_snat(struct uci_policynatRule *pn,struct uci_context *ctx,struct iptc_handle *handle_nat,char *orig_range_ip,char*trans_orig_ip,char *subnet_start,char *subnet_end,int mark,int prefix);
int insert_policynat(struct uci_policynatRule *pn, struct uci_context *ctx,struct iptc_handle *handle_filter, struct iptc_handle *handle_nat,int mark);
void add_snatip(char *ipaddress,struct iface_data *findIface);
void insert_lan_netconf_input(struct iptc_handle *handle,int netconf_port);
void insert_lan_netconf_dnat(struct iptc_handle *handle,int netconf_port,struct iface_data *findIface);
void insert_wan_netconf_dnat(struct iptc_handle *nat_handle,struct iface_data *findIface,int netconf_port);
void insert_wan_netconf_input(struct iptc_handle *filter_handle,struct iface_data *findIface,int netconf_port);
void insert_wan_restconf_dnat(struct iptc_handle *nat_handle,struct iface_data *findIface,int restconf_port);
void insert_wan_restconf_input(struct iptc_handle *filter_handle,struct iface_data *findIface,int restconf_port);
void insert_lan_restconf_input(struct iptc_handle *handle,int restconf_port);
void insert_lan_restconf_dnat(struct iptc_handle *handle,int restconf_port,struct iface_data *findIface);
int insert_netconf(struct uci_context *ctx,struct iptc_handle *filter_handle,struct iptc_handle *nat_handle);
int insert_restconf(struct uci_context *ctx,struct iptc_handle *filter_handle,struct iptc_handle *nat_handle);
int insert_policynat_output(struct uci_policynatRule *pn, struct uci_context *ctx,struct iptc_handle *handle_filter,struct iptc_handle *handle_nat);
void insert_lanvpn_dnat(int port,struct iptc_handle *handle_nat);
