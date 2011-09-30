/*
 * processor.h
 * Defines the interface of a processor and
 * the basic internals of a MIPS processor.
 * Also includes declarations for a simple 
 * implementation of MIPS architecture.
 */
#include <stdint.h>
#include "../memory/memory.h"
#include "register_file.h"
#include "mipsISA.h"

#ifndef __PROCESSOR_H__
#define __PROCESSOR_H__

#define MEM_SIZE	0x7ffffffd
#define DATA_LOW	0x10000000
#define TEXT_LOW	0x00400000


/*
 * Interface of a simple processor.
 */
class processor {

public:
	virtual void reset() {};
	virtual void run() {};
	virtual void step() {};
	virtual void setTextArea( uint32_t startAddr, uint32_t endAddr ) { this->startAddr = startAddr; this->endAddr = endAddr; }	

	processor() {};
	processor( uint32_t startAddr, uint32_t endAddr ) : startAddr( startAddr ), endAddr( endAddr ) {};
	processor( simpleMemory *mem, Register_File *reg, uint32_t startAddr, uint32_t endAddr ) :  mem( mem ), reg( reg ),  pc( startAddr ), startAddr( startAddr ), endAddr( endAddr ) {};



protected:
	
	//Memory
	simpleMemory *mem;

	//Registers
	Register_File *reg;
	
	//Program Counter
	uint32_t pc;	

	//Addresses defining text area
	uint32_t startAddr;
	uint32_t endAddr;
	
	int32_t signExtend( int16_t );
	int32_t signExtend( int8_t );

};

/*
 * A Basic MIPS processor.
 */
class simpleProcessor : processor {


public:

	virtual void reset();
	virtual void run();
	virtual void step();
	virtual void setTextArea( uint32_t startAddr, uint32_t endAddr );	

	simpleProcessor() 
	{
		 this->mem = new simpleMemory( 1UL << 22 );
		 this->reg = new Register_File();
		 this->startAddr = 0;
		 this->endAddr = 0;
	}

	simpleProcessor( uint32_t startAddr, uint32_t endAddr ) 
	{
		this->startAddr = startAddr;
		this->endAddr = endAddr;
	}

	simpleProcessor( simpleMemory *mem, Register_File *reg, uint32_t startAddr, uint32_t endAddr ) 
	{
		 this->mem = mem;
		 this->reg = reg;
		 this->startAddr = startAddr;
		 this->endAddr = endAddr;
		 this->pc = startAddr;
	}

//protected:
protected:
	/*
	 * Make these fields protected so that 
	 * any new implementation of MIPS can inherit 
 	 * from here.
	 */

	//Memory
	simpleMemory *mem;

	//Registers
	Register_File *reg;
	
	//Program Counter
	uint32_t pc;	

	//Addresses defining text area
	uint32_t startAddr;
	uint32_t endAddr;



	//decode an instruction
	uint32_t OP( uint32_t instruction ) const { return instruction >> 26; }
	uint32_t RS( uint32_t instruction ) const { return (instruction >> 21) & 0x1f; }
	uint32_t RT( uint32_t instruction ) const { return (instruction >> 16) & 0x1f; }
	uint32_t RD( uint32_t instruction ) const { return (instruction >> 11) & 0x1f; }
	uint32_t SHAMT( uint32_t instruction ) const { return (instruction >> 6) & 0x1f; }
	uint32_t FUNCT( uint32_t instruction ) const { return instruction & 0x3f; }
	int16_t IMMED( uint32_t instruction ) const { return (int16_t)(instruction & 0xffff); }
	uint32_t TARG( uint32_t instruction ) const { return instruction & 0x3ffffff; }


	//functions extending sign for halfwords and bytes	
	int32_t signExtend( int16_t );
	int32_t signExtend( int8_t );
	
	//exception codes
	typedef enum {
		Int = 0,
		AdEl = 4,
		AdEs = 5,
		IBE = 6,
		DBE = 7,
		Sys = 8,
		Bp = 9,
		RI = 10,
		CpU = 11,
		Ov = 12,
		Tr = 16,
		FPE = 15
	} exception;

	//exception control registers
	uint32_t cause;
	// this enum contains the bits of interest in cause register
	typedef enum {
		BD = 0x10000000, //Branch Delay
		IM = 0x0000ff00, //Interupt Mack
		EC = 0x0000007C, //Exception Code
	}exception_cause;

	uint32_t status;
	// similarly to exception_cause this enum contains the
 	// bits of interest in status register.
	typedef enum {
		PI = 0x0000ff00, //Pending Interrupt
		UM = 0x00000010, //User Mode
		EL = 0x00000002, //Exception Level
		IE = 0x00000001, //Interrupt Enable
	}exception_status;


private:
	bool executeCmd( uint32_t cmd );
	
};

#endif /* __PROCESSOR_H__ */
