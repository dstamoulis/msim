/*
 * memory.h
 * Memory model for MIPS. Implemented as 
 * big array, byte addressable. Can be either Big-Endian
 * or Little-Endian
 */

#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <stdint.h>
#include <cstring>


typedef enum {
	BIG_END = 0, 
	LITTLE_END = 1
} endian;


class Memory {

//Memory Interface

public:

	Memory() : byteOrder( BIG_END ){};
	~Memory() {};
	
	/*
 	 * Functions for loading and setting memory areas.
	 * Need to check for word alignment and bad address.
	 */
	int32_t loadWord( uint32_t addr );
	void storeWord( uint32_t addr, int32_t val );
	int16_t loadHalfWord( uint32_t addr );
	void storeHalfWord( uint32_t addr, int16_t val );
	int8_t loadByte( uint32_t addr );
	void storeByte( uint32_t addr, int8_t val );

protected:
	endian byteOrder;

};

class simpleMemory : Memory {

public:
	simpleMemory( uint32_t size );
	simpleMemory( uint32_t size, endian order );
	~simpleMemory();
	int32_t loadWord( uint32_t addr );
	void storeWord( uint32_t addr, int32_t val );
	int16_t loadHalfWord( uint32_t addr );
	void storeHalfWord( uint32_t addr, int16_t val );
	int8_t loadByte( uint32_t addr );
	void storeByte( uint32_t addr, int8_t val );

private:
	uint8_t *mem;
	uint32_t mem_size;
	bool big_endian() { return byteOrder == BIG_END; }
	bool little_endian() { return byteOrder == LITTLE_END; }

};


#endif /* __MEMORY_H__ */
