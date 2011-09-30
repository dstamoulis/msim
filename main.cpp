#include "processor/processor.h"
#include "processor/mipsPipelined.h"
#include "processor/register_file.h"
#include "memory/memory.h"
#include <stdlib.h>
#include <stdio.h>
#include <iomanip>
#include <iostream>

using namespace std;

int main()
{
	mipsPipelined *proc;
	Register_File *regs = new Register_File();
	simpleMemory *mem = new simpleMemory( 4096 );

	mem->storeWord( 100, 0x2003000a );		// addi $3,$0,10 
	mem->storeWord( 104, 0xac030004 );		// sw $3,4($0)
	mem->storeWord( 108, 0x00630820 );		// add $1,$3,$3
	mem->storeWord( 112, 0xac010008 );		// sw $1,8($0)

	proc = new mipsPipelined( mem, regs, 100, 112 );

	cout << "Processor constructed..." << endl;



	try {
		while( true ) {
			proc->step();
			proc->printRegisters();
			if( getchar() == EOF )
				break;	
		}
	} catch ( char const *msg ) {
		cout << "Exception encountered! --> " << msg << endl;
	} catch ( ... ) {
		cout << "Default exception!! " << endl;	
	}


	return 0;
}

