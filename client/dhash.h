#ifndef DHASH_H
#define DHASH_H

#include <netinet/in.h>

typedef void (*dhash_request_reply_handler)(void *ctx, char code,
											const char *file);

struct event;
struct event_base;
struct Server;
struct Transfer;
typedef struct DHash DHash;

struct DHash
{
	struct Server **servers;
	int *chord_tunnel_socks;
	int nservers;

	Transfer *trans_head;

	char *files_path;
	int control_sock;

	struct event_base *ev_base;
	struct event *udt_sock_event;
	struct event *control_sock_event;
};

enum
{
	DHASH_CLIENT_QUERY = 0,
};

enum
{
	DHASH_CLIENT_REPLY_LOCAL = 0,
	DHASH_CLIENT_REPLY_SUCCESS,
	DHASH_CLIENT_REPLY_FAILURE,
};

enum
{
	DHASH_QUERY = 0,
	DHASH_QUERY_REPLY_SUCCESS,
	DHASH_QUERY_REPLY_FAILURE,
	DHASH_PUSH,
	DHASH_PUSH_REPLY,
};

// in send.cpp
extern void dhash_client_send_request(int sock, const char *file);
// in pack.cpp
extern int dhash_client_unpack_request_reply(int sock, void *ctx,
											 dhash_request_reply_handler handler);

DHash *new_dhash(const char *files_path);
int dhash_start(DHash *dhash, char **conf_files, int nservers);

void dhash_add_transfer(DHash *dhash, Transfer *trans);
void dhash_remove_transfer(DHash *dhash, Transfer *remove);
void dhash_handle_transfer_statechange(Transfer *trans, int old_state,
									   void *arg);

void dhash_receive_file(DHash *dhash, int sock, int file_size, const char *file,
						in6_addr *addr, ushort port, int push);
void dhash_send_file(DHash *dhash, int sock, const char *file, in6_addr *addr,
					 ushort port, int push);

int dhash_stat_local_file(DHash *dhash, const char *file,
						  struct stat *stat_buf);
int dhash_local_file_exists(DHash *dhash, const char *file);
int dhash_local_file_size(DHash *dhash, const char *file);

#endif
