#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>

#define BITMASK(n) ((1ULL << (n)) - 1ULL)
#define BITSHIFT(val, bitn) ((val) >> (bitn))
#define CHECK_BIT(val, bitn) (((val) >> (bitn)) & 1)
#define MASK_N_BITS(val, bitn, n) (((val) >> (bitn - n)) & ((1ULL << (n)) - 1ULL))

#define CANREQ_BIT_IDX 31
#define CANID_BIT_IDX 29
#define PRIO_BIT_IDX 29
#define PRIO_BITMASK_LEN 3
#define PGN_BIT_IDX 26
#define PGN_BITMASK_LEN 18
#define SA_BIT_IDX 8
#define SA_BITMASK_LEN 8

typedef struct __attribute__((__packed__))
{
	unsigned char prio : 3;
	u_int32_t pgn : 18;
	unsigned char src_addr;
} CAN_J1939_29bit_addr_t;

static void parseCanID(CAN_J1939_29bit_addr_t* can_id_bits, u_int32_t can_id)
{
	can_id_bits->prio = MASK_N_BITS(can_id, PRIO_BIT_IDX, PRIO_BITMASK_LEN);
	printf("PRIO = %02X\n", can_id_bits->prio);
	can_id_bits->pgn = MASK_N_BITS(can_id, PGN_BIT_IDX, PGN_BITMASK_LEN);
	printf("PGN = %06X\n", can_id_bits->pgn);
	can_id_bits->src_addr = MASK_N_BITS(can_id, SA_BIT_IDX, SA_BITMASK_LEN);
	printf("SA = %02X\n", can_id_bits->src_addr);
}

int main()
{
	u_int32_t val = 0x18ff0f55;
	CAN_J1939_29bit_addr_t j = { 0 };
	parseCanID(&j, val);

	u_int32_t pgnval = ntohl(j.pgn);
	printf("PGN0 = %02X\n", j.pgn & 0xFF);
	printf("PGN1 = %02X\n", (j.pgn >> 8) & 0xFF);
	printf("PGN2 = %02X\n", (j.pgn >> 16) & 0xFF);

	printf("PGN0 = %02X\n", pgnval & 0xFF);
	printf("PGN1 = %02X\n", (pgnval >> 8) & 0xFF);
	printf("PGN2 = %02X\n", (pgnval >> 16) & 0xFF);
}