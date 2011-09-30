/*
 * mipsPipelined.h 
 * implementation of mips with pipeline
 */
#ifndef __MIPS_PIPELINED_H__
#define __MIPS_PIPELINED_H__

#include "pipelineRegisters.h"
#include "processor.h"
#include <stdio.h>

#define STAGES 5
#define IF  0
#define ID  1
#define EX  2
#define MEM 3
#define WB  4


class mipsPipelined : simpleProcessor {

public:
	void reset(){};
	void run();
	void step();
	void setTextArea( uint32_t, uint32_t ){};


	//constructors and destructor
	mipsPipelined() : simpleProcessor() {
		
		innerRegs = new intermediateRegisters();
		cmd = new uint32_t[STAGES];
		valid = new bool[STAGES];
		srcRegs = new uint32_t*[STAGES];
		dstRegs = new uint32_t*[STAGES];
		dependence = false;

		for( int i=0; i<STAGES; ++i ) {
			srcRegs[i] = new uint32_t[2];
			dstRegs[i] = new uint32_t[2];
			cmd[i] = 0;
			valid[i] = false;
		}

	}	

	mipsPipelined( simpleMemory *mem, RegisterFile *reg, uint32_t startAddress, uint32_t endAddress ) : simpleProcessor( mem,reg,startAddress,endAddress ) {

		innerRegs = new intermediateRegisters();
		cmd = new uint32_t[STAGES];
		valid = new bool[STAGES];
		srcRegs = new uint32_t*[STAGES];
		dstRegs = new uint32_t*[STAGES];
		dependence = false;

		for( int i=0; i<STAGES; ++i ) {
			srcRegs[i] = new uint32_t[2];
			dstRegs[i] = new uint32_t[2];
			cmd[i] = 0;
			valid[i] = false;
		}
	}

	~mipsPipelined() 
	{
		delete[] cmd;
		delete[] innerRegs;
	}

	void showMemory( uint32_t start, uint32_t end ) {
		mem->showMemory( start, end );
	}

	void printRegisters() {
		reg->printRegisters();
		printf( "PC:\t%d\n", pc );
		printf( "IF: %x,\tvalid: %s,\tsrc[0]: %d,\tsrc[1]: %d,\tdst[0]: %d,\tdst[1]: %d\n",
					valid[IF] ? cmd[IF] : 0x00000000, ( valid[IF] )? "true" : "false", srcRegs[IF][0], srcRegs[IF][1], dstRegs[IF][0], dstRegs[IF][1] );
		printf( "ID:  %x,\tvalid: %s,\tsrc[0]: %d,\tsrc[1]: %d,\tdst[0]: %d,\tdst[1]: %d\n",
					valid[ID] ? cmd[ID] : 0x00000000, valid[ID] ? "true" : "false", srcRegs[ID][0], srcRegs[ID][1], dstRegs[ID][0], dstRegs[ID][1] );
		printf( "EX:  %x,\tvalid: %s,\tsrc[0]: %d,\tsrc[1]: %d,\tdst[0]: %d,\tdst[1]: %d\n",
					valid[EX] ? cmd[EX] : 0x00000000, valid[EX] ? "true" : "false", srcRegs[EX][0], srcRegs[EX][1], dstRegs[EX][0], dstRegs[EX][1] );
		printf( "MEM: %x,\tvalid: %s,\tsrc[0]: %d,\tsrc[1]: %d,\tdst[0]: %d,\tdst[1]: %d\n",
					valid[MEM] ? cmd[MEM] : 0x00000000, valid[MEM] ? "true" : "false", srcRegs[MEM][0], srcRegs[MEM][1], dstRegs[MEM][0], dstRegs[MEM][1] );
		printf( "WB:  %x,\tvalid: %s,\tsrc[0]: %d,\tsrc[1]: %d,\tdst[0]: %d,\tdst[1]: %d\n",
					valid[WB] ? cmd[WB] : 0x00000000, valid[WB] ? "true" : "false", srcRegs[WB][0], srcRegs[WB][1], dstRegs[WB][0], dstRegs[WB][1] );



	
}

private:

	intermediateRegisters *innerRegs;
	uint32_t *cmd;
	bool *valid;
	uint32_t **srcRegs;
	uint32_t **dstRegs;
	bool dependence;
	int ll;


	void fetch();
	void decode();
	void execute();
	void memory();
	void writeback(); 

	bool checkDependence();

	/*********************
	 * execute functions *
	 *********************/
	void executeSLL();
	void executeSRL();
	void executeSRA();
	void executeSLLV();
	void executeSRLV();
	void executeSRAV();
	void executeJR();
	void executeJALR();
	void executeBREAK();
	void executeMFHI();
	void executeMTHI();
	void executeMFLO();
	void executeMTLO();
	void executeMULT();
	void executeMULTU();
	void executeDIV();
	void executeDIVU();
	void executeADD();
	void executeADDU();
	void executeSUB();
	void executeSUBU();
	void executeAND();
	void executeOR();
	void executeXOR();
	void executeNOR();
	void executeSLT();
	void executeSLTU();
	void executeMADD();
	void executeMADDU();
	void executeMUL();
	void executeMSUB();
	void executeMSUBU();
	void executeCLZ();
	void executeCLO();
	void executeMOVZ();
	void executeMOVN();
	void executeJ();
	void executeJAL();
	void executeBEQ();
	void executeBNE();
	void executeBLEZ();
	void executeBGEZ();
	void executeADDI();
	void executeADDIU();
	void executeSLTI();
	void executeSLTIU();
	void executeANDI();
	void executeORI();
	void executeXORI();
	void executeLUI();
	void executeLB();
	void executeLH();
	void executeLWL();
	void executeLW();
	void executeLBU();
	void executeLHU();
	void executeLWR();
	void executeSB();
	void executeSH();
	void executeSWL();
	void executeSW();
	void executeSWR();
	void executeLL();
	void executeSC();


	
	/********************
	 * memory functions *
	 ********************/

	void memorySLL();
	void memorySRL();
	void memorySRA();
	void memorySLLV();
	void memorySRLV();
	void memorySRAV();
	void memoryJR();
	void memoryJALR();
	void memoryBREAK();
	void memoryMFHI();
	void memoryMTHI();
	void memoryMFLO();
	void memoryMTLO();
	void memoryMULT();
	void memoryMULTU();
	void memoryDIV();
	void memoryDIVU();
	void memoryADD();
	void memoryADDU();
	void memorySUB();
	void memorySUBU();
	void memoryAND();
	void memoryOR();
	void memoryXOR();
	void memoryNOR();
	void memorySLT();
	void memorySLTU();
	void memoryMADD();
	void memoryMADDU();
	void memoryMUL();
	void memoryMSUB();
	void memoryMSUBU();
	void memoryCLZ();
	void memoryCLO();
	void memoryMOVZ();
	void memoryMOVN();
	void memoryJ();
	void memoryJAL();
	void memoryBEQ();
	void memoryBNE();
	void memoryBLEZ();
	void memoryBGEZ();
	void memoryADDI();
	void memoryADDIU();
	void memorySLTI();
	void memorySLTIU();
	void memoryANDI();
	void memoryORI();
	void memoryXORI();
	void memoryLUI();
	void memoryLB();
	void memoryLH();
	void memoryLWL();
	void memoryLW();
	void memoryLBU();
	void memoryLHU();
	void memoryLWR();
	void memorySB();
	void memorySH();
	void memorySWL();
	void memorySW();
	void memorySWR();
	void memoryLL();
	void memorySC();


	/************************
	 * write-back functions *
	 ************************/

	void writebackSLL();
	void writebackSRL();
	void writebackSRA();
	void writebackSLLV();
	void writebackSRLV();
	void writebackSRAV();
	void writebackMFHI();
	void writebackMTHI();
	void writebackMFLO();
	void writebackMTLO();
	void writebackMULT();
	void writebackMULTU();
	void writebackDIV();
	void writebackDIVU();
	void writebackADD();
	void writebackADDU();
	void writebackSUB();
	void writebackSUBU();
	void writebackAND();
	void writebackOR();
	void writebackXOR();
	void writebackNOR();
	void writebackSLT();
	void writebackSLTU();
	void writebackMADD();
	void writebackMADDU();
	void writebackMUL();
	void writebackMSUB();
	void writebackMSUBU();
	void writebackCLZ();
	void writebackCLO();
	void writebackMOVZ();
	void writebackMOVN();
	void writebackADDI();
	void writebackADDIU();
	void writebackSLTI();
	void writebackSLTIU();
	void writebackANDI();
	void writebackORI();
	void writebackXORI();
	void writebackLUI();
	void writebackLB();
	void writebackLH();
	void writebackLWL();
	void writebackLW();
	void writebackLBU();
	void writebackLHU();
	void writebackLWR();
	void writebackLL();		//???
	void writebackSC();
	void writebackSB();
	void writebackSH();
	void writebackSWL();
	void writebackSW();
	void writebackSWR();
/*

SOOOOOOOOS

den evala:

JR
JALR
BREAK
BEQ
BNE
BLEZ
BGEZ

den tis eixe kai o mpampis epeidi den kanoun tpt sto WB
alla rwta teammates an toulaxiston eksasfalizw orthotita

*/

};

#endif /* __MIPS_PIPELINED_H__ */
