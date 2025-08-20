#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <errno.h>

#define packed __attribute__((packed))

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

enum icmp_type {
    ICMP_NONE,
    ICMP_ECHO,
    ICMP_REPLY
};

typedef enum icmp_type icmp_kind;

struct raw_icmp {
    u8 type;
    u8 code;
    u16 checksum;
    u16 identifier;
    u16 sequence;
    u8 payload[];
} packed;

typedef struct {
    icmp_kind kind;
    u16 identifier;
    u16 sequence;
    u16 length;
    u8 *payload;
} icmp_packet;

void dumphex(u8 *buf, u16 len, u8 newline) {
    for (u16 i = 0; i < len; i++)
        printf("%02x ", buf[i]);
    if (newline) printf("\n");
}

void clearbuf(u8 *buf, u16 len) {
    memset(buf, 0, len);
}

u16 compute_checksum(u8 *data, u16 len) {
    u32 sum = 0;
    u16 *p = (u16 *)data;

    while (len > 1) {
        sum += *p++;
        len -= 2;
    }

    if (len == 1)
        sum += *(u8 *)p;

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    return ~((u16)sum);
}

icmp_packet *create_icmp(icmp_kind kind, const u8 *data, u16 len, u16 id, u16 seq) {
    icmp_packet *pkt = malloc(sizeof(icmp_packet));
    assert(pkt);

    pkt->kind = kind;
    pkt->identifier = id;
    pkt->sequence = seq;
    pkt->length = len;
    pkt->payload = malloc(len);
    assert(pkt->payload);
    memcpy(pkt->payload, data, len);

    return pkt;
}

u8 *serialize_icmp(icmp_packet *pkt, u16 *out_len) {
    if (!pkt || !pkt->payload) return NULL;

    u16 total_len = sizeof(struct raw_icmp) + pkt->length;
    if (total_len % 2 != 0) total_len++;

    u8 *mem = malloc(total_len);
    assert(mem);
    clearbuf(mem, total_len);

    struct raw_icmp *header = (struct raw_icmp *)mem;
    header->type = (pkt->kind == ICMP_ECHO) ? 8 : 0;
    header->code = 0;
    header->checksum = 0;
    header->identifier = pkt->identifier;
    header->sequence = pkt->sequence;

    memcpy(header->payload, pkt->payload, pkt->length);
    header->checksum = compute_checksum(mem, total_len);

    if (out_len) *out_len = total_len;
    return mem;
}

icmp_packet *parse_icmp(const u8 *data, u16 len) {
    if (!data || len < sizeof(struct raw_icmp)) return NULL;

    const struct raw_icmp *header = (const struct raw_icmp *)data;
    u16 payload_len = len - sizeof(struct raw_icmp);

    icmp_packet *pkt = malloc(sizeof(icmp_packet));
    assert(pkt);
    pkt->payload = malloc(payload_len);
    assert(pkt->payload);

    pkt->length = payload_len;
    memcpy(pkt->payload, header->payload, payload_len);

    pkt->kind = (header->type == 8) ? ICMP_ECHO : ICMP_REPLY;
    pkt->identifier = header->identifier;
    pkt->sequence = header->sequence;

    return pkt;
}

void display_icmp(icmp_packet *pkt) {
    if (!pkt) return;

    printf("Kind:       %s\n", (pkt->kind == ICMP_ECHO) ? "Echo" : "Echo Reply");
    printf("Identifier: %d\n", pkt->identifier);
    printf("Sequence:   %d\n", pkt->sequence);
    printf("Length:     %d\n", pkt->length);
    printf("Payload:    ");
    dumphex(pkt->payload, pkt->length, 1);
}

int main() {
    const char *msg = "Hello!";
    icmp_packet *pkt = create_icmp(ICMP_ECHO, (const u8 *)msg, strlen(msg), 42, 1);

    printf("--- Original Packet ---\n");
    display_icmp(pkt);

    u16 raw_len;
    u8 *raw = serialize_icmp(pkt, &raw_len);
    printf("--- Serialized Bytes ---\n");
    dumphex(raw, raw_len, 1);

    icmp_packet *parsed = parse_icmp(raw, raw_len);
    printf("--- Parsed Back ---\n");
    display_icmp(parsed);

    // Cleanup
    free(pkt->payload);
    free(pkt);
    free(raw);
    free(parsed->payload);
    free(parsed);

    return 0;
}