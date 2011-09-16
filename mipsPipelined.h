/*
 * mipsPipelined.h 
 * implementation of mips with pipeline
 */
#ifndef __MIPS_PIPELINED_H__
#define __MIPS_PIPELINED_H__

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
	mipsPipelined() : simpleProcessor() {
		
		innerRegs = new intermediateRegisters();
		cmd = new uint32_t[STAGES];
		dependence = 0;

		for( int i=0; i<STAGES; ++i ) {
			cmd[i] = 0;
			valid[i] = false;
		}

	}	

	mipsPipelined( simpleMemory *mem, Register_File *reg, uint32_t startAddress, uint32_t endAddress ) : simpleProcessor( mem,reg,startAddress,endAddress ) {

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
	void executeJAR();
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
	void memoryJR();
	void memoryJALR();
	//TODO: add declarations for JTYPE
	//TODO: handle case of BGEZ, BGEZAL, BLTZAL, BLTZ
	void memoryBEQ();
	void memoryBNE();
	void memoryBLEZ();
	void memoryBGEZ();
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

};

#endif /* __MIPS_PIPELINED_H__ */
