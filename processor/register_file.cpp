/*
 *	register_file.cpp
 *	Implementation of the RegisterFile.
 * Defines functions for load and store of
 * a register value.
 */

#include <stdio.h>
#include "register_file.h"

/*
 * initialize all registers in register file
 */
RegisterFile::RegisterFile() : HI(0), LO(0), EX(0), BRK( 0x1000000 ) 
{
	memset( registers, 0, REG_NR*4 );
	registers[29] = STACK_MAX;
	registers[28] = GLOBAL_INIT;
}

int32_t RegisterFile::getReg( unsigned int reg ) const
{
	if( reg > REG_NR )
		throw "Not valid register";

	return registers[reg];
}

void RegisterFile::setReg( unsigned int reg, int32_t val )
{
	if( reg > REG_NR )
		throw "Not valid register";

	if( reg == 0 )
		return;

	if( reg == 29 && val > STACK_MAX )
		throw "Not valid address for stack pointer";

	registers[reg] = val;
}

void RegisterFile::reset()
{
	memset( registers, 0, REG_NR*4 );
	registers[29] = STACK_MAX;
	registers[28] = GLOBAL_INIT;
}

void RegisterFile::printRegisters()
{
	printf( "-------------REGISTER FILE-------------\n" );
	
	for( int i=0; i<REG_NR; ++i ) 
		printf( "%d:\t%x\n", i, registers[i] );
	
	printf( "HI:\t%x\n", HI );
	printf( "LO:\t%x\n", LO );

}
