

#include <iostream>
#include <iomanip>
#include <sstream>

#include "mipsPipelined.h"



void mipsPipelined::step()
{
	stringstream ex;
			
	ex.setf( ios::hex, ios::basefield );
	ex.setf( ios::showbase );

	if( pc < startAddr )

	{

		ex << "Error: pc out of range" << pc << endl;
	
		throw ex.str();

	}

	if(  ( !(valid[IF] || valid[ID] || valid[EX] || valid[MEM] || valid[WB]) ) && ( pc > endAddr )   ) {

		

		
		ex << "Error: empty pipeline" << endl;
		

		throw ex.str();
	}
	

	
	

	writeback();
	mem();
	execute();
	decode();
	fetch();

	for (int i=WB; i < dep; --i)
	{ 
		cmd[i]   = cmd[i-1];
		valid[i] = valid[i-1];
	}

	valid[EXE] = ~dep;   // Take one's complement


}


void mipsPipelined::fetch(){


	if ( pc > endAddr ) // TODO : Add J-type case later...
		valid[IF] = false;


	IFID_setPC( loadWord( pc ) );
	IFID_setNextPC( loadWord( pc + 4 ) );


	pc += 4;

}


void mipsPipelined::decode(){

	uint32_t temp = IFID_getPC();
	

	IDEX_setRT( getReg(RT(temp)) );
	IDEX_setRS( getReg(RS(temp)) );

	IDEX_setNextPC( IFID_getNextPC() );
	

	IDEX_setImmed( signExtend( (uint16_t) IMMED(temp)) );

	IDEX_setDestRegs( RT(temp), RD(temp) );

	//TODO: Add functionallity for J-type cases.

}

void mipsPipelined::execute(){




}
void mipsPipelined::mem(){}
void mipsPipelined::writeback(){}

