/*
 * pipelineRegisters.h 
 *
 * Defines the class which contains the 
 * registers located between the stages 
 * of the pipeline.
 */
#ifndef PIPELINE_REGS_H
#define PIPELINE_REGS_H

#include <stdint.h>
#include <stdlib.h>

#define STAGES_NR 5

class intermediateRegisters {

public:

	//constructor and destructor methods
	intermediateRegisters() 
	{
		regs = new uint32_t* [STAGES_NR-1];
		regs_size = new uint8_t [STAGES_NR-1];
		regs[IF_ID] = new uint32_t[2];
		regs_size[IF_ID] = 2;
		regs[ID_EX] = new uint32_t[8]; //TODO: probably must be changed when we take care of jumps.
		regs_size[ID_EX] = 8;
		regs[EX_MEM] = new uint32_t[5]; //TODO: check if we can merge some of the registers and allocate less memory.
		regs_size[EX_MEM] = 5;
		regs[MEM_WB] = new uint32_t[4]; //TODO: same here.
		regs_size[MEM_WB] = 4;
		
	}

	~intermediateRegisters()
	{
		for( int i=IF_ID; i<STAGES_NR; ++i )
			delete[] regs[i];
	
		delete[] regs;
		delete[] regs_size;	
	}

	/*
	 * defining access methods so that
	 * the user doesn't have to access 
	 * immediately the array.
	 */

	//accessors for IF_ID
	void IFID_setPC( uint32_t pc ) { regs[IF_ID][0] = pc; }
	uint32_t IFID_getPC() { return regs[IF_ID][0]; }

	void IFID_setNextPC( uint32_t nextpc ) { regs[IF_ID][1] = nextpc; }
	uint32_t IFID_getNextPC() { return regs[IF_ID][1]; }

	//accessors for ID_EX
	void IDEX_setRT( uint32_t rt ) { regs[ID_EX][0] = rt; }
	uint32_t IDEX_getRT() { return regs[ID_EX][0]; }

	void IDEX_setRS( uint32_t rs ) { regs[ID_EX][1] = rs; }
	uint32_t IDEX_getRS() { return regs[ID_EX][1]; } 

	void IDEX_setImmed( uint32_t imm ) { regs[ID_EX][2] = imm; }
	uint32_t IDEX_getImmed() { return regs[ID_EX][2]; }

	void IDEX_setNextPC( uint32_t nextpc ) { regs[ID_EX][3] = nextpc; }
	uint32_t IDEX_setNextPC() { return regs[ID_EX][3]; }

	void IDEX_setDestRegs( uint32_t rd, uint32_t rt )
	{
		regs[ID_EX][4] = ( rd  << 8 ) | rt;
	}
	uint32_t IDEX_getDestRegs() { return regs[ID_EX][4]; }
	uint32_t IDEX_getDestRegRD() { return (regs[ID_EX][4] >> 8 ); }
	uint32_t IDEX_getDestRegRT() { return (regs[ID_EX][4] & 0xff ); }

	void IDEX_setShamt( uint32_t shamt ) { regs[ID_EX][5] = shamt; }
	uint32_t IDEX_getShamt() { return regs[ID_EX][5]; }

	void IDEX_setLO( uint32_t lo ) {regs[ID_EX][6] = lo; }
	uint32_t IDEX_getLO() { return regs[ID_EX][6]; }

	void IDEX_setHI( uint32_t hi ) {regs[ID_EX][7] = hi; }
	uint32_t IDEX_getHI() { return regs[ID_EX][7]; }



	//accessors for EX_MEM
	void EXMEM_setBranchAddr( uint32_t addr ) { regs[EX_MEM][0] = addr; }
	uint32_t EXMEM_getBranchAddr() { return regs[EX_MEM][0]; }

	void EXMEM_setAluRes( uint32_t res ) { regs[EX_MEM][1] = res; }
	uint32_t EXMEM_getAluRes() { return regs[EX_MEM][1]; } 

	void EXMEM_setAluRes2( uint32_t res2 ) { regs[EX_MEM][2] = res2; }
	uint32_t EXMEM_getAluRes2() { return regs[EX_MEM][2]; }

	void EXMEM_setStoreData( uint32_t data ) { regs[EX_MEM][3] = data; }
	uint32_t EXMEM_getStoreData() { return regs[EX_MEM][3]; }

	void EXMEM_setDestRegs( uint32_t dest ) { regs[EX_MEM][4] = dest; }
	uint32_t EXMEM_getDestRegs() { return regs[EX_MEM][4]; }


	//accessors for MEM_WB
	void MEMWB_setMem( uint32_t word ) { regs[MEM_WB][0] = word; }
	uint32_t MEMWB_getMem() { return regs[MEM_WB][0]; }

	void MEMWB_setAlu( uint32_t res ) { regs[MEM_WB][1] = res; }
	uint32_t MEMWB_getAlu() { return regs[MEM_WB][1]; }

	void MEMWB_setAlu2( uint32_t res ) { regs[MEM_WB][2] = res; }
	uint32_t MEMWB_getAlu2() { return regs[MEM_WB][2]; }	

	void MEMWB_setDestRegs( uint32_t dest ) { regs[MEM_WB][3] = dest; }
	uint32_t MEMWB_getDestRegs() { return regs[MEM_WB][3]; }

	uint32_t MEMWB_getDestRegRD() { return (regs[MEM_WB][3] >> 8 ); }
	uint32_t MEMWB_getDestRegRT() { return (regs[MEM_WB][3] & 0xff ); }


// autes sto stadio tou WB tha pairnoun ta 5bits pou deixnoun se ena kataxwrhth
// kai me basi auta tha kanw eggrafi newn timwn sto register file


//private:

	//defining names for registers
	enum {
		IF_ID = 0,
		ID_EX = 1,
		EX_MEM = 2,
		MEM_WB = 3
	};

	uint32_t **regs;
	uint8_t *regs_size;


};

#endif
