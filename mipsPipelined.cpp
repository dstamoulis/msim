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
			                    //that is, opuction located at ID stage does not progress
}


void mipsPipelined::fetch(){


	if ( pc > endAddr ) // TODO : Add J-type case later...
		valid[IF] = false;

	//set IF_ID intermediate register fields
	innerRegs->IFID_setPC( mem->loadWord( pc ) );
	innerRegs->IFID_setNextPC( mem->loadWord( pc + 4 ) );

	pc += 4;

}


void mipsPipelined::decode() {

	uint32_t temp = innerRegs->IFID_getPC();
	
	//set fields of IDEX intermediate regiser fields
	innerRegs->IDEX_setRT( reg->getReg(RT(temp)) );
	innerRegs->IDEX_setRS( reg->getReg(RS(temp)) );
	innerRegs->IDEX_setNextPC( innerRegs->IFID_getNextPC() );
	innerRegs->IDEX_setImmed( signExtend( IMMED(temp)) );
	innerRegs->IDEX_setDestRegs( RT(temp), RD(temp) );
	//TODO: Add functionallity for J-type cases.

}

void mipsPipelined::execute() {
	uint32_t op = OP( cmd[EX] );
	if( op == RTYPE1 ) {

		//RTYPE1 opuctions
		switch( FUNCT( cmd[EX] ) ) {
			case( SLL ): executeSLL(); break;
			case( SRL ): executeSRL(); break;
			case( SRA ): executeSRA(); break;
			case( SLLV ): executeSLLV(); break;
			case( SRLV ): executeSRLV(); break;
			case( SRAV ): executeSRAV(); break;
			case( JR ): executeJR(); break;
			case( JALR ): executeJAR(); break;
			case( BREAK ): executeBREAK(); break;
			case( MFHI ): executeMFHI(); break;
			case( MTHI ): executeMTHI(); break;
			case( MFLO ): executeMFLO(); break;
			case( MTLO ): executeMTLO(); break;
			case( MULT ): executeMULT(); break;
			case( MULTU ): executeMULTU(); break;
			case( DIV ): executeDIV(); break;
			case( DIVU ): executeDIVU(); break;
			case( ADD ): executeADD(); break;
			case( ADDU ): executeADDU(); break;
			case( SUB ): executeSUB(); break;
			case( SUBU ): executeSUBU(); break;
			case( AND ): executeAND(); break;
			case( OR ): executeOR(); break;
			case( XOR ): executeXOR(); break;
			case( NOR ): executeNOR(); break;
			case( SLT ): executeSLT(); break;
			case( SLTU ): executeSLTU(); break;
			default:
				throw "Unhandled opuction";
		} 

	} else if ( op == RTYPE2 ) {
	
		//RTYPE2 opuctions
		switch( FUNCT( cmd[EX] ) ) {
			case( MADD ): executeMADD(); break;
			case( MADDU ): executeMADDU(); break;
			case( MUL ): executeMUL(); break;
			case( MSUB ): executeMSUB(); break;
			case( MSUBU ): executeMSUBU(); break;
			case( CLZ ): executeCLZ(); break;
			case( CLO ): executeCLO(); break;
			case( MOVZ ): executeMOVZ(); break;
			case( MOVN ): executeMOVN(); break;
			default:
				throw "Unhandled opuction";
		}

	} else if ( op == J ) { //the two JTYPE opuctions are following
		
	} else if ( op == JAL ) {


	} else { //ITYPE here
		
		switch( op ) {
			case( BGEZ ):
				//TODO: handle case of BGEZ, BGEZAL, BLTZAL, BLTZ
				break;
			case( BEQ ): executeBEQ(); break;
			case( BNE ): executeBNE(); break;
			case( BLEZ ): executeBLEZ(); break;
			case( BGTZ ): executeBGEZ(); break;
			case( ADDI ): executeADDI(); break;
			case( ADDIU ): executeADDIU(); break;
			case( SLTI ): executeSLTI(); break;
			case( SLTIU ): executeSLTIU(); break;
			case( ANDI ): executeANDI(); break;
			case( ORI ): executeORI(); break;
			case( XORI ): executeXORI(); break;
			case( LUI ): executeLUI(); break;
			case( LB ): executeLB(); break;
			case( LH ): executeLH(); break;
			case( LWL ): executeLWL(); break;
			case( LW ): executeLW(); break;
			case( LBU ): executeLBU(); break;
			case( LHU ): executeLHU(); break;
			case( LWR ): executeLWR(); break;
			case( SB ): executeSB(); break;
			case( SH ): executeSH(); break;
			case( SWL ): executeSWL(); break;
			case( SW ): executeSW(); break;
			case( SWR ): executeSWR(); break;
			case( LL ): executeLL(); break;
			case( SC ): executeSC(); break;
		} 	
	}
}


void mipsPipelined::memory()
{
	uint32_t op = OP( cmd[MEM] );
	if( op == RTYPE1 ) {
		switch( FUNCT( cmd[MEM] ) ) {		//only these two RTYPE instruction have work to do during mem stage
			case( JR ): memoryJR(); break;
			case( JALR ): memoryJALR(); break;
			default:
				break;
		}
			
	} else if( op == RTYPE2 ) 
		return;

	else if ( op == J ) { //the two JTYPE opuctions are following
		
	} else if ( op == JAL ) {


	} else { //ITYPE here
	
		switch( op ) {
			case( BGEZ ):
				//TODO: handle case of BGEZ, BGEZAL, BLTZAL, BLTZ
				break;
			case( BEQ ): memoryBEQ(); break;
			case( BNE ): memoryBNE(); break;
			case( BLEZ ): memoryBLEZ(); break;
			case( BGTZ ): memoryBGEZ(); break;
			case( LB ): memoryLB(); break;
			case( LH ): memoryLH(); break;
			case( LWL ): memoryLWL(); break;
			case( LW ): memoryLW(); break;
			case( LBU ): memoryLBU(); break;
			case( LHU ): memoryLHU(); break;
			case( LWR ): memoryLWR(); break;
			case( SB ): memorySB(); break;
			case( SH ): memorySH(); break;
			case( SWL ): memorySWL(); break;
			case( SW ): memorySW(); break;
			case( SWR ): memorySWR(); break;
			case( LL ): memoryLL(); break;
			case( SC ): memorySC(); break;
		} 	
	}
}
void mipsPipelined::writeback() 
{
	uint32_t op = OP( cmd[WB] );
	if( op == RTYPE1 ) {

		//RTYPE1 opuctions
		switch( FUNCT( cmd[WB] ) ) {
			case( SLL ): writebackSLL(); break;
			case( SRL ): writebackSRL(); break;
			case( SRA ): writebackSRA(); break;
			case( SLLV ): writebackSLLV(); break;
			case( SRLV ): writebackSRLV(); break;
			case( SRAV ): writebackSRAV(); break;
			//case( JR ): executeJR(); break;
			//case( JALR ): executeJAR(); break;
			//case( BREAK ): executeBREAK(); break;
			case( MFHI ): writebackMFHI(); break;
			case( MTHI ): writebackMTHI(); break;
			case( MFLO ): writebackMFLO(); break;
			case( MTLO ): writebackMTLO(); break;
			case( MULT ): writebackMULT(); break;
			case( MULTU ): writebackMULTU(); break;
			case( DIV ): writebackDIV(); break;
			case( DIVU ): writebackDIVU(); break;
			case( ADD ): writebackADD(); break;
			case( ADDU ): writebackADDU(); break;
			case( SUB ): writebackSUB(); break;
			case( SUBU ): writebackSUBU(); break;
			case( AND ): writebackAND(); break;
			case( OR ): writebackOR(); break;
			case( XOR ): writebackXOR(); break;
			case( NOR ): writebackNOR(); break;
			case( SLT ): writebackSLT(); break;
			case( SLTU ): writebackSLTU(); break;
			default:
				throw "Unhandled opuction";
		} 

	} else if ( op == RTYPE2 ) {
	
		//RTYPE2 opuctions
		switch( FUNCT( cmd[WB] ) ) {
			case( MADD ): writebackMADD(); break;
			case( MADDU ): writebackMADDU(); break;
			case( MUL ): writebackMUL(); break;
			case( MSUB ): writebackMSUB(); break;
			case( MSUBU ): writebackMSUBU(); break;
			case( CLZ ): writebackCLZ(); break;
			case( CLO ): writebackCLO(); break;
			case( MOVZ ): writebackMOVZ(); break;
			case( MOVN ): writebackMOVN(); break;
			default:
				throw "Unhandled opuction";
		}

	} else if ( op == J ) { //the two JTYPE opuctions are following
		
	} else if ( op == JAL ) {


	} else { //ITYPE here
		
		switch( op ) {
			case( ADDI ): writebackADDI(); break;
			case( ADDIU ): writebackADDIU(); break;
			case( SLTI ): writebackSLTI(); break;
			case( SLTIU ): writebackSLTIU(); break;
			case( ANDI ): writebackANDI(); break;
			case( ORI ): writebackORI(); break;
			case( XORI ): writebackXORI(); break;
			case( LUI ): writebackLUI(); break;
			case( LB ): writebackLB(); break;
			case( LH ): writebackLH(); break;
			case( LWL ): writebackLWL(); break;
			case( LW ): writebackLW(); break;
			case( LBU ): writebackLBU(); break;
			case( LHU ): writebackLHU(); break;
			case( LWR ): writebackLWR(); break;
			case( LL ): writebackLL(); break;		//???
			case( SC ): writebackSC(); break;
		} 	
	}
}

