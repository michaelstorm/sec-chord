#include <openssl/evp.h>
#include <string.h>
#include "chord.h"

#undef C_DEBUG_ON

#ifdef C_DEBUG_ON
#define C_DEBUG(x) x
#else
#define C_DEBUG(x)
#endif

void vpack_hash(EVP_MD_CTX *ctx, const char *fmt, va_list args)
{
	char c;
	ushort s;
	ulong l;
	chordID *id;
	in6_addr *v6addr;
	
	for (; *fmt != '\0'; fmt++) {
		switch (*fmt) {
		case 'c':	 /* char */
			c = va_arg(args, int);
			LogTrace("hashing char %x", (char)c);
			EVP_DigestUpdate(ctx, &c, 1);
			break;
		case 's':	 /* short */
			s = va_arg(args, int);
			LogTrace("hashing short %x", (short)s);
			EVP_DigestUpdate(ctx, &s, sizeof(ushort));
			break;
		case 'l':	 /* long */
			l = va_arg(args, ulong);
			LogTrace("hashing long %lx", l);
			EVP_DigestUpdate(ctx, &l, sizeof(ulong));
			break;
		case 'x':	 /* id */
			id = va_arg(args, chordID *);
			LogTrace("hashing id %s", buf_to_hex(id->x, CHORD_ID_LEN));
			EVP_DigestUpdate(ctx, id->x, CHORD_ID_LEN);
			break;
		case '6':
			v6addr = va_arg(args, in6_addr *);
			LogTrace("hashing address %s", v6addr_to_str(v6addr));
			EVP_DigestUpdate(ctx, v6addr->s6_addr, 16);
			break;
		default:	 /* illegal type character */
			LogError("bad ticket type %c", *fmt);
			break;
		}
	}
}

void pack_hash(EVP_MD_CTX *ctx, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vpack_hash(ctx, fmt, args);
	va_end(args);
}

int pack_ticket(const uchar *salt, int salt_len, int hash_len, const uchar *out,
				const char *fmt, ...)
{
	va_list args;
	uchar md_value[EVP_MAX_MD_SIZE];
	uint32_t epoch_time = (uint32_t)time(NULL);

	// hash the unix epoch time together with the caller's arguments
	EVP_MD_CTX ctx;
	EVP_MD_CTX_init(&ctx);
	EVP_DigestInit_ex(&ctx, EVP_sha1(), NULL);

	pack_hash(&ctx, "l", epoch_time);

	va_start(args, fmt);
	vpack_hash(&ctx, fmt, args);
	va_end(args);

	unsigned int len;
	EVP_DigestFinal_ex(&ctx, md_value, &len);
	EVP_MD_CTX_cleanup(&ctx);

	// pack the 32-bit epoch time and 32-bit hash into a buffer
	Ticket ticket = TICKET__INIT;
	ticket.time = epoch_time;
	ticket.hash.len = hash_len;
	ticket.hash.data = md_value;
	
	LogMessage(DEBUG, "Packed ticket:", &ticket.base);
	
	return ticket__pack(&ticket, (uint8_t *)out);
}

int verify_ticket(const uchar *salt, int salt_len, int hash_len,
				  const uchar *ticket_buf, int ticket_len, const char *fmt, ...)
{
	va_list args;
	uchar md_value[EVP_MAX_MD_SIZE];

	// decrypt the ticket
	Ticket *ticket = ticket__unpack(NULL, ticket_len, ticket_buf);
	if (!ticket) {
		LogInfo("Ticket verification failed because the ticket could not be unpacked");
		goto fail;
	}
	if (ticket->hash.len != hash_len) {
		LogInfo("Ticket verification failed because ticket length %ul does not match expected length %ul", ticket->hash.len, hash_len);
		goto fail;
	}

	LogMessage(DEBUG, "Verifying ticket:", &ticket->base);

	time_t current_time = time(NULL);
	if (ticket->time < current_time-TICKET_TIMEOUT) {
		LogInfo("Ticket failed due to timeout; ticket timestamp is %ul, current time is %ul, configured ticket timeout is %ul", ticket->time, current_time, TICKET_TIMEOUT);
		goto fail;
	}

	// hash together the time provided in the ticket with the data given in the
	// arguments (that were presumably in the packet received) to verify that
	// the remote host didn't modify our ticket
	EVP_MD_CTX ctx;
	EVP_MD_CTX_init(&ctx);
	EVP_DigestInit_ex(&ctx, EVP_sha1(), NULL);

	pack_hash(&ctx, "l", ticket->time);

	va_start(args, fmt);
	vpack_hash(&ctx, fmt, args);
	va_end(args);
	
	unsigned int len;
	EVP_DigestFinal_ex(&ctx, md_value, &len);
	EVP_MD_CTX_cleanup(&ctx);
	
	LogDebug("Expecting ticket hash: %s", buf_to_hex(md_value, hash_len));
	LogDebug("Message ticket hash:   %s", buf_to_hex(ticket->hash.data, hash_len));

	int ret = memcmp(md_value, ticket->hash.data, hash_len) == 0;
	if (!ret)
		LogDebug("Ticket verification failed because hashes do not match");
	
	ticket__free_unpacked(ticket, NULL);
	return ret;

fail:
	ticket__free_unpacked(ticket, NULL);
	return 0;
}

void get_data_id(chordID *id, const uchar *buf, int n)
{
	EVP_MD_CTX ctx;
	EVP_MD_CTX_init(&ctx);
	EVP_DigestInit_ex(&ctx, EVP_sha1(), NULL);

	EVP_DigestUpdate(&ctx, buf, n);

	unsigned int len;
	EVP_DigestFinal_ex(&ctx, id->x, &len);
	EVP_MD_CTX_cleanup(&ctx);
}

void get_address_id(chordID *id, in6_addr *addr, ushort port)
{
	EVP_MD_CTX ctx;
	EVP_MD_CTX_init(&ctx);
	EVP_DigestInit_ex(&ctx, EVP_sha1(), NULL);

	pack_hash(&ctx, "6s", addr, htons(port));

	unsigned int len;
	EVP_DigestFinal_ex(&ctx, id->x, &len);
	EVP_MD_CTX_cleanup(&ctx);
}

int verify_address_id(chordID *id, in6_addr *addr, ushort port)
{
	chordID correct_id;
	get_address_id(&correct_id, addr, port);
	return equals(&correct_id, id);
}
