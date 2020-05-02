#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <uci.h>

#include "wifi_check.h"

struct uci_context *uci_ctx = NULL;
struct uci_context *uci_ctx_nvram = NULL;
struct uci_context *uci_ctx_old = NULL;

static enum {
	CLI_FLAG_MERGE =    (1 << 0),
	CLI_FLAG_QUIET =    (1 << 1),
	CLI_FLAG_NOCOMMIT = (1 << 2),
	CLI_FLAG_BATCH =    (1 << 3),
	CLI_FLAG_SHOW_EXT = (1 << 4),
} flags;

struct uci_type_list {
	unsigned int idx;
	const char *name;
	struct uci_type_list *next;
};

static struct uci_type_list *type_list = NULL;
static char *typestr = NULL;
static const char *cur_section_ref = NULL;

static void
uci_reset_typelist(void)
{
	struct uci_type_list *type;
	while (type_list != NULL) {
		type = type_list;
		type_list = type_list->next;
		free(type);
	}
	if (typestr) {
		free(typestr);
		typestr = NULL;
	}
	cur_section_ref = NULL;
}

static char *
uci_lookup_section_ref(struct uci_section *s)
{
	struct uci_type_list *ti = type_list;
	int maxlen;

	if (!s->anonymous || !(flags & CLI_FLAG_SHOW_EXT))
		return s->e.name;

	/* look up in section type list */
	while (ti) {
		if (strcmp(ti->name, s->type) == 0)
			break;
		ti = ti->next;
	}
	if (!ti) {
		ti = malloc(sizeof(struct uci_type_list));
		if (!ti)
			return NULL;
		memset(ti, 0, sizeof(struct uci_type_list));
		ti->next = type_list;
		type_list = ti;
		ti->name = s->type;
	}

	maxlen = strlen(s->type) + 1 + 2 + 10;
	if (!typestr) {
		typestr = malloc(maxlen);
	} else {
		typestr = realloc(typestr, maxlen);
	}

	if (typestr)
		sprintf(typestr, "@%s[%d]", ti->name, ti->idx);

	ti->idx++;

	return typestr;
}

wifi_section_t wifi_section_g = {0};
wifi_section_t wifi_section_old_g = {0};
wifi_nvram_t wifi_nvram_g = {0};

static void uci_get_section(wifi_section_t *entry_wifi_section, struct uci_section *s)
{
	struct uci_element *e;
	struct uci_option *o;
	wifi_section_t *entry = entry_wifi_section;

	entry->type = SECTION_TYPE_SSID;
	entry->s = s;

	uci_foreach_element(&s->options, e) {
		o = uci_to_option(e);
		if (strcmp(o->e.name, "ssid") == 0)
			strcpy(entry->ssid, o->v.string);

		if (strcmp(o->e.name, "device") == 0)
			strcpy(entry->radio, o->v.string);
	}
}

static void uci_get_section_nvram(wifi_nvram_t **entry_wifi_nvram, struct uci_section *s)
{
	struct uci_element *e;
	struct uci_option *o;
	char *iface_ptr = NULL;
	char iface_name[16] = {0};
	char iface_index[4] = {0};
	wifi_nvram_t *entry = NULL;

	uci_foreach_element(&s->options, e) {
		o = uci_to_option(e);
		if ((iface_ptr = strstr(o->e.name, "_ssid")) != NULL) {
			if (strlen(o->e.name) >= sizeof(iface_name))
				continue;

			entry = (wifi_nvram_t *)calloc(1, sizeof(wifi_nvram_t));
			if (entry == NULL)
				return;

			memset(iface_name, 0, sizeof(iface_name));
			memcpy(iface_name, o->e.name, strlen(o->e.name) - strlen(iface_ptr));
			if (iface_ptr = strstr(iface_name, "___")) {
				memcpy(entry->radio, iface_name, strlen(iface_name) - strlen(iface_ptr));
				strcpy(entry->index, iface_ptr+3);
				sprintf(entry->ifname, "%s.%s", entry->radio, entry->index);
			} else {
				strcpy(entry->radio, iface_name);
				strcpy(entry->index, "0");
				strcpy(entry->ifname, iface_name);
			}

			strcpy(entry->ssid, o->v.string);

			entry->next = *entry_wifi_nvram;
			*entry_wifi_nvram = entry;
		}	
	}
}

static int uci_get_package(wifi_section_t **entry_wifi_section, struct uci_package *p)
{
	struct uci_element *e;
	wifi_section_t *entry = NULL;

	uci_reset_typelist();
	uci_foreach_element(&p->sections, e) {
		struct uci_section *s = uci_to_section(e);
		if (strcmp(s->type, CONFIG_NAME) == 0) {
			entry = (wifi_section_t *)calloc(1, sizeof(wifi_section_t));
			if (entry == NULL)
				return -1;
			strcpy(entry->radio, s->e.name);
			entry->s = s;
			entry->type = SECTION_TYPE_RADIO;

			entry->next = *entry_wifi_section;
			*entry_wifi_section = entry;
		} else if (strcmp(s->type, SECTION_NAME) == 0) {
			entry = (wifi_section_t *)calloc(1, sizeof(wifi_section_t));
			if (entry == NULL)
				return -1;
			cur_section_ref = uci_lookup_section_ref(s);
			uci_get_section(entry, s);

			entry->next = *entry_wifi_section;
			*entry_wifi_section = entry;
		}
	}
	uci_reset_typelist();

	return 0;
}

static int uci_get_package_nvram(wifi_nvram_t **entry_wifi_nvram, struct uci_package *p)
{
	struct uci_element *e;

	uci_reset_typelist();
	uci_foreach_element(&p->sections, e) {
		struct uci_section *s = uci_to_section(e);

		cur_section_ref = uci_lookup_section_ref(s);
		uci_get_section_nvram(entry_wifi_nvram, s);
	}
	uci_reset_typelist();

	return 0;
}

static wifi_radio_index_t radio_index_g = {0};

#define CHECK_RADIO_CMD	1
#define CHECK_SSID_CMD	0

int main(int argc, char **argv)
{
	int radio_len = sizeof(wifi_radio_options)/sizeof(wifi_uci_t);
	int ssid_len = sizeof(wifi_ssid_options)/sizeof(wifi_uci_t);
	int idx = 0;
	char output[256] = {0};
	const char *opt;
	const char *opt_old;
	struct uci_ptr ptr;
	struct uci_ptr ptr_nvram;
	struct uci_ptr ptr_old;
	wifi_section_t *entry = NULL;
	wifi_section_t *entry_old = NULL;
	wifi_section_t *next = NULL;
	wifi_nvram_t *entry_nvram = NULL;
	wifi_nvram_t *next_nvram = NULL;
	wifi_radio_index_t *entry_radio = NULL;
	wifi_radio_index_t *next_radio = NULL;
	char *radio = NULL;
	char *cmd = NULL;
	int cmd_flag = CHECK_SSID_CMD;
	int ssid_is_updating = 0;
	int ssid_is_disabled = 0;

	if (argc < 3)
		return -1;

	++argv;

	if (*argv) {
		cmd = *argv;
		++argv;
		if (*argv) {
			radio = *argv;
		} else
			return -1;

	} else {
		return -1;
	}

	if (strcmp(cmd, "radio") == 0)
		cmd_flag = CHECK_RADIO_CMD;

	if (!uci_ctx) {
		uci_ctx = uci_alloc_context();
		if (!uci_ctx)
			return -1;
	}

	if (!uci_ctx_old) {
		uci_ctx_old = uci_alloc_context();
		if (!uci_ctx_old) {
			uci_free_context(uci_ctx);
			uci_ctx = NULL;
			return -1;
		}
	}

	if (!uci_ctx_nvram) {
		uci_ctx_nvram = uci_alloc_context();
		if (!uci_ctx_nvram) {
			uci_free_context(uci_ctx);
			uci_free_context(uci_ctx_old);
			uci_ctx = NULL;
			uci_ctx_old = NULL;
			return -1;
		}
	}

	if (uci_lookup_ptr(uci_ctx, &ptr_nvram, "wificonfig", true) != UCI_OK)
		goto err;
	if (0 != uci_get_package_nvram(&wifi_nvram_g.next, ptr_nvram.p))
		goto err;

	if (uci_lookup_ptr(uci_ctx, &ptr, "wireless", true) != UCI_OK)
		goto err;
	if (0 != uci_get_package(&wifi_section_g.next, ptr.p))
		goto err;

	if (uci_lookup_ptr(uci_ctx_old, &ptr_old, "wireless_old", true) != UCI_OK)
		goto err;
	if (0 != uci_get_package(&wifi_section_old_g.next, ptr_old.p))
		goto err;

	for (entry_nvram = wifi_nvram_g.next; entry_nvram != NULL; entry_nvram = entry_nvram->next) {
		if (strcmp(radio, entry_nvram->radio) != 0) {
			continue;
		}

		for (entry_radio = radio_index_g.next; entry_radio != NULL; entry_radio = entry_radio->next) {
			if (strcmp(entry_nvram->radio, entry_radio->radio) == 0) {
				entry_radio->index_used |= (1 << atoi(entry_nvram->index));
				break;
			}
		}
		if (NULL == entry_radio) {
			entry_radio = (wifi_radio_index_t *)calloc(1, sizeof(wifi_radio_index_t));
			if (NULL == entry_radio)
				goto err;

			strcpy(entry_radio->radio, entry_nvram->radio);
			entry_radio->index_used |= (1 << atoi(entry_nvram->index));
			entry_radio->next = radio_index_g.next;
			radio_index_g.next = entry_radio;
		}

		for (entry = wifi_section_g.next; entry != NULL; entry = entry->next) {
			if (entry->type == SECTION_TYPE_SSID &&
				strcmp(entry_nvram->ssid, entry->ssid) == 0 &&
				strcmp(entry_nvram->radio, entry->radio) == 0) {
				break;
			}
		}

		if (NULL == entry) {
			entry_nvram->del_flag = 1;
		} else {
			if (cmd_flag == CHECK_SSID_CMD)
				printf("init %s\n%s\n", entry_nvram->index, entry_nvram->ssid);
		}
	} /* end of for (entry_nvram = ...) */
	
	for (entry_radio = radio_index_g.next; entry_radio != NULL; entry_radio = entry_radio->next) {
		if (strcmp(radio, entry_radio->radio) == 0) {
			break;
		}
	}

	if (NULL == entry_radio) {
		entry_radio = (wifi_radio_index_t *)calloc(1, sizeof(wifi_radio_index_t));
		if (NULL == entry_radio)
			goto err;

		strcpy(entry_radio->radio, radio);
		entry_radio->index_used = 0;
		entry_radio->next = radio_index_g.next;
		radio_index_g.next = entry_radio;
	}

	for (entry = wifi_section_g.next; entry != NULL; entry = entry->next) {
		if (strcmp(radio, entry->radio) != 0) {
			continue;
		}
		if (entry->type == SECTION_TYPE_RADIO) {
			for (entry_old = wifi_section_old_g.next; entry_old != NULL; entry_old = entry_old->next) {
				if ((entry_old->type == SECTION_TYPE_RADIO) &&
					(strcmp(entry_old->radio, entry->radio) == 0)) {
					break;
				}
			}
			if (NULL == entry_old) {
				continue;
			}

			for (idx = 0; idx < radio_len; idx++) {
				opt = uci_lookup_option_string(uci_ctx, entry->s, wifi_radio_options[idx].option);
				opt_old = uci_lookup_option_string(uci_ctx_old, entry_old->s, wifi_radio_options[idx].option);
				if ((opt && opt_old && (strcmp(opt, opt_old) == 0)) ||
					(!opt && !opt_old)) {
					continue;
				} else {
					if (cmd_flag == CHECK_RADIO_CMD)
						printf("radio %s\n", entry->radio);
					break;
				}
			}
		} else if (entry->type == SECTION_TYPE_SSID) {
			ssid_is_updating = 0;
			ssid_is_disabled = 0;

			for (entry_old = wifi_section_old_g.next; entry_old != NULL; entry_old = entry_old->next) {
				if ((entry_old->type == SECTION_TYPE_SSID) &&
					(strcmp(entry_old->ssid, entry->ssid) == 0) &&
					(strcmp(entry_old->radio, entry->radio) == 0)) {
					break;
				}
			}
			if (NULL == entry_old) {
				for (entry_nvram = wifi_nvram_g.next; entry_nvram != NULL; entry_nvram = entry_nvram->next) {
					if (strcmp(entry_nvram->radio, entry->radio) == 0 &&
						entry_nvram->del_flag == 1) {
						entry_nvram->del_flag = 0;
						if (cmd_flag == CHECK_SSID_CMD)
							printf("rename %s\n%s\n", entry_nvram->index, entry->ssid);
						break;
					}
				} /* end of for (entry_nvram = ...) */

				if (NULL == entry_nvram) {
					for (entry_radio = radio_index_g.next; entry_radio != NULL; entry_radio = entry_radio->next) {
						if (strcmp(radio, entry_radio->radio) != 0) {
							continue;
						}

						int to_idx = 0;
						for (to_idx = 0; to_idx < 4; to_idx++) {
							if ((entry_radio->index_used & (1 << to_idx)) == 0) {
								entry_radio->index_used |= (1 << to_idx);
								if (cmd_flag == CHECK_SSID_CMD)
									printf("add %d\n%s\n", to_idx, entry->ssid);
								break;
							}
						}
					}
				}
				continue;
			}

			for (entry_nvram = wifi_nvram_g.next; entry_nvram != NULL; entry_nvram = entry_nvram->next) {
				if (strcmp(entry_nvram->ssid, entry->ssid) == 0 &&
					strcmp(entry_nvram->radio, entry->radio) == 0) {
					break;
				}
			} /* end of for (entry_nvram = ...) */

			for (idx = 0; idx < ssid_len; idx++) {
				opt = uci_lookup_option_string(uci_ctx, entry->s, wifi_ssid_options[idx].option);
				opt_old = uci_lookup_option_string(uci_ctx_old, entry_old->s, wifi_ssid_options[idx].option);
				if (strcmp(wifi_ssid_options[idx].option, "disabled") == 0) {
					if (opt && opt_old && (strcmp(opt, "1") == 0) && (strcmp(opt, opt_old) == 0)) {
						ssid_is_disabled = 1;
					}
				}
				if ((opt && opt_old && (strcmp(opt, opt_old) == 0)) || (!opt && !opt_old)) {
					if (strcmp(wifi_ssid_options[idx].option, "captive") == 0) {
						if (opt && opt_old && (strcmp(opt, "1") == 0) && (strcmp(opt, opt_old) == 0)) {
							opt = uci_lookup_option_string(uci_ctx, entry->s, "captive_portal_profile");
							opt_old = uci_lookup_option_string(uci_ctx_old, entry_old->s, "captive_portal_profile");
						}
					} else if (strcmp(wifi_ssid_options[idx].option, "encryption") == 0) {
						if (opt && opt_old && (strcmp(opt, opt_old) == 0)) {
							opt = uci_lookup_option_string(uci_ctx, entry->s, "key");
							opt_old = uci_lookup_option_string(uci_ctx_old, entry_old->s, "key");
							if (opt && opt_old && (strcmp(opt, opt_old) == 0)) {
								opt = uci_lookup_option_string(uci_ctx, entry->s, "server");
								opt_old = uci_lookup_option_string(uci_ctx_old, entry_old->s, "server");
								if (opt && opt_old && (strcmp(opt, opt_old) == 0)) {
									opt = uci_lookup_option_string(uci_ctx, entry->s, "port");
									opt_old = uci_lookup_option_string(uci_ctx_old, entry_old->s, "port");
								}
							}
						}
					} else if (strcmp(wifi_ssid_options[idx].option, "mac_filter") == 0) {
						if (opt && opt_old && (strcmp(opt, "1") == 0) && (strcmp(opt, opt_old) == 0)) {
							opt = uci_lookup_option_string(uci_ctx, entry->s, "macpolicy");
							opt_old = uci_lookup_option_string(uci_ctx_old, entry_old->s, "macpolicy");
						}

						if (opt && opt_old && (strcmp(opt, "0") != 0) && (strcmp(opt, opt_old) == 0)) {
							struct uci_option *_o;
							struct uci_option *_o_old;
							struct uci_element *_e;
							struct uci_element *_e_old;
							_o = uci_lookup_option(uci_ctx, entry->s, "maclist");
							_o_old = uci_lookup_option(uci_ctx_old, entry_old->s, "maclist");
							if (_o && _o_old) {
                                if (_o->type == UCI_TYPE_LIST) {
                                    uci_foreach_element(&_o->v.list, _e) {
                                        uci_foreach_element(&_o_old->v.list, _e_old) {
                                            if (strcmp(_e->name, _e_old->name) == 0)
                                                break;
                                        }
                                        if (&_e_old->list == &_o_old->v.list)
                                            break;
                                    }
                                    if (&_e->list != &_o->v.list) {
                                        if (NULL != entry_nvram) {
                                            if (cmd_flag == CHECK_SSID_CMD)
                                                printf("update %s\n%s\n", entry_nvram->index, entry->ssid);
                                        } else {
                                            for (entry_radio = radio_index_g.next; entry_radio != NULL; entry_radio = entry_radio->next) {
                                                if (strcmp(radio, entry_radio->radio) != 0) {
                                                    continue;
                                                }

                                                int to_idx = 0;
                                                for (to_idx = 0; to_idx < 4; to_idx++) {
                                                    if ((entry_radio->index_used & (1 << to_idx)) == 0) {
                                                        entry_radio->index_used |= (1 << to_idx);
                                                        if (cmd_flag == CHECK_SSID_CMD) {
                                                            printf("update %d\n%s\n", to_idx, entry->ssid);
                                                            ssid_is_updating = 1;
                                                        }
                                                        break;
                                                    }
                                                }
                                            }
                                        }
                                        break;
                                    }

                                    uci_foreach_element(&_o_old->v.list, _e_old) {
                                        uci_foreach_element(&_o->v.list, _e) {
                                            if (strcmp(_e->name, _e_old->name) == 0)
                                                break;
                                        }
                                        if (&_e->list == &_o->v.list)
                                            break;
                                    }
                                    if (&_e_old->list != &_o_old->v.list) {
                                        if (NULL != entry_nvram) {
                                            if (cmd_flag == CHECK_SSID_CMD)
                                                printf("update %s\n%s\n", entry_nvram->index, entry->ssid);
                                        } else {
                                            for (entry_radio = radio_index_g.next; entry_radio != NULL; entry_radio = entry_radio->next) {
                                                if (strcmp(radio, entry_radio->radio) != 0) {
                                                    continue;
                                                }

                                                int to_idx = 0;
                                                for (to_idx = 0; to_idx < 4; to_idx++) {
                                                    if ((entry_radio->index_used & (1 << to_idx)) == 0) {
                                                        entry_radio->index_used |= (1 << to_idx);
                                                        if (cmd_flag == CHECK_SSID_CMD) {
                                                            printf("update %d\n%s\n", to_idx, entry->ssid);
                                                            ssid_is_updating = 1;
                                                        }
                                                        break;
                                                    }
                                                }
                                            }
                                        }
                                        break;
                                    }
                                } else if (_o->type == UCI_TYPE_STRING) {
                                    opt = uci_lookup_option_string(uci_ctx, entry->s, "maclist");
                                    opt_old = uci_lookup_option_string(uci_ctx_old, entry_old->s, "maclist");
                                }
							} else if (!_o && !_o_old) {
								continue;
							} else {
								if (NULL != entry_nvram) {
									if (cmd_flag == CHECK_SSID_CMD)
										printf("update %s\n%s\n", entry_nvram->index, entry->ssid);
								} else {
									for (entry_radio = radio_index_g.next; entry_radio != NULL; entry_radio = entry_radio->next) {
										if (strcmp(radio, entry_radio->radio) != 0) {
											continue;
										}

										int to_idx = 0;
										for (to_idx = 0; to_idx < 4; to_idx++) {
											if ((entry_radio->index_used & (1 << to_idx)) == 0) {
												entry_radio->index_used |= (1 << to_idx);
												if (cmd_flag == CHECK_SSID_CMD) {
													printf("update %d\n%s\n", to_idx, entry->ssid);
													ssid_is_updating = 1;
												}
												break;
											}
										}
									}
								}
								break;
							}
						}
					}

					if ((opt && opt_old && (strcmp(opt, opt_old) == 0)) || (!opt && !opt_old)) {
						continue;
					} else {
						if (NULL != entry_nvram) {
							if (cmd_flag == CHECK_SSID_CMD)
								printf("update %s\n%s\n", entry_nvram->index, entry->ssid);
						} else {
							for (entry_radio = radio_index_g.next; entry_radio != NULL; entry_radio = entry_radio->next) {
								if (strcmp(radio, entry_radio->radio) != 0) {
									continue;
								}

								int to_idx = 0;
								for (to_idx = 0; to_idx < 4; to_idx++) {
									if ((entry_radio->index_used & (1 << to_idx)) == 0) {
										entry_radio->index_used |= (1 << to_idx);
										if (cmd_flag == CHECK_SSID_CMD) {
											printf("update %d\n%s\n", to_idx, entry->ssid);
											ssid_is_updating = 1;
										}
										break;
									}
								}
							}
						}
						break;
					}
					continue;
				} else {
					if (NULL != entry_nvram) {
						if (cmd_flag == CHECK_SSID_CMD)
							printf("update %s\n%s\n", entry_nvram->index, entry->ssid);
					} else {
						for (entry_radio = radio_index_g.next; entry_radio != NULL; entry_radio = entry_radio->next) {
							if (strcmp(radio, entry_radio->radio) != 0) {
								continue;
							}

							int to_idx = 0;
							for (to_idx = 0; to_idx < 4; to_idx++) {
								if ((entry_radio->index_used & (1 << to_idx)) == 0) {
									entry_radio->index_used |= (1 << to_idx);
									if (cmd_flag == CHECK_SSID_CMD) {
										printf("update %d\n%s\n", to_idx, entry->ssid);
										ssid_is_updating = 1;
									}
									break;
								}
							}
						}
					}
					break;
				}
			}
#if 0
			if (ssid_is_disabled == 1 && ssid_is_updating == 0) {
				for (entry_radio = radio_index_g.next; entry_radio != NULL; entry_radio = entry_radio->next) {
					if (strcmp(radio, entry_radio->radio) != 0) {
						continue;
					}

					int to_idx = 0;
					for (to_idx = 0; to_idx < 4; to_idx++) {
						if ((entry_radio->index_used & (1 << to_idx)) == 0) {
							entry_radio->index_used |= (1 << to_idx);
							if (cmd_flag == CHECK_SSID_CMD)
								printf("init %d\n%s\n", to_idx, entry->ssid);
							break;
						}
					}
				}
			}
#endif
		} /* end of else if (entry->type == SECTION_TYPE_SSID) */
	} /* end of for (entry = wifi_section_g.next...) */

	for (entry_nvram = wifi_nvram_g.next; entry_nvram != NULL; entry_nvram = entry_nvram->next) {
		if (strcmp(radio, entry_nvram->radio) != 0) {
			continue;
		}
		if (entry_nvram->del_flag == 1 && cmd_flag == CHECK_SSID_CMD)
			printf("del %s\n%s\n", entry_nvram->index, entry_nvram->ssid);
	}

err:
	entry = wifi_section_g.next;
	while (entry) {
		next = entry->next;
		free(entry);
		entry = next;
	}

	entry = wifi_section_old_g.next;
	while (entry) {
		next = entry->next;
		free(entry);
		entry = next;
	}

	entry_nvram = wifi_nvram_g.next;
	while (entry_nvram) {
		next_nvram = entry_nvram->next;
		free(entry_nvram);
		entry_nvram = next_nvram;
	}
	
	entry_radio = radio_index_g.next;
	while (entry_radio) {
		next_radio = entry_radio->next;
		free(entry_radio);
		entry_radio = next_radio;
	}

	if (uci_ctx) {
		uci_free_context(uci_ctx);
		uci_ctx = NULL;
	}

	if (uci_ctx_old) {
		uci_free_context(uci_ctx_old);
		uci_ctx_old = NULL;
	}

	if (uci_ctx_nvram) {
		uci_free_context(uci_ctx_nvram);
		uci_ctx_nvram = NULL;
	}

	return 0;
}
