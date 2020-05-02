#ifndef __WIFI_CHECK_H__
#define __WIFI_CHECK_H__

#define CONFIG_NAME		"wifi-device"
#define SECTION_NAME	"wifi-iface"

#define CONFIG_STR		"Radio"
#define SECTION_STR		"SSID"

#define SSID_LENGTH	32

typedef struct _wifi_uci_s {
	char config[32];
	char option[32];
	int log_mask;
	char log_str[16];
} wifi_uci_t;

#define SECTION_TYPE_SSID	0
#define SECTION_TYPE_RADIO	1

typedef struct _wifi_section_s {
	int type;
	char ssid[SSID_LENGTH+1];
	char radio[8];
	struct uci_section *s;
	struct _wifi_section_s *next;
} wifi_section_t;

typedef struct _wifi_nvram_s {
	char ssid[SSID_LENGTH+1];
	char index[4];
	char radio[8];
	char ifname[16];
	int del_flag;
	struct _wifi_nvram_s *next;
} wifi_nvram_t;

typedef struct _wifi_radio_index_s {
	char radio[8];
	unsigned int index_used;
	struct _wifi_radio_index_s *next;
} wifi_radio_index_t;

wifi_uci_t wifi_radio_options[] = {
	{CONFIG_NAME,	"disabled",				1,	CONFIG_STR},
	{CONFIG_NAME,	"hwmode",				2,	CONFIG_STR},
	{CONFIG_NAME,	"channel",				2,	CONFIG_STR},
	{CONFIG_NAME,	"htmode",				2,	CONFIG_STR},
	{CONFIG_NAME,	"frameburst",			1,	CONFIG_STR},
	{CONFIG_NAME,	"wme_no_ack",			1,	CONFIG_STR},
	{CONFIG_NAME,	"basic_rate",			2,	CONFIG_STR},
	{CONFIG_NAME,	"transmit_rate",		2,	CONFIG_STR},
	{CONFIG_NAME,	"ht_mcs",				2,	CONFIG_STR},
	{CONFIG_NAME,	"vht_mcs_1",				2,	CONFIG_STR},
	{CONFIG_NAME,	"vht_mcs_2",				2,	CONFIG_STR},
	{CONFIG_NAME,	"vht_mcs_3",				2,	CONFIG_STR},
	{CONFIG_NAME,	"vht_mcs_4",				2,	CONFIG_STR},
	{CONFIG_NAME,	"cts_protection_mode",	1,	CONFIG_STR},
	{CONFIG_NAME,	"beacon_int",			2,	CONFIG_STR},
	{CONFIG_NAME,	"mu_mimo",				1,	CONFIG_STR},
	{CONFIG_NAME,	"beamforming",			1,	CONFIG_STR},
	{CONFIG_NAME,	"dtim_period",			1,	CONFIG_STR},
	{CONFIG_NAME,	"frag",					2,	CONFIG_STR},
	{CONFIG_NAME,	"rts",					2,	CONFIG_STR},
	{CONFIG_NAME,	"wme_apsd",				1,	CONFIG_STR},
	{CONFIG_NAME,	"txpower",				2,	CONFIG_STR},
	{CONFIG_NAME,	"maxassoc",				2,	CONFIG_STR},
};

wifi_uci_t wifi_ssid_options[] = {
	{SECTION_NAME,	"disabled",				1,	SECTION_STR},
	{SECTION_NAME,	"hidden",				1,	SECTION_STR},
	{SECTION_NAME,	"wmm",					1,	SECTION_STR},
	{SECTION_NAME,	"pmf",					1,	SECTION_STR},
	{SECTION_NAME,	"vlanid",				2,	SECTION_STR},
	{SECTION_NAME,	"isolate",				1,	SECTION_STR},
	{SECTION_NAME,	"wps",					1,	SECTION_STR},
	{SECTION_NAME,	"mac_filter",			1,	SECTION_STR},
	{SECTION_NAME,	"captive",				1,	SECTION_STR},
	{SECTION_NAME,	"encryption",			2,	SECTION_STR},
	{SECTION_NAME,	"schedule",				2,	SECTION_STR},
};

#endif
