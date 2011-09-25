/*
 *	register_file.cpp
 *	Implementation of the Register_File.
 * Defines functions for load and store of
 * a register value.
 */

#include <stdio.h>
#include "register_file.h"

/*
 * initialize all registers in register file
 */
Register_File::Register_File() : HI(0), LO(0), EX(0), BRK( 0x1000000 ) 
{
	memset( registers, 0, REG_NR*4 );
	registers[29] = STACK_MAX;
	registers[28] = GLOBAL_INIT;
}

int32_t Register_File::getReg( unsigned int reg ) const
{
	if( reg > REG_NR )
		throw "Not valid register";

	return reg;
}

void Register_File::setReg( unsigned int reg, int32_t val )
{
	if( reg > REG_NR )
		throw "Not valid register";

	if( reg == 0 )
		return;

	if( reg == 29 && val > STACK_MAX )
		throw "Not valid address for stack pointer";

	registers[reg] = val;
}

void Register_File::reset()
{
	memset( registers, 0, REG_NR*4 );
	registers[29] = STACK_MAX;
	registers[28] = GLOBAL_INIT;
}

void Register_File::printRegisters()
{
	printf( "-------------REGISTER FILE-------------\n" );
	
	for( int i=0; i<REG_NR; ++i ) 
		printf( "%d:\t%x\n", i, registers[i] );
	
	printf( "HI:\t%x\n", HI );
	printf( "LO:\t%x\n", LO );

}
