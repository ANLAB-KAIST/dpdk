/*
 * trace.c
 *
 *  Created on: Feb 27, 2016
 *      Author: khlee
 */

#include "trace.h"
#include "common.h"
#include <stdint.h>
#include <rte_byteorder.h>
#include <rte_common.h>
#include <rte_memcpy.h>

struct pcap_file_header {
	uint32_t magic;
	u_short version_major;
	u_short version_minor;
	uint32_t thiszone;     /* gmt to local correction */
	uint32_t sigfigs;    /* accuracy of timestamps */
	uint32_t snaplen;    /* max length saved portion of each pkt */
	uint32_t linktype;   /* data link type (LINKTYPE_*) */
};

struct pcap_packet_header {
	uint32_t ts_sec;         /* timestamp seconds */
	uint32_t ts_usec;        /* timestamp microseconds */
	uint32_t incl_len;       /* number of octets of packet saved in file */
	uint32_t orig_len;       /* actual length of packet */
};


static const uint32_t MAGIC = 0xa1b2c3d4;
static const uint32_t NANO_MAGIC = 0xa1b23c4d;
static const uint32_t MAGIC_SWAP = 0xd4c3b2a1;
static const uint32_t NANO_MAGIC_SWAP = 0x4d3cb2a1;

static int pcap_is_host_endian(const void* content)
{
	const struct pcap_file_header* hdr = content;
	if(hdr->magic == MAGIC || hdr->magic == NANO_MAGIC)
		return 1;
	if(hdr->magic == MAGIC_SWAP || hdr->magic == NANO_MAGIC_SWAP)
		return 0;
	return -1;
}

int pcap_begin(const void* content, const void** next)
{
	int ret = pcap_is_host_endian(content);
	if(ret < 0)
	{
		*next = NULL;
		return ret;
	}
	const struct pcap_file_header* hdr = content;
	*next = (const void*)(hdr+1);
	return ret;
}

unsigned pcap_current_length(const void* const * next, int endian)
{
	const struct pcap_packet_header* hdr = *next;
	unsigned orig_length = 0;
	if(endian == 1)
	{
		orig_length = hdr->orig_len;
	}
	else if(endian == 0)
	{
		orig_length = rte_bswap32(hdr->orig_len);
	}
	return orig_length;
}

unsigned pcap_next(void* buffer, unsigned buffer_len,
		struct random_data* random_data, const void** next, int endian)
{
	const struct pcap_packet_header* hdr = *next;
	const void* content = (const void*)(hdr+1);
	unsigned incl_length = 0;
	unsigned orig_length = 0;
	if(endian == 1)
	{
		incl_length = hdr->incl_len;
		orig_length = hdr->orig_len;
	}
	else if(endian == 0)
	{
		incl_length = rte_bswap32(hdr->incl_len);
		orig_length = rte_bswap32(hdr->orig_len);
	}
	incl_length = RTE_MIN(incl_length, buffer_len);
	orig_length = RTE_MIN(orig_length, buffer_len);

	*next = RTE_PTR_ADD(content, incl_length);

	rte_memcpy(buffer, content, incl_length);

	void* rem = RTE_PTR_ADD(buffer, incl_length);
	unsigned rem_len = orig_length - incl_length;

	int32_t fill = 0;
	if(random_data)
	{
		random_r(random_data, &fill);
	}
	memset(rem, fill, rem_len);

	return orig_length;
}

unsigned void_pcap_size(void* aux)
{
	struct queue_aux* queue_aux = (struct queue_aux*)aux;
	return pcap_current_length(&queue_aux->trace_ptr, queue_aux->trace_byteorder);
}

void* void_pcap_rx(void* buf, unsigned length, void* aux)
{
	struct queue_aux* queue_aux = (struct queue_aux*)aux;
	pcap_next(buf, length, &queue_aux->rand_data, &queue_aux->trace_ptr, queue_aux->trace_byteorder);
	if(queue_aux->trace_ptr == queue_aux->trace_end)
		queue_aux->trace_ptr = queue_aux->trace_start;
	return NULL;
}
