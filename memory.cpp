/*
 * memory.cpp
 * Implementation of a simple memory.
 * Just handles setting and loading of locations 
 * in memory. Takes care of requests for unaligned 
 * memory addresses, and out of bounds addressing.
 * requesting areas of memory MIPS considers to be 
 * protected are leaved to the processor to be checked. 
 */
#include "memory.h"

using namespace std;


/*
 * Constructors. It allocates memory for the array 
 * representing the processor's memory. By default
 * byteOrder is set to BIG_END
 */
simpleMemory::simpleMemory( uint32_t size ) : mem_size( size )
{
	if( size == 0 ) 
		throw "WTF??? zero memory??";
	
	mem = new uint8_t[ size ];
}


/*
 * This constructor sets byteOrder too.
 */
simpleMemory::simpleMemory( uint32_t size, endian order )
{
	if( size == 0 )
		throw "WTF??? zero memory??";

	mem = new uint8_t[ size ];
	byteOrder = order;
}


/*
 * Free the area allocated for mem.
 */
simpleMemory::~simpleMemory() 
{
	delete [] mem;
}

/*
 * loads/stores of memory areas. Checks for proper alignment 
 * of memory areas requested and addresses in bound.
 */
int32_t simpleMemory::loadWord( uint32_t addr )
{
	if( addr % 4 != 0 )
		throw "Memory addresses should be word aligned";

	if( addr >= mem_size )
		throw "Address requested is out of bounds";

	int32_t ret;
	
	if( big_endian() )
		memcpy( &ret, &mem[addr], 4 );
	else {
		//little-endian
		ret = mem[addr+3];
		for( int i=2; i>=0; --i) {
			ret = ret << 8;
			ret = ret | mem[addr+i];
		}
	}

	return ret;
}

void simpleMemory::storeWord( uint32_t addr, int32_t val )
{
	if( addr % 4 != 0 )
		throw "Memory addresses should be word aligned";

	if( addr >= mem_size )
		throw "Address requested is out of bounds";
	if( big_endian() )
		memcpy( &mem[addr], &val, 4 );
	else {
		//little-endian
		for( int i=0; i<4; ++i ) {
			mem[addr+i] = val & 0x000000ff;
			val = val >> 8;
		}
		
	}
}

int16_t simpleMemory::loadHalfWord( uint32_t addr )
{

	/*
	 * address requested must be either the first or the 
 	 * last two bytes of a word.
	 */
	if( addr % 2 != 0 )
		throw "Memory addresses should be word aligned";

	if( addr >= mem_size )
		throw "Address requested is out of bounds";

	int16_t ret;
	if( big_endian() )
		memcpy( &ret, &mem[addr], 2 );
	else {
		//little endian
		ret = 0;
		ret = ret | mem[addr+1];
		ret = ret << 8;
		ret = ret | mem[addr+2];
		ret = ret << 8;
	}
	return ret;
}

void simpleMemory::storeHalfWord( uint32_t addr, int16_t val )
{
	if( addr % 4 != 0 )
		throw "Memory addresses should be word aligned";

	if( addr >= mem_size )
		throw "Address requested is out of bounds";
	if( big_endian() )	
		memcpy( &mem[addr], &val, 2 );
	else {
		mem[addr] = val & 0x000000ff;
		val = val >> 8;
		mem[addr+1] = val & 0x000000ff;
	}
}

/*
 * Functions concerning byte don't have to check
 * for aligment restrictions.
 */
int8_t simpleMemory::loadByte( uint32_t addr )
{
	if( addr >= mem_size )
		throw "Address requested is out of bounds";

	return mem[addr];
}

void simpleMemory::storeByte( uint32_t addr, int8_t val )
{
	if( addr >= mem_size )
		throw "Address requested is out of bounds";

	mem[addr] = val;
}


