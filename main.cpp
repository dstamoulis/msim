#include <stdio.h>
#include <stdlib.h>
#include "mipsPipelined.h"
#include "memory.h"
#include "register_file.h"


int main( int argc, char **argv )
{

	mipsPipelined *proc;
	simpleMemory *mem;
	Register_File *reg;

	mem = new simpleMemory( 4096 );
	reg = new Register_File();
	
	mem->storeWord( 24, 0x2003000a );
	mem->storeWord( 28, 0xac030004 );
//	mem->storeWord( 32, 0x8c040004 );
//	mem->storeWord( 36, 0x20030004 );
//	mem->storeWord( 40, 0x00831823 );
//	mem->storeWord( 44, 0xac030008 );	


	proc = new mipsPipelined( mem, reg, 24, 44 );
	proc->showMemory( 24, 44 ); 
	proc->printRegisters();

	int c = getchar();
	while( c != EOF ) {
		proc->step();
		proc->printRegisters();
		c = getchar();
	}

	proc->showMemory( 0, 24 ); 

	return 0;
}
