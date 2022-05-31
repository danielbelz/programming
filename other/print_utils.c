#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

// Prints the give number as binary without padding
void print_u32_binary(unsigned int number)
{
    char bitarray[33] = {0};
	for (int i = 0; i < 32; i++) {
		bitarray[i] = (((number>>(31-i)) & 1) ? '1' : '0');
	}
    printf("%s\n", bitarray);
}

// Prints the given 8 bit input as binary with padding
void print_u8_binary(unsigned char number)
{
    char bitarray[9] = {0};
	for (int i = 0; i < 8; i++) {
		bitarray[i] = (((number>>(7-i)) & 1) ? '1' : '0');
	}
    printf("%s\n", bitarray);
}

// Prints data as hexdump
void print_hexdump(uint8_t* data, uint16_t dataLen)
{
        if (data && dataLen > 0) {
            char line[64] = {0};
            int pos = 0;
            for (int i = 0; i < dataLen; i++) {
                pos += snprintf(&line[pos], 64, "%02X ", *data);
                if ((i % 16) == 0 && i > 0) {
        		    printf("%s\n", line);
                    pos = 0;
                }
                data++;
            }
			if (pos != 0) {
				printf("%s\n", line);
			}
        }
		printf("\n");
}
