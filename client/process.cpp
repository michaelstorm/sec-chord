#include <assert.h>
#include <string.h>
#include "chord.h"
#include "dhash.h"
#include "process.h"
#include "send.h"
#include "transfer.h"

int dhash_process_query(DHash *dhash, Server *srv, in6_addr *reply_addr,
						ushort reply_port, const char *file, Node *from)
{
	int file_len = strlen(file);
	if (file_len == 0) {
		fprintf(stderr, "dropping query for zero-length file from [%s]:%d ",
				v6addr_to_str(&from->addr), from->port);
		fprintf(stderr, "(reply addr [%s]:%d)\n", v6addr_to_str(reply_addr),
				reply_port);
		return 1;
	}

	fprintf(stderr, "received query from [%s]:%d for %s\n",
			v6addr_to_str(reply_addr), reply_port, file);

	/* if we have the file, notify the requesting node */
	if (dhash_local_file_exists(dhash, file)) {
		fprintf(stderr, "we have %s\n", file);
		dhash_send_query_reply_success(dhash, srv, reply_addr, reply_port,
									   file);

		Transfer *trans = new_transfer(srv->node.port+1, reply_addr,
									   reply_port+1, dhash->files_path, 0, 0, 0,
									   0);
		transfer_start_sending(trans);
	}
	else {
		fprintf(stderr, "we don't have %s\n", file);
		chordID id;
		get_data_id(&id, (const uchar *)file, strlen(file));

		/* if we should have the file, as its successor, but don't, also notify
		   the requesting node */
		if (chord_is_local(srv, &id)) {
			fprintf(stderr, "but we should, so we're replying\n", file);
			dhash_send_query_reply_failure(dhash, srv, reply_addr, reply_port,
										   file);
			fprintf(stderr, "and listening on port %d\n", srv->node.port);
		}
		/* otherwise, forward the request to the closest finger */
		else {
			fprintf(stderr, "so we're forwarding the query\n", file);
			return 0;
		}
	}

	fprintf(stderr, "and we're dropping the routing packet\n");
	return 1;
}

static void receive_success(Transfer *trans, void *arg)
{
	DHash *dhash = (DHash *)arg;
	dhash_send_control_query_success(dhash, trans->file);
	dhash_send_push(dhash, trans->file);
	free_transfer(trans);
}

static void receive_fail(Transfer *trans, void *arg)
{
	DHash *dhash = (DHash *)arg;
	dhash_send_control_query_failure(dhash, trans->file);
	free_transfer(trans);
}

int dhash_process_query_reply_success(DHash *dhash, Server *srv,
									  const char *file, Node *from)
{
	fprintf(stderr, "receiving transfer of \"%s\" from [%s]:%d\n", file,
			v6addr_to_str(&from->addr), from->port);

	Transfer *trans = new_transfer(srv->node.port+1, &from->addr, from->port+1,
								   dhash->files_path, receive_success,
								   receive_fail, dhash, dhash->ev_base);
	transfer_start_receiving(trans, file);
	return 0;
}

int dhash_process_query_reply_failure(DHash *dhash, Server *srv,
									  const char *file, Node *from)
{
	dhash_send_control_query_failure(dhash, file);
	return 0;
}

int dhash_process_push(DHash *dhash, Server *srv, in6_addr *reply_addr,
					   ushort reply_port, const char *file, Node *from)
{
	fprintf(stderr, "received push for \"%s\"\n", file);
	if (dhash_local_file_exists(dhash, file)) {
		fprintf(stderr, "but we already have the file, so not replying\n");
		return 0;
	}
	fprintf(stderr, "and sending push reply\n", file);

	dhash_send_push_reply(dhash, srv, reply_addr, reply_port, file);

	Transfer *trans = new_transfer(srv->node.port+1, reply_addr, reply_port+1,
								   dhash->files_path, 0, 0, 0, 0);
	transfer_start_receiving(trans, file);
	return 0;
}

int dhash_process_push_reply(DHash *dhash, Server *srv, const char *file,
							 Node *from)
{
	fprintf(stderr, "received push reply for \"%s\"\n", file);

	Transfer *trans = new_transfer(srv->node.port+1, &from->addr, from->port+1,
								   dhash->files_path, 0, 0, 0, 0);
	transfer_start_sending(trans);
}

int dhash_process_client_query(DHash *dhash, const char *file)
{
	if (dhash_local_file_exists(dhash, file))
		dhash_send_control_packet(dhash, DHASH_CLIENT_REPLY_LOCAL, file);
	else
		dhash_send_query(dhash, file);
	return 0;
}
