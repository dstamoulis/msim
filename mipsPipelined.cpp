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
#include "safeops.h"

using namespace std;

#define IF  0
#define ID  1
#define EX  2
#define MEM 3
#define WB  4

#define EMPTY_PIPELINE(valid) !( valid[IF] || valid[ID] || valid[EX] || valid[MEM] || valid[WB] )
#define DEP_RS_RT RT( cmd[ID] ) == RT( cmd[EX] ) || RT( cmd[ID] ) == RT( cmd[MEM] ) || RT( cmd[ID] ) == RT( cmd[WB] )
#define DEP_RT_RT RS( cmd[ID] ) == RT( cmd[EX] ) || RS( cmd[ID] ) == RT( cmd[MEM] ) || RS( cmd[ID] ) == RT( cmd[WB] )
#define DEP_RS_RD ( OP( cmd[EX] ) == RTYPE1 || OP( cmd[EX] == RTYPE2 ) && RT( cmd[ID] ) == RD( cmd[EX] ) ) \
					|| ( OP( cmd[MEM] ) == RTYPE1 || OP( cmd[MEM] == RTYPE2 ) && RT( cmd[ID] ) == RD( cmd[MEM] ) ) \
					|| ( OP( cmd[WB] ) == RTYPE1 || OP( cmd[WB] == RTYPE2 ) && RT( cmd[ID] ) == RD( cmd[WB] ) )
#define DEP_RT_RD RS( cmd[ID] ) == RD( cmd[EX] ) || RS( cmd[ID] ) == RD( cmd[MEM] ) || RS( cmd[ID] ) == RD( cmd[WB] )

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

	if( dependence ) 
		return;

	if ( pc > endAddr ) { // TODO : Add J-type case later...
		valid[IF] = false;
		return;
	}

	valid[IF] = true;

	//set IF_ID intermediate register fields
	innerRegs->IFID_setPC( mem->loadWord( pc ) );
	innerRegs->IFID_setNextPC( mem->loadWord( pc + 4 ) );

	pc += 4;
}


void mipsPipelined::decode() {
	//TODO implement check for hazards.


	uint32_t temp = innerRegs->IFID_getPC();
	
	//set fields of IDEX intermediate regiser fields
	innerRegs->IDEX_setRT( reg->getReg(RT(temp)) );
	innerRegs->IDEX_setRS( reg->getReg(RS(temp)) );
	innerRegs->IDEX_setNextPC( innerRegs->IFID_getNextPC() );
	innerRegs->IDEX_setImmed( IMMED(temp) );
	innerRegs->IDEX_setDestRegs( RT(temp), RD(temp) );
	innerRegs->IDEX_setShamt( SHAMT( temp ) );
	//TODO: Add functionallity for J-type cases.

	innerRegs->IDEX_setLO( reg->getLO() );	
	innerRegs->IDEX_setHI( reg->getHI() );
	
}

bool checkDependence() 
{
	if( OP( cmd[ID] ) == RTYPE1 || OP( cmd[ID] ) == RTYPE2 ) {
		
		if( ) {

		} else if () {


		} else if () {


		} else
			if(  DEP_RS_RT || DEP_RT_RT || DEP_RS_RD || DEP_RT_RD ) {
				dependence = 1;
				return true;
			}
	}



}


/********************************************
 *-------------EXECUTION  STAGE-------------*
 * functionality of instruction in EX stage *
 ********************************************/


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
			case( JALR ): executeJALR(); break;
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
				throw "Unhandled operation";
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
				throw "Unhandled operation";
		}

	} else if ( op == J ) { 
		executeJ();
		
	} else if ( op == JAL ) {
		executeJAL();


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
			default:
				throw "Unhandled operation";
		} 	
	}
}

//functionality of MIPS instruction
//in execute stage.
void mipsPipelined::executeSLL()
{
	uint32_t shamt = innerRegs->IDEX_getShamt();
	uint32_t rt = innerRegs->IDEX_getRT();
	innerRegs->EXMEM_setAluRes( rt << shamt );
	innerRegs->EXMEM_setDestRegs(innerRegs->IDEX_getDestRegs() );
	
}

void mipsPipelined::executeSRL()
{
	uint32_t shamt = innerRegs->IDEX_getShamt();
	uint32_t rt = innerRegs->IDEX_getRT();
	innerRegs->EXMEM_setAluRes( rt >> shamt );
	innerRegs->EXMEM_setDestRegs(innerRegs->IDEX_getDestRegs() );
}

void mipsPipelined::executeSRA()
{
	uint32_t shamt = innerRegs->IDEX_getShamt();
	int32_t rt = (int32_t) innerRegs->IDEX_getRT();
	innerRegs->EXMEM_setAluRes( rt >> shamt );
	innerRegs->EXMEM_setDestRegs(innerRegs->IDEX_getDestRegs() );
}

void mipsPipelined::executeSLLV()
{
	uint32_t rs = innerRegs->IDEX_getRS();
	uint32_t rt = innerRegs->IDEX_getRT();
	innerRegs->EXMEM_setAluRes( rt << rs );
	innerRegs->EXMEM_setDestRegs(innerRegs->IDEX_getDestRegs() );
}

void mipsPipelined::executeSRLV()
{
	uint32_t rs = innerRegs->IDEX_getRS();
	uint32_t rt = innerRegs->IDEX_getRT();
	innerRegs->EXMEM_setAluRes( rt >> rs );
	innerRegs->EXMEM_setDestRegs(innerRegs->IDEX_getDestRegs() );
}

void mipsPipelined::executeSRAV()
{
	uint32_t rs = innerRegs->IDEX_getRS();
	int32_t rt = innerRegs->IDEX_getRT();
	innerRegs->EXMEM_setAluRes( rt >> rs );
	innerRegs->EXMEM_setDestRegs(innerRegs->IDEX_getDestRegs() );
}

void mipsPipelined::executeJR()
{
	//TODO
}

void mipsPipelined::executeJALR()
{
	//TODO
}

void mipsPipelined::executeBREAK()
{
	//TODO
}

void mipsPipelined::executeMFHI()
{
	uint32_t hi = reg->getHI();
	innerRegs->EXMEM_setAluRes( hi );
	innerRegs->EXMEM_setDestRegs(innerRegs->IDEX_getDestRegs() );
}

void mipsPipelined::executeMTHI()
{
	uint32_t rs = innerRegs->IDEX_getRS();
	innerRegs->EXMEM_setAluRes( rs );
}

void mipsPipelined::executeMFLO()
{
	uint32_t lo = reg->getLO();
	innerRegs->EXMEM_setAluRes( lo );
	innerRegs->EXMEM_setDestRegs(innerRegs->IDEX_getDestRegs() );
}

void mipsPipelined::executeMTLO()
{
	uint32_t rs = innerRegs->IDEX_getRS();
	innerRegs->EXMEM_setAluRes( rs );
}

void mipsPipelined::executeMULT()
{
	int32_t rs = (int32_t) innerRegs->IDEX_getRS();
	int32_t rt = (int32_t) innerRegs->IDEX_getRT();

	int64_t result = rs * rt;

	innerRegs->EXMEM_setAluRes(result && 0xffffffff);  // Stores into that register the result for LO
	innerRegs->EXMEM_setAluRes2((result >> 32 ) && 0xffffffff); // Stores into that register the result for HI

}

void mipsPipelined::executeMULTU()
{
	uint32_t rs = innerRegs->IDEX_getRS();
	uint32_t rt = innerRegs->IDEX_getRT();

	uint64_t result = rs * rt;

	innerRegs->EXMEM_setAluRes(result && 0xffffffff);  // Stores into that register the result for LO
	innerRegs->EXMEM_setAluRes2((result >> 32 ) && 0xffffffff); // Stores into that register the result for HI
}

void mipsPipelined::executeDIV()
{
	int32_t rs = (int32_t) innerRegs->IDEX_getRS();
	int32_t rt = (int32_t) innerRegs->IDEX_getRT();

	innerRegs->EXMEM_setAluRes(rs / rt);
	innerRegs->EXMEM_setAluRes2(rs % rt);

	//TODO -> Ask silverwind for overflow + safeops...
}

void mipsPipelined::executeDIVU()
{
	uint32_t rs = innerRegs->IDEX_getRS();
	uint32_t rt = innerRegs->IDEX_getRT();

	innerRegs->EXMEM_setAluRes(rs / rt);
	innerRegs->EXMEM_setAluRes2(rs % rt);

}
void mipsPipelined::executeADD()
{
	int32_t rs = (int32_t) innerRegs->IDEX_getRS();
	int32_t rt = (int32_t) innerRegs->IDEX_getRT();
	innerRegs->EXMEM_setAluRes( add(rt,rs));

	innerRegs->EXMEM_setDestRegs(innerRegs->IDEX_getDestRegs() );
}

void mipsPipelined::executeADDU()
{
	uint32_t rs = innerRegs->IDEX_getRS();
	uint32_t rt = innerRegs->IDEX_getRT();
	
	innerRegs->EXMEM_setAluRes( rt + rs );

	innerRegs->EXMEM_setDestRegs(innerRegs->IDEX_getDestRegs()) ;
}

void mipsPipelined::executeSUB()
{
	int32_t rs = (int32_t) innerRegs->IDEX_getRS();
	int32_t rt = (int32_t) innerRegs->IDEX_getRT();

	innerRegs->EXMEM_setAluRes( subtract(rs,rt));

	innerRegs->EXMEM_setDestRegs(innerRegs->IDEX_getDestRegs() );
}

void mipsPipelined::executeSUBU()
{
	uint32_t rs = innerRegs->IDEX_getRS();
	uint32_t rt = innerRegs->IDEX_getRT();
	
	innerRegs->EXMEM_setAluRes( rs - rt );

	innerRegs->EXMEM_setDestRegs(innerRegs->IDEX_getDestRegs()) ;
}

void mipsPipelined::executeAND()
{
	uint32_t rs = innerRegs->IDEX_getRS();
	uint32_t rt = innerRegs->IDEX_getRT();
	
	innerRegs->EXMEM_setAluRes( rs & rt );

	innerRegs->EXMEM_setDestRegs(innerRegs->IDEX_getDestRegs()) ;
}

void mipsPipelined::executeOR()
{

	uint32_t rs = innerRegs->IDEX_getRS();
	uint32_t rt = innerRegs->IDEX_getRT();
	
	innerRegs->EXMEM_setAluRes( rs | rt );

	innerRegs->EXMEM_setDestRegs(innerRegs->IDEX_getDestRegs()) ;
}

void mipsPipelined::executeXOR()
{
	uint32_t rs = innerRegs->IDEX_getRS();
	uint32_t rt = innerRegs->IDEX_getRT();
	
	innerRegs->EXMEM_setAluRes( rs ^ rt );

	innerRegs->EXMEM_setDestRegs(innerRegs->IDEX_getDestRegs()) ;	
}

void mipsPipelined::executeNOR()
{
	uint32_t rs = innerRegs->IDEX_getRS();
	uint32_t rt = innerRegs->IDEX_getRT();
	
	innerRegs->EXMEM_setAluRes( ~(rs | rt) );

	innerRegs->EXMEM_setDestRegs(innerRegs->IDEX_getDestRegs()) ;
}


void mipsPipelined::executeSLT()
{
	int32_t rs = (int32_t) innerRegs->IDEX_getRS();
	int32_t rt = (int32_t) innerRegs->IDEX_getRT();
	
	innerRegs->EXMEM_setAluRes( (rs<rt) ? 1U : 0 );

	innerRegs->EXMEM_setDestRegs(innerRegs->IDEX_getDestRegs()) ;
}

void mipsPipelined::executeSLTU()
{
	uint32_t rs = innerRegs->IDEX_getRS();
	uint32_t rt = innerRegs->IDEX_getRT();
	
	innerRegs->EXMEM_setAluRes((rs<rt) ? 1U : 0  );

	innerRegs->EXMEM_setDestRegs(innerRegs->IDEX_getDestRegs()) ;
}


void mipsPipelined::executeMADD()
{
	//TODO -> Tsekare an einai swsta ta orismata signed - unsigned (dimitris)

	int32_t rs = (int32_t) innerRegs->IDEX_getRS();
	int32_t rt = (int32_t) innerRegs->IDEX_getRT();

	int64_t result = (int32_t) rs * rt;

	int64_t lo = (int64_t) innerRegs->IDEX_getLO();

	int64_t hi =  ( (int64_t) innerRegs->IDEX_getHI() ) << 32  ;

	int64_t hi_lo = ( hi | lo );

	result += hi_lo;
	

	innerRegs->EXMEM_setAluRes(result && 0xffffffff);  // Stores into that register the result for LO
	innerRegs->EXMEM_setAluRes2((result >> 32 ) && 0xffffffff); // Stores into that register the result for HI


}

void mipsPipelined::executeMADDU()
{
	
	uint32_t rs = innerRegs->IDEX_getRS();
	uint32_t rt = innerRegs->IDEX_getRT();

	uint64_t result = rs * rt;

	uint64_t lo = (uint64_t) innerRegs->IDEX_getLO();

	uint64_t hi =  ( (uint64_t) innerRegs->IDEX_getHI() ) << 32  ;

	uint64_t hi_lo = ( hi | lo );

	result += hi_lo;
	

	innerRegs->EXMEM_setAluRes(result && 0xffffffff);  // Stores into that register the result for LO
	innerRegs->EXMEM_setAluRes2((result >> 32 ) && 0xffffffff); // Stores into that register the result for HI
}

void mipsPipelined::executeMUL()
{



	int32_t rs = (int32_t) innerRegs->IDEX_getRS();
	int32_t rt = (int32_t) innerRegs->IDEX_getRT();

	innerRegs->EXMEM_setDestRegs(innerRegs->IDEX_getDestRegs() );

	int64_t result = rs * rt;

	innerRegs->EXMEM_setAluRes(result && 0xffffffff);  // Stores into that register the result for register rd



}

void mipsPipelined::executeMSUB()
{
	//TODO -> Tsekare an einai swsta ta orismata signed - unsigned (dimitris)

	int32_t rs = (int32_t) innerRegs->IDEX_getRS();
	int32_t rt = (int32_t) innerRegs->IDEX_getRT();

	int64_t result = (int32_t) rs * rt;

	int64_t lo = (int64_t) innerRegs->IDEX_getLO();

	int64_t hi =  ( (int64_t) innerRegs->IDEX_getHI() ) << 32  ;

	int64_t hi_lo = ( hi | lo );

	hi_lo -= result;
	

	innerRegs->EXMEM_setAluRes(hi_lo && 0xffffffff);  // Stores into that register the result for LO
	innerRegs->EXMEM_setAluRes2((hi_lo >> 32 ) && 0xffffffff); // Stores into that register the result for HI
}


void mipsPipelined::executeMSUBU()
{
	uint32_t rs = innerRegs->IDEX_getRS();
	uint32_t rt = innerRegs->IDEX_getRT();

	uint64_t result = rs * rt;

	uint64_t lo = (uint64_t) innerRegs->IDEX_getLO();

	uint64_t hi =  ( (uint64_t) innerRegs->IDEX_getHI() ) << 32  ;

	uint64_t hi_lo = ( hi | lo );

	hi_lo -= result;
	

	innerRegs->EXMEM_setAluRes(hi_lo && 0xffffffff);  // Stores into that register the result for LO
	innerRegs->EXMEM_setAluRes2((hi_lo >> 32 ) && 0xffffffff); // Stores into that register the result for HI
}

void mipsPipelined::executeCLZ()
{

	uint32_t rs = innerRegs->IDEX_getRS();

	innerRegs->EXMEM_setDestRegs(innerRegs->IDEX_getDestRegs() );
	
	uint32_t count = 0;

	__asm__("again1:");

	rs<<1;

	__asm__("jc stop1");

	count++;

	__asm__("jmp again1");

	__asm__("stop1:");

	innerRegs->EXMEM_setAluRes( count ); // Stores into that register the result for register rd

	
}

void mipsPipelined::executeCLO()
{
	uint32_t rs = innerRegs->IDEX_getRS();

	innerRegs->EXMEM_setDestRegs(innerRegs->IDEX_getDestRegs() );
	
	uint32_t count = 0;

	__asm__("again2:");

	rs<<1;

	__asm__("jnc stop2");

	count++;

	__asm__("jmp again2");

	__asm__("stop2:");

	innerRegs->EXMEM_setAluRes( count );
}


void mipsPipelined::executeMOVZ()
{
	uint32_t rs = innerRegs->IDEX_getRS();
	uint32_t rt = innerRegs->IDEX_getRT();

	
	innerRegs->EXMEM_setAluRes( (rt==0) ? 1U :0 ) ; // Stores the comparison's result, in order to have it at WB stage
							
	innerRegs->EXMEM_setAluRes2(rs); // Stores rs' value in order to have it at WB stage.

	innerRegs->EXMEM_setDestRegs(innerRegs->IDEX_getDestRegs()) ;
}

void mipsPipelined::executeMOVN()
{
	uint32_t rs = innerRegs->IDEX_getRS();
	uint32_t rt = innerRegs->IDEX_getRT();

	
	innerRegs->EXMEM_setAluRes( (rt==1) ? 1U :0 ) ; // Stores the comparison's result, in order to have it at WB stage
							
	innerRegs->EXMEM_setAluRes2(rs); // Stores rs' value in order to have it at WB stage.

	innerRegs->EXMEM_setDestRegs(innerRegs->IDEX_getDestRegs()) ;
}





void mipsPipelined::executeJ()
{
	//TODO
}


void mipsPipelined::executeJAL()
{
	//TODO
}



//TODO -> Fix branch cases, ask teammates for MIPS's delay slots issue.

void mipsPipelined::executeBEQ()
{
	//TODO	
}

void mipsPipelined::executeBNE()
{
	//TODO	
}
void mipsPipelined::executeBLEZ()
{
	//TODO	
}

void mipsPipelined::executeBGEZ()
{
	//TODO	
}



void mipsPipelined::executeADDI()
{
	int32_t rs = (int32_t) innerRegs->IDEX_getRS();
	innerRegs->EXMEM_setAluRes( add( signExtend( (int16_t) innerRegs->IDEX_getImmed() ) ,rs));
	innerRegs->EXMEM_setDestRegs(innerRegs->IDEX_getDestRegs() );	
}

void mipsPipelined::executeADDIU()
{
	uint32_t rs = innerRegs->IDEX_getRS();
	innerRegs->EXMEM_setAluRes( rs + signExtend( (int16_t) innerRegs->IDEX_getImmed() ) );
	innerRegs->EXMEM_setDestRegs(innerRegs->IDEX_getDestRegs() );
}

void mipsPipelined::executeSLTI()
{
	int32_t rs = (int32_t) innerRegs->IDEX_getRS();
	innerRegs->EXMEM_setAluRes( ( rs < signExtend( (int16_t) innerRegs->IDEX_getImmed() )  )  ? 1U : 0  );
	innerRegs->EXMEM_setDestRegs(innerRegs->IDEX_getDestRegs() );	
}

void mipsPipelined::executeSLTIU()
{
	uint32_t rs = innerRegs->IDEX_getRS();
	innerRegs->EXMEM_setAluRes( ( rs < (uint32_t) innerRegs->IDEX_getImmed()   )  ? 1U : 0  );
	innerRegs->EXMEM_setDestRegs(innerRegs->IDEX_getDestRegs() );
}

void mipsPipelined::executeANDI()
{
	uint32_t rs = innerRegs->IDEX_getRS();
	innerRegs->EXMEM_setAluRes( rs & (uint32_t) innerRegs->IDEX_getImmed()  );
	innerRegs->EXMEM_setDestRegs(innerRegs->IDEX_getDestRegs() );
}


void mipsPipelined::executeORI()
{
	uint32_t rs = innerRegs->IDEX_getRS();
	innerRegs->EXMEM_setAluRes( rs | (uint32_t) innerRegs->IDEX_getImmed()  );
	innerRegs->EXMEM_setDestRegs(innerRegs->IDEX_getDestRegs() );

}



void mipsPipelined::executeXORI()
{
	uint32_t rs = innerRegs->IDEX_getRS();
	innerRegs->EXMEM_setAluRes( ~ ( rs | (uint32_t) innerRegs->IDEX_getImmed() ) );
	innerRegs->EXMEM_setDestRegs(innerRegs->IDEX_getDestRegs() );
}


void mipsPipelined::executeLUI()
{
	uint32_t res = (uint32_t) innerRegs->IDEX_getImmed();
	innerRegs->EXMEM_setAluRes( res << 16 );
	innerRegs->EXMEM_setDestRegs(innerRegs->IDEX_getDestRegs() );
}



// Rwta ton mpampi an exoun alli diafora sto execute stage... nomizw mono sto WB allazoun
// pou kratas 8, 16 kai 32 bits


void mipsPipelined::executeLB()
{
	int32_t addr = (int32_t) signExtend( (int16_t) innerRegs->IDEX_getImmed() ); // get byte's address
	innerRegs->EXMEM_setAluRes( addr );  // pass byte's address at EXMEM register
	innerRegs->EXMEM_setDestRegs(innerRegs->IDEX_getDestRegs() ); 
}



void mipsPipelined::executeLH()
{
	int32_t addr = (int32_t) signExtend( (int16_t) innerRegs->IDEX_getImmed() );
	innerRegs->EXMEM_setAluRes( addr );
	innerRegs->EXMEM_setDestRegs(innerRegs->IDEX_getDestRegs() );
}


void mipsPipelined::executeLWL()
{
	//TODO
	// ask teammates for function's usage
}


void mipsPipelined::executeLW()
{
	int32_t addr =  (int32_t) signExtend( (int16_t) innerRegs->IDEX_getImmed()   );
	innerRegs->EXMEM_setAluRes( addr );
	innerRegs->EXMEM_setDestRegs( innerRegs->IDEX_getDestRegs() );
}


void mipsPipelined::executeLBU()
{
	innerRegs->EXMEM_setAluRes( (uint32_t) innerRegs->IDEX_getImmed() );
	innerRegs->EXMEM_setDestRegs(innerRegs->IDEX_getDestRegs() );
}


void mipsPipelined::executeLHU()
{
	innerRegs->EXMEM_setAluRes( (uint32_t) innerRegs->IDEX_getImmed() );
	innerRegs->EXMEM_setDestRegs(innerRegs->IDEX_getDestRegs() );
}


void mipsPipelined::executeLWR()
{
	//TODO
	// ask teammates for function's usage
}




void mipsPipelined::executeSB()
{
	int32_t addr = (int32_t) signExtend( (int16_t) innerRegs->IDEX_getImmed() ); // get byte's address
	innerRegs->EXMEM_setAluRes( addr );  // pass byte's address at EXMEM register
	uint32_t rt = innerRegs->IDEX_getRT();
	innerRegs->EXMEM_setStoreData( rt && 0xff );
}

void mipsPipelined::executeSH()
{
	int32_t addr = (int32_t) signExtend( (int16_t) innerRegs->IDEX_getImmed() );
	innerRegs->EXMEM_setAluRes( addr );  
	uint32_t rt = innerRegs->IDEX_getRT();
	innerRegs->EXMEM_setStoreData( rt && 0xffff );
}

void mipsPipelined::executeSWL()
{
	//TODO
}

void mipsPipelined::executeSW()
{
	int32_t addr = (int32_t) signExtend( (int16_t) innerRegs->IDEX_getImmed() );
	innerRegs->EXMEM_setAluRes( addr );  
	uint32_t rt = innerRegs->IDEX_getRT();
	innerRegs->EXMEM_setStoreData( rt );
}

void mipsPipelined::executeSWR()
{
	//TODO
	// ask teammates for function's usage
}

// TODO  SOS   => Ask MPAMPIS about RMW... ???????????
void mipsPipelined::executeLL()
{
	//TODO
}

void mipsPipelined::executeSC()
{
	//TODO
}

void mipsPipelined::memory()
{
	uint32_t op = OP( cmd[MEM] );
	if( op == RTYPE1 ) {
		switch( FUNCT( cmd[MEM] ) ) {		

			case( SLL ): memorySLL(); break;
			case( SRL ): memorySRL(); break;
			case( SRA ): memorySRA(); break;
			case( SLLV ): memorySLLV(); break;
			case( SRLV ): memorySRLV(); break;
			case( SRAV ): memorySRAV(); break;
			case( JR ): memoryJR(); break;
			case( JALR ): memoryJALR(); break;
			case( BREAK ): memoryBREAK(); break;
			case( MFHI ): memoryMFHI(); break;
			case( MTHI ): memoryMTHI(); break;
			case( MFLO ): memoryMFLO(); break;
			case( MTLO ): memoryMTLO(); break;
			case( MULT ): memoryMULT(); break;
			case( MULTU ): memoryMULTU(); break;
			case( DIV ): memoryDIV(); break;
			case( DIVU ): memoryDIVU(); break;
			case( ADD ): memoryADD(); break;
			case( ADDU ): memoryADDU(); break;
			case( SUB ): memorySUB(); break;
			case( SUBU ): memorySUBU(); break;
			case( AND ): memoryAND(); break;
			case( OR ): memoryOR(); break;
			case( XOR ): memoryXOR(); break;
			case( NOR ): memoryNOR(); break;
			case( SLT ): memorySLT(); break;
			case( SLTU ): memorySLTU(); break;
			default:
				break;
		}
			
	} else if ( op == RTYPE2 ) {
	
		//RTYPE2 opuctions
		switch( FUNCT( cmd[EX] ) ) {
			case( MADD ): memoryMADD(); break;
			case( MADDU ): memoryMADDU(); break;
			case( MUL ): memoryMUL(); break;
			case( MSUB ): memoryMSUB(); break;
			case( MSUBU ): memoryMSUBU(); break;
			case( CLZ ): memoryCLZ(); break;
			case( CLO ): memoryCLO(); break;
			case( MOVZ ): memoryMOVZ(); break;
			case( MOVN ): memoryMOVN(); break;
			default:
				throw "Unhandled operation";
		}

	} else if ( op == J ) { //the two JTYPE opuctions are following
		
	} else if ( op == JAL ) {

	// TODO -> simplirwse J kai JAL


	} else { //ITYPE here
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
SB
SH
SWL
SW
SWR

den tis eixe kai o mpampis epeidi den kanoun tpt sto WB
alla rwta teammates an toulaxiston eksasfalizw orthotita

*/
		switch( op ) {
			case( BGEZ ):
				//TODO: handle case of BGEZ, BGEZAL, BLTZAL, BLTZ
				break;
			case( BEQ ): memoryBEQ(); break;
			case( BNE ): memoryBNE(); break;
			case( BLEZ ): memoryBLEZ(); break;
			case( BGTZ ): memoryBGEZ(); break;
			case( ADDI ): memoryADDI(); break;
			case( ADDIU ): memoryADDIU(); break;
			case( SLTI ): memorySLTI(); break;
			case( SLTIU ): memorySLTIU(); break;
			case( ANDI ): memoryANDI(); break;
			case( ORI ): memoryORI(); break;
			case( XORI ): memoryXORI(); break;
			case( LUI ): memoryLUI(); break;
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



//functionality of MIPS instruction
//in memory stage.
void mipsPipelined::memorySLL()
{
	innerRegs->MEMWB_setAlu( innerRegs->EXMEM_getAluRes() );
	innerRegs->MEMWB_setDestRegs( innerRegs->EXMEM_getDestRegs() );
}

void mipsPipelined::memorySRL()
{
	innerRegs->MEMWB_setAlu( innerRegs->EXMEM_getAluRes() );
	innerRegs->MEMWB_setDestRegs( innerRegs->EXMEM_getDestRegs() );
}

void mipsPipelined::memorySRA()
{	
	innerRegs->MEMWB_setAlu( innerRegs->EXMEM_getAluRes() );
	innerRegs->MEMWB_setDestRegs( innerRegs->EXMEM_getDestRegs() );
}

void mipsPipelined::memorySLLV()
{
	innerRegs->MEMWB_setAlu( innerRegs->EXMEM_getAluRes() );
	innerRegs->MEMWB_setDestRegs( innerRegs->EXMEM_getDestRegs() );
}

void mipsPipelined::memorySRLV()
{
	innerRegs->MEMWB_setAlu( innerRegs->EXMEM_getAluRes() );
	innerRegs->MEMWB_setDestRegs( innerRegs->EXMEM_getDestRegs() );
}

void mipsPipelined::memorySRAV()
{
	innerRegs->MEMWB_setAlu( innerRegs->EXMEM_getAluRes() );
	innerRegs->MEMWB_setDestRegs( innerRegs->EXMEM_getDestRegs() );
}

void mipsPipelined::memoryJR()
{
	//TODO
}

void mipsPipelined::memoryJALR()
{
	//TODO
}

void mipsPipelined::memoryBREAK()
{
	//TODO
}

void mipsPipelined::memoryMFHI()
{
	innerRegs->MEMWB_setAlu( innerRegs->EXMEM_getAluRes() );
	innerRegs->MEMWB_setDestRegs( innerRegs->EXMEM_getDestRegs() );
}

void mipsPipelined::memoryMTHI()
{
	innerRegs->MEMWB_setAlu( innerRegs->EXMEM_getAluRes() );
}

void mipsPipelined::memoryMFLO()
{
	innerRegs->MEMWB_setAlu( innerRegs->EXMEM_getAluRes() );
	innerRegs->MEMWB_setDestRegs( innerRegs->EXMEM_getDestRegs() );
}

void mipsPipelined::memoryMTLO()
{
	innerRegs->MEMWB_setAlu( innerRegs->EXMEM_getAluRes() );
}

void mipsPipelined::memoryMULT()
{
	innerRegs->MEMWB_setAlu( innerRegs->EXMEM_getAluRes() );
	innerRegs->MEMWB_setAlu2( innerRegs->EXMEM_getAluRes2() );

}

void mipsPipelined::memoryMULTU()
{
	innerRegs->MEMWB_setAlu( innerRegs->EXMEM_getAluRes() );
	innerRegs->MEMWB_setAlu2( innerRegs->EXMEM_getAluRes2() );
}

void mipsPipelined::memoryDIV()
{
	innerRegs->MEMWB_setAlu( innerRegs->EXMEM_getAluRes() );
	innerRegs->MEMWB_setAlu2( innerRegs->EXMEM_getAluRes2() );

}

void mipsPipelined::memoryDIVU()
{
	innerRegs->MEMWB_setAlu( innerRegs->EXMEM_getAluRes() );
	innerRegs->MEMWB_setAlu2( innerRegs->EXMEM_getAluRes2() );

}

void mipsPipelined::memoryADD()
{
	innerRegs->MEMWB_setAlu( innerRegs->EXMEM_getAluRes() );
	innerRegs->MEMWB_setDestRegs( innerRegs->EXMEM_getDestRegs() );
}

void mipsPipelined::memoryADDU()
{
	innerRegs->MEMWB_setAlu( innerRegs->EXMEM_getAluRes() );
	innerRegs->MEMWB_setDestRegs( innerRegs->EXMEM_getDestRegs() );
}

void mipsPipelined::memorySUB()
{
	innerRegs->MEMWB_setAlu( innerRegs->EXMEM_getAluRes() );
	innerRegs->MEMWB_setDestRegs( innerRegs->EXMEM_getDestRegs() );
}

void mipsPipelined::memorySUBU()
{
	innerRegs->MEMWB_setAlu( innerRegs->EXMEM_getAluRes() );
	innerRegs->MEMWB_setDestRegs( innerRegs->EXMEM_getDestRegs() );
}

void mipsPipelined::memoryAND()
{
	innerRegs->MEMWB_setAlu( innerRegs->EXMEM_getAluRes() );
	innerRegs->MEMWB_setDestRegs( innerRegs->EXMEM_getDestRegs() );
}

void mipsPipelined::memoryOR()
{

	innerRegs->MEMWB_setAlu( innerRegs->EXMEM_getAluRes() );
	innerRegs->MEMWB_setDestRegs( innerRegs->EXMEM_getDestRegs() );
}

void mipsPipelined::memoryXOR()
{
	innerRegs->MEMWB_setAlu( innerRegs->EXMEM_getAluRes() );
	innerRegs->MEMWB_setDestRegs( innerRegs->EXMEM_getDestRegs() );
}

void mipsPipelined::memoryNOR()
{
	innerRegs->MEMWB_setAlu( innerRegs->EXMEM_getAluRes() );
	innerRegs->MEMWB_setDestRegs( innerRegs->EXMEM_getDestRegs() );
}


void mipsPipelined::memorySLT()
{
	innerRegs->MEMWB_setAlu( innerRegs->EXMEM_getAluRes() );
	innerRegs->MEMWB_setDestRegs( innerRegs->EXMEM_getDestRegs() );
}

void mipsPipelined::memorySLTU()
{
	innerRegs->MEMWB_setAlu( innerRegs->EXMEM_getAluRes() );
	innerRegs->MEMWB_setDestRegs( innerRegs->EXMEM_getDestRegs() );
}


void mipsPipelined::memoryMADD()
{
	innerRegs->MEMWB_setAlu( innerRegs->EXMEM_getAluRes() );
	innerRegs->MEMWB_setAlu2( innerRegs->EXMEM_getAluRes2() );
}

void mipsPipelined::memoryMADDU()
{
	innerRegs->MEMWB_setAlu( innerRegs->EXMEM_getAluRes() );
	innerRegs->MEMWB_setAlu2( innerRegs->EXMEM_getAluRes2() );
}

void mipsPipelined::memoryMUL()
{
	innerRegs->MEMWB_setAlu( innerRegs->EXMEM_getAluRes() );
	innerRegs->MEMWB_setDestRegs( innerRegs->EXMEM_getDestRegs() );
}

void mipsPipelined::memoryMSUB()
{
	innerRegs->MEMWB_setAlu( innerRegs->EXMEM_getAluRes() );
	innerRegs->MEMWB_setAlu2( innerRegs->EXMEM_getAluRes2() );
}


void mipsPipelined::memoryMSUBU()
{
	innerRegs->MEMWB_setAlu( innerRegs->EXMEM_getAluRes() );
	innerRegs->MEMWB_setAlu2( innerRegs->EXMEM_getAluRes2() );
}

void mipsPipelined::memoryCLZ()
{
	innerRegs->MEMWB_setAlu( innerRegs->EXMEM_getAluRes() );
	innerRegs->MEMWB_setDestRegs( innerRegs->EXMEM_getDestRegs() );
}

void mipsPipelined::memoryCLO()
{
	innerRegs->MEMWB_setAlu( innerRegs->EXMEM_getAluRes() );
	innerRegs->MEMWB_setDestRegs( innerRegs->EXMEM_getDestRegs() );
}


void mipsPipelined::memoryMOVZ()
{
	innerRegs->MEMWB_setAlu( innerRegs->EXMEM_getAluRes() );
	innerRegs->MEMWB_setAlu2( innerRegs->EXMEM_getAluRes2() );
	innerRegs->MEMWB_setDestRegs( innerRegs->EXMEM_getDestRegs() );
}

void mipsPipelined::memoryMOVN()
{
	innerRegs->MEMWB_setAlu( innerRegs->EXMEM_getAluRes() );
	innerRegs->MEMWB_setAlu2( innerRegs->EXMEM_getAluRes2() );
	innerRegs->MEMWB_setDestRegs( innerRegs->EXMEM_getDestRegs() );
}


// TODO -> simplirwse J kai JAL


void mipsPipelined::memoryBEQ()
{

}

void mipsPipelined::memoryBNE()
{

}

void mipsPipelined::memoryBLEZ()
{

}

void mipsPipelined::memoryBGEZ()
{

}

void mipsPipelined::memoryADDI()
{
	innerRegs->MEMWB_setAlu( innerRegs->EXMEM_getAluRes() );
	innerRegs->MEMWB_setDestRegs( innerRegs->EXMEM_getDestRegs() );	
}

void mipsPipelined::memoryADDIU()
{
	innerRegs->MEMWB_setAlu( innerRegs->EXMEM_getAluRes() );
	innerRegs->MEMWB_setDestRegs( innerRegs->EXMEM_getDestRegs() );;
}

void mipsPipelined::memorySLTI()
{
	innerRegs->MEMWB_setAlu( innerRegs->EXMEM_getAluRes() );
	innerRegs->MEMWB_setDestRegs( innerRegs->EXMEM_getDestRegs() );
}

void mipsPipelined::memorySLTIU()
{
	innerRegs->MEMWB_setAlu( innerRegs->EXMEM_getAluRes() );
	innerRegs->MEMWB_setDestRegs( innerRegs->EXMEM_getDestRegs() );
}

void mipsPipelined::memoryANDI()
{
	innerRegs->MEMWB_setAlu( innerRegs->EXMEM_getAluRes() );
	innerRegs->MEMWB_setDestRegs( innerRegs->EXMEM_getDestRegs() );
}


void mipsPipelined::memoryORI()
{
	innerRegs->MEMWB_setAlu( innerRegs->EXMEM_getAluRes() );
	innerRegs->MEMWB_setDestRegs( innerRegs->EXMEM_getDestRegs() );

}

void mipsPipelined::memoryXORI()
{
	innerRegs->MEMWB_setAlu( innerRegs->EXMEM_getAluRes() );
	innerRegs->MEMWB_setDestRegs( innerRegs->EXMEM_getDestRegs() );
}

void mipsPipelined::memoryLUI()
{
	innerRegs->MEMWB_setAlu( innerRegs->EXMEM_getAluRes() );
	innerRegs->MEMWB_setDestRegs( innerRegs->EXMEM_getDestRegs() );
}

void mipsPipelined::memoryLB()
{
	uint32_t addr = innerRegs->EXMEM_getAluRes();
	uint8_t val = mem->loadByte( addr );
	innerRegs->MEMWB_setMem( signExtend( (int8_t)val ) );
	innerRegs->MEMWB_setDestRegs( innerRegs->EXMEM_getDestRegs() );
}

void mipsPipelined::memoryLH()
{
	uint32_t addr = innerRegs->EXMEM_getAluRes();
	uint16_t val = mem->loadHalfWord( addr );
	innerRegs->MEMWB_setMem( signExtend( (int16_t)val ) );
	innerRegs->MEMWB_setDestRegs( innerRegs->EXMEM_getDestRegs() );
}

void mipsPipelined::memoryLWL() 
{
	uint32_t res = 0;
	uint32_t addr = innerRegs->EXMEM_getAluRes();
	uint8_t bytes = 4 - addr%4;
	for( int i=1; i<=bytes; ++i ) {
		res = res | ( mem->loadByte( addr ) << ( (4-i)*8 ) ); 
		addr++;
	}
}
void mipsPipelined::memoryLW()
{
	uint32_t addr = innerRegs->EXMEM_getAluRes();
	uint32_t val = mem->loadWord( addr );
	innerRegs->MEMWB_setMem( val );
	innerRegs->MEMWB_setDestRegs( innerRegs->EXMEM_getDestRegs() );
}

void mipsPipelined::memoryLBU()
{
	uint32_t addr = innerRegs->EXMEM_getAluRes();
	uint8_t val = mem->loadByte( addr );
	innerRegs->MEMWB_setMem( (uint32_t)val );
	innerRegs->MEMWB_setDestRegs( innerRegs->EXMEM_getDestRegs() );
}

void mipsPipelined::memoryLHU()
{
	uint32_t addr = innerRegs->EXMEM_getAluRes();
	uint16_t val = mem->loadHalfWord( addr );
	innerRegs->MEMWB_setMem( (uint32_t) val  );
	innerRegs->MEMWB_setDestRegs( innerRegs->EXMEM_getDestRegs() );

}
void mipsPipelined::memoryLWR()
{
	uint32_t res = 0;
	uint32_t addr = innerRegs->EXMEM_getAluRes();
	uint8_t bytes = 1 + addr%4;
	for( i=0; i<bytes; ++i ) {
		res = res | ( mem->loadByte( addr ) << (i*8) );
		addr--;
	}
}

void mipsPipelined::memorySB()
{
	uint32_t addr = innerRegs->EXMEM_getAluRes();
	uint32_t data = innerRegs->EXMEM_getStoreData();
	mem->storeByte( addr,  (uint8_t) data );
}

void mipsPipelined::memorySH()
{
	uint32_t addr = innerRegs->EXMEM_getAluRes();
	uint32_t data = innerRegs->EXMEM_getStoreData();
	mem->storeHalfWord( addr, (uint16_t) data);
}

void mipsPipelined::memorySWL()
{
	//TODO
}
void mipsPipelined::memorySW()
{
	uint32_t addr = innerRegs->EXMEM_getAluRes();
	uint32_t data = innerRegs->EXMEM_getStoreData();
	mem->storeWord( addr,data );
}
void mipsPipelined::memorySWR()
{
	//TODO
}

void mipsPipelined::memoryLL() 
{
	uint32_t addr = innerRegs->EXMEM_getAluRes();
	uint32_t val = mem->loadWord( addr );
	innerRegs->MEMWB_setMem( val );
	innerRegs->MEMWB_setDestRegs( innerRegs->EXMEM_getDestRegs() );
	ll = 1;
}

void mipsPipelined::memorySC()
{
	uint32_t addr = innerRegs->EXMEM_getAluRes();
	uint32_t data = innerRegs->EXMEM_getStoreData();
	if( ll == 1 ) 
		mem->storeWord( addr,data );

	innerRegs->MEMWB_setAlu( ll );
	innerRegs->MEMWB_setDestRegs( innerRegs->EXMEM_getDestRegs() );
}


/*********************************************
 *-----------------WRITE BACK----------------*
 * functionality of instruction in WB stage  *
 ********************************************/


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
SB
SH
SWL
SW
SWR

den tis eixe kai o mpampis epeidi den kanoun tpt sto WB
alla rwta teammates an toulaxiston eksasfalizw orthotita

*/


void mipsPipelined::writebackSLL()
{

	/*

	sos => rwta mpampi an dinw swsta orismata..


	*/

	reg->setReg( (unsigned int) innerRegs->MEMWB_getDestRegRD()  , (int32_t) innerRegs->MEMWB_getAlu() );


	
}

void mipsPipelined::writebackSRL()
{
	reg->setReg( (unsigned int) innerRegs->MEMWB_getDestRegRD()  , (int32_t) innerRegs->MEMWB_getAlu() );
}

void mipsPipelined::writebackSRA()
{	
	reg->setReg( (unsigned int) innerRegs->MEMWB_getDestRegRD()  , (int32_t) innerRegs->MEMWB_getAlu() );
}

void mipsPipelined::writebackSLLV()
{
	reg->setReg( (unsigned int) innerRegs->MEMWB_getDestRegRD()  , (int32_t) innerRegs->MEMWB_getAlu() );
}

void mipsPipelined::writebackSRLV()
{
	reg->setReg( (unsigned int) innerRegs->MEMWB_getDestRegRD()  , (int32_t) innerRegs->MEMWB_getAlu() );
}

void mipsPipelined::writebackSRAV()
{
	reg->setReg( (unsigned int) innerRegs->MEMWB_getDestRegRD()  , (int32_t) innerRegs->MEMWB_getAlu() );
}




void mipsPipelined::writebackMFHI()
{
	reg->setReg( (unsigned int) innerRegs->MEMWB_getDestRegRD()  , (int32_t) innerRegs->MEMWB_getAlu() );
}

void mipsPipelined::writebackMTHI()
{
	reg->setHI( (int32_t) innerRegs->MEMWB_getAlu() );
}

void mipsPipelined::writebackMFLO()
{
	reg->setReg( (unsigned int) innerRegs->MEMWB_getDestRegRD()  , (int32_t) innerRegs->MEMWB_getAlu() );
}

void mipsPipelined::writebackMTLO()
{
	reg->setLO( (int32_t) innerRegs->MEMWB_getAlu() );
}

void mipsPipelined::writebackMULT()
{
	
	reg->setLO( (int32_t) innerRegs->MEMWB_getAlu() );
	reg->setHI( (int32_t) innerRegs->MEMWB_getAlu2() );

}

void mipsPipelined::writebackMULTU()
{
	reg->setLO( (int32_t) innerRegs->MEMWB_getAlu() );
	reg->setHI( (int32_t) innerRegs->MEMWB_getAlu2() );
}

void mipsPipelined::writebackDIV()
{
	reg->setLO( (int32_t) innerRegs->MEMWB_getAlu() );
	reg->setHI( (int32_t) innerRegs->MEMWB_getAlu2() );

}

void mipsPipelined::writebackDIVU()
{
	reg->setLO( (int32_t) innerRegs->MEMWB_getAlu() );
	reg->setHI( (int32_t) innerRegs->MEMWB_getAlu2() );
}

void mipsPipelined::writebackADD()
{
	reg->setReg( (unsigned int) innerRegs->MEMWB_getDestRegRD()  , (int32_t) innerRegs->MEMWB_getAlu() );
}

void mipsPipelined::writebackADDU()
{
	reg->setReg( (unsigned int) innerRegs->MEMWB_getDestRegRD()  , (int32_t) innerRegs->MEMWB_getAlu() );
}

void mipsPipelined::writebackSUB()
{
	reg->setReg( (unsigned int) innerRegs->MEMWB_getDestRegRD()  , (int32_t) innerRegs->MEMWB_getAlu() );
}

void mipsPipelined::writebackSUBU()
{
	reg->setReg( (unsigned int) innerRegs->MEMWB_getDestRegRD()  , (int32_t) innerRegs->MEMWB_getAlu() );
}

void mipsPipelined::writebackAND()
{
	reg->setReg( (unsigned int) innerRegs->MEMWB_getDestRegRD()  , (int32_t) innerRegs->MEMWB_getAlu() );
}

void mipsPipelined::writebackOR()
{
	reg->setReg( (unsigned int) innerRegs->MEMWB_getDestRegRD()  , (int32_t) innerRegs->MEMWB_getAlu() );
}

void mipsPipelined::writebackXOR()
{
	reg->setReg( (unsigned int) innerRegs->MEMWB_getDestRegRD()  , (int32_t) innerRegs->MEMWB_getAlu() );
}

void mipsPipelined::writebackNOR()
{
	reg->setReg( (unsigned int) innerRegs->MEMWB_getDestRegRD()  , (int32_t) innerRegs->MEMWB_getAlu() );
}


void mipsPipelined::writebackSLT()
{
	reg->setReg( (unsigned int) innerRegs->MEMWB_getDestRegRD()  , (int32_t) innerRegs->MEMWB_getAlu() );
}

void mipsPipelined::writebackSLTU()
{
	reg->setReg( (unsigned int) innerRegs->MEMWB_getDestRegRD()  , (int32_t) innerRegs->MEMWB_getAlu() );
}


void mipsPipelined::writebackMADD()
{
	reg->setLO( (int32_t) innerRegs->MEMWB_getAlu() );
	reg->setHI( (int32_t) innerRegs->MEMWB_getAlu2() );

}

void mipsPipelined::writebackMADDU()
{
	reg->setLO( (int32_t) innerRegs->MEMWB_getAlu() );
	reg->setHI( (int32_t) innerRegs->MEMWB_getAlu2() );

}

void mipsPipelined::writebackMUL()
{
	reg->setReg( (unsigned int) innerRegs->MEMWB_getDestRegRD()  , (int32_t) innerRegs->MEMWB_getAlu() );
}

void mipsPipelined::writebackMSUB()
{
	reg->setLO( (int32_t) innerRegs->MEMWB_getAlu() );
	reg->setHI( (int32_t) innerRegs->MEMWB_getAlu2() );
}


void mipsPipelined::writebackMSUBU()
{
	reg->setLO( (int32_t) innerRegs->MEMWB_getAlu() );
	reg->setHI( (int32_t) innerRegs->MEMWB_getAlu2() );
}

void mipsPipelined::writebackCLZ()
{
	reg->setReg( (unsigned int) innerRegs->MEMWB_getDestRegRD()  , (int32_t) innerRegs->MEMWB_getAlu() );
	
}

void mipsPipelined::writebackCLO()
{
	reg->setReg( (unsigned int) innerRegs->MEMWB_getDestRegRD()  , (int32_t) innerRegs->MEMWB_getAlu() );
}


void mipsPipelined::writebackMOVZ()
{


	if ( innerRegs->MEMWB_getAlu() == 1 )
	{

	reg->setReg( (unsigned int) innerRegs->MEMWB_getDestRegRD()  , (int32_t) innerRegs->MEMWB_getAlu2() );

	}

}

void mipsPipelined::writebackMOVN()
{
	

	if ( innerRegs->MEMWB_getAlu() == 1 )
	{

	reg->setReg( (unsigned int) innerRegs->MEMWB_getDestRegRD()  , (int32_t) innerRegs->MEMWB_getAlu2() );

	}
}






void mipsPipelined::writebackADDI()
{
	reg->setReg( (unsigned int) innerRegs->MEMWB_getDestRegRT()  , (int32_t) innerRegs->MEMWB_getAlu() );
}

void mipsPipelined::writebackADDIU()
{
	reg->setReg( (unsigned int) innerRegs->MEMWB_getDestRegRT()  , (int32_t) innerRegs->MEMWB_getAlu() );
}

void mipsPipelined::writebackSLTI()
{
	reg->setReg( (unsigned int) innerRegs->MEMWB_getDestRegRT()  , (int32_t) innerRegs->MEMWB_getAlu() );
}

void mipsPipelined::writebackSLTIU()
{
	reg->setReg( (unsigned int) innerRegs->MEMWB_getDestRegRT()  , (int32_t) innerRegs->MEMWB_getAlu() );
}

void mipsPipelined::writebackANDI()
{
	reg->setReg( (unsigned int) innerRegs->MEMWB_getDestRegRT()  , (int32_t) innerRegs->MEMWB_getAlu() );
}


void mipsPipelined::writebackORI()
{
	reg->setReg( (unsigned int) innerRegs->MEMWB_getDestRegRT()  , (int32_t) innerRegs->MEMWB_getAlu() );

}



void mipsPipelined::writebackXORI()
{
	reg->setReg( (unsigned int) innerRegs->MEMWB_getDestRegRT()  , (int32_t) innerRegs->MEMWB_getAlu() );
}


void mipsPipelined::writebackLUI()
{
	reg->setReg( (unsigned int) innerRegs->MEMWB_getDestRegRT()  , (int32_t) innerRegs->MEMWB_getAlu() );

}





void mipsPipelined::writebackLB()
{
	reg->setReg( (unsigned int) innerRegs->MEMWB_getDestRegRT()  , (int32_t) innerRegs->MEMWB_getMem() );
}

void mipsPipelined::writebackLH()
{
	reg->setReg( (unsigned int) innerRegs->MEMWB_getDestRegRT()  , (int32_t) innerRegs->MEMWB_getMem() );
}

void mipsPipelined::writebackLWL() 
{
	//TODO
}
void mipsPipelined::writebackLW()
{
	reg->setReg( (unsigned int) innerRegs->MEMWB_getDestRegRT()  , (int32_t) innerRegs->MEMWB_getMem() );
}

void mipsPipelined::writebackLBU()
{
	reg->setReg( (unsigned int) innerRegs->MEMWB_getDestRegRT()  , (int32_t) innerRegs->MEMWB_getMem() );
}

void mipsPipelined::writebackLHU()
{
	reg->setReg( (unsigned int) innerRegs->MEMWB_getDestRegRT()  , (int32_t) innerRegs->MEMWB_getMem() );

}
void mipsPipelined::writebackLWR()
{
	//TODO
}





void mipsPipelined::writebackLL() 
{

// TODO


}

void mipsPipelined::writebackSC()
{
//TODO


}






