/*
 * 
 *
 */
#ifndef _MIPS_PIPELINED_H
#define _MIPS_PIPELINED_H

#include "pipelineRegisters.h"
#include "processor.h"

#define STAGES 5

class mipsPipelined : simpleProcessor {

public:
	virtual void reset();
	virtual void run();
	virtual void step();
	virtual void setTextArea( uint32_t, uint32_t );


	//constructors and destructor
	mipsPipelined() {
		this->mem = new simpleMemory( 1UL << 22 );
		this->reg = new Register_File();
		this->startAddr = 0;
		this->endAddr = 0;
		
		innerRegs = new intermediateRegisters();
		cmd = new uint32_t[STAGES];
		dependence = 0;

		for( int i=0; i<STAGES; ++i ) {
			cmd[i] = 0;
			valid[i] = false;
		}

	}	

	mipsPipelined( simpleMemory *mem, Register_File *reg, uint32_t startAddress, uint32_t endAddress ) {
		this->mem = mem;
		this->reg = reg;
		this->startAddr = startAddress;
		this->endAddr = endAddress;
	
		innerRegs = new intermediateRegisters();
		cmd = new uint32_t[STAGES];
		dependence = 0;

		for( int i=0; i<STAGES; ++i ) {
			cmd[i] = 0;
			valid[i] = false;
		}
	}

	~mipsPipelined() 
	{
		delete[] cmd;
		delete[] innerRegs;
	}

private:

	intermediateRegisters *innerRegs;
	uint32_t *cmd;
	bool *valid;
	int dependence;

	void fetch();
	void decode();
	void execute();
	void memory();
	void writeback();	
	void forward();
};



#endif /* _MIPS_PIPELINED_H */
