/* 
 * mipsPipelined.cpp
 * implementation of a mips processor.
 * implements the classic five-stage pipeline
 */
#include <iostream>
#include <iomanip>
#include <sstream>
#include "mipsPipelined.h"
#include "memory.h"
#include "register_file.h"

using namespace std;

#define IF  0
#define ID  1
#define EX  2
#define MEM 3
#define WB  4

#define EMPTY_PIPELINE(valid) !( valid[IF] || valid[ID] || valid[EX] || valid[MEM] || valid[WB] )

void mipsPipelined::step()
{
	//print numbers as hexademical with 0x prefix
	stringstream ex;
	ex.setf( ios::hex, ios::basefield );
	ex.setf( ios::showbase );

	if( pc < startAddr ) {
		ex << "Error: pc out of range" << pc << endl;
		throw ex.str();
	}

	if( EMPTY_PIPELINE( valid ) && pc > endAddr ) {
		ex << "Error: empty pipeline" << endl;
		throw ex.str();
	}

	//execute each stage of the pipeline	
	writeback();
	memory();
	execute();
	decode();
	fetch();

	for (int i = WB; i < dependence; --i) { 
		cmd[i]   = cmd[i-1];
		valid[i] = valid[i-1];
	}

	valid[EX] = !dependence;  //if dependance exists EX stage is always invalid
			                    //that is, instruction located at ID stage does not progress
}


void mipsPipelined::fetch(){


	if ( pc > endAddr ) // TODO : Add J-type case later...
		valid[IF] = false;

	//set IF_ID intermediate register fields
	innerRegs->IFID_setPC( mem->loadWord( pc ) );
	innerRegs->IFID_setNextPC( mem->loadWord( pc + 4 ) );

	pc += 4;

}


void mipsPipelined::decode(){

	uint32_t temp = innerRegs->IFID_getPC();
	
	//set fields of IDEX intermediate regiser fields
	innerRegs->IDEX_setRT( reg->getReg(RT(temp)) );
	innerRegs->IDEX_setRS( reg->getReg(RS(temp)) );
	innerRegs->IDEX_setNextPC( innerRegs->IFID_getNextPC() );
	innerRegs->IDEX_setImmed( signExtend( (uint16_t) IMMED(temp)) );
	innerRegs->IDEX_setDestRegs( RT(temp), RD(temp) );
	//TODO: Add functionallity for J-type cases.

}

void mipsPipelined::execute(){

}
void mipsPipelined::memory(){}
void mipsPipelined::writeback(){}

