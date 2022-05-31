/**
 * Bitwise operations in C
 * gcc bit_ops.c -o bitops
 */

#include <stdio.h>

typedef unsigned char uchar;
#define BITMASK(n) ((1ULL << (n)) - 1ULL)
#define BITSHIFT(val,bitn) ((val)>>(bitn))
#define CHECK_BIT(val,bitn)  (((val)>>(bitn)) & 1)
#define MASK_N_BITS(val,bitn,n)  (((val)>>(bitn-n)) & ((1ULL << (n)) - 1ULL))

void toggle_bit(unsigned char number, int bit)
{
	if (bit > 7 || bit < 0)
		printf("ERROR\n");
	else
		printf("%d -> %d\n", number, number ^= 1 << bit);
}

void set_bit(unsigned char number, int bit)
{
	if (bit > 7 || bit < 0)
		printf("ERROR\n");
	else
		printf("%d -> %d\n", number, number |= 1 << bit);
}

void clear_bit(unsigned char number, int bit)
{
	if (bit > 7 || bit < 0)
		printf("ERROR\n");
	else
		printf("%d -> %d\n", number, number &= ~(1 << bit));
}

void count_bits(unsigned char number)
{
	int cnt = 0;

	while (number != 0) {
		cnt += number & 0x01;
		number >>= 1;
	}
	printf("%d -> %d\n", number, cnt);
}

uchar reverse_byte(uchar byte)
{
	uchar new_byte = 0;
	int i = 0;
	for (i = 0; i < 7; i++) {
		new_byte += (byte & 0x01);
		new_byte <<= 1;
		byte >>= 1;
	}
	printf("%d\n", new_byte);
	return (new_byte);
}

int main()
{
	unsigned char num = 0x33;
	count_bits(num);
	set_bit(num, 6);
	clear_bit(num, 6);
	toggle_bit(num, 6);
	reverse_byte(num);
	return 0;
}
