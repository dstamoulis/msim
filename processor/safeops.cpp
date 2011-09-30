#include <iostream>
#include <inttypes.h>
#include "safeops.h"

using namespace std;

bool overflow;

int32_t subtract(int32_t first, int32_t second)
{
	return add(first, -second);
}

/*
 * This function adds two numbers. We raise overflow in two cases, if the
 * operands are both positive and the result is negative, or if the operands
 * are both negative and the result is positive.
 */
int32_t add(int32_t first, int32_t second)
{
	int32_t ret;
	overflow = false;
	ret = first + second;

	if ((first > 0 && second > 0 && ret < 0) || (first < 0 && second < 0 && ret > 0))
		overflow = true;

	return ret;
}

/*
 * This function multiplies two numbers. To catch the overflow, we use the x86
 * overflow flag through inline assembly.
 */
int32_t multiply(int32_t first, int32_t second)
{
	int32_t ret;
	overflow = false;
	ret = first * second;

	__asm__ ("jno NOOVERFLOW_MUL\n");
	
	overflow = true;

	__asm__ ("NOOVERFLOW_MUL:\n");

	return ret;
}

uint32_t multiplyUnsigned(uint32_t first, uint32_t second)
{
	uint32_t ret;
	overflow = false;
	ret = first * second;

	__asm__ ("jno NOOVERFLOW_MULU\n");
	
	overflow = true;

	__asm__ ("NOOVERFLOW_MULU:\n");

	return ret;
}
