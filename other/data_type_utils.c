#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

// converts a 8 byte array to uint64
uint64_t bytearray_to_uint64(uint8_t *payload)
{
	uint64_t value = 0;
	for (int i=0; i<8; i++) {
		value += (uint64_t)(payload[7-i]) << (8*i);
	}
	return value;
}

// Returns a string containing the given input formatted as binary
char *data_to_bitarray(uint8_t *payload, int payloadLen)
{
    int bitcount = 8*payloadLen;
    char *bitarray = calloc(1, bitcount+1);
    for (int x = 0; x < payloadLen; x++) {
        for (int i = 0; i < 8; i++) {
            bitarray[(x*8) + i] = (((payload[x]>>(7-i)) & 1) ? '1' : '0');
        }
    }
    return bitarray;
}
