/*
 * pipelinedProcessor.h
 *
 * A MIPS processor implementing the classic 5-stage
 * pipeline.
 */

#ifndef _PIPELINED_MIPS_H
#define _PIPELINED_MIPS_H

#include "processor.h"

#define STAGES 5

class pipelinedMips : simpleProcessor {

public:

	virtual void reset();
	virtual void run();
	virtual void step();
	virtual void setTextArea( uint32_t startAddr, uint32_t endAddr );	

	pipelinedMips() 
	{
		this->mem = NULL;
		this->reg = NULL;
		this->startAddr = 0;
		this->endAddr = 0;

		for( int i=0; i<STAGES; ++i ) {
			valid[i] = false;
			opcode[i] = funct[i] = 0;
		}			
	}

	pipelinedMips( uint32_t startAddr, uint32_t endAddr ) 
	{
		this->startAddr = startAddr;
		this->endAddr = endAddr;
		
		for( int i=0; i<STAGES; ++i ) {
			valid[i] = false;
			opcode[i] = funct[i] = 0;
		}
	}

	pipelinedMips( Memory *mem, Register_File *reg, uint32_t startAddr, uint32_t endAddr ) 
	{
		this->mem = mem;
		this->reg = reg;
		this->startAddr = startAddr;
		this->endAddr = endAddr;
		this->pc = startAddr;

		for( int i=0; i<STAGES; ++i ) {
			valid[i] = false;
			opcode[i] = funct[i] = 0;
		}

	}	

private:
	bool valid[STAGES];
	uint8_t opcode[STAGES];
	uint8_t funct[STAGES];
	

	void forward();

	void fetch();
	void decode();
	void execute();
	void mem();
	void writeback();

	void invalidate( uint8_t );
};


#endif
