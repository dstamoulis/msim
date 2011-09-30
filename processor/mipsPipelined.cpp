/* 
 * mipsPipelined.cpp
 * implementation of a mips processor.
 * implements the classic five-stage pipeline
 */
#include <iostream>
#include <iomanip>
#include <sstream>
#include "mipsPipelined.h"
#include "../memory/memory.h"
#include "register_file.h"
#include "safeops.h"

using namespace std;

#define EMPTY_PIPELINE(valid) !( valid[IF] || valid[ID] || valid[EX] || valid[MEM] || valid[WB] )

#define DEP_ID_EX ( ( srcRegs[ID][0] != INVAL_REG && ( srcRegs[ID][0] == dstRegs[EX][0] || srcRegs[ID][0] == dstRegs[EX][1] ) ) \
				 || ( srcRegs[ID][1] != INVAL_REG && ( srcRegs[ID][1] == dstRegs[EX][0] || srcRegs[ID][1] == dstRegs[EX][1] ) ) )

#define DEP_ID_MEM ( ( srcRegs[ID][0] != INVAL_REG && ( srcRegs[ID][0] == dstRegs[MEM][0] || srcRegs[ID][0] == dstRegs[MEM][1] ) ) \
				 || ( srcRegs[ID][1] != INVAL_REG && ( srcRegs[ID][1] == dstRegs[MEM][0] || srcRegs[ID][1] == dstRegs[MEM][1] ) ) )

#define DEP_ID_WB ( ( srcRegs[ID][0] != INVAL_REG && ( srcRegs[ID][0] == dstRegs[WB][0] || srcRegs[ID][0] == dstRegs[WB][1] ) ) \
				 || ( srcRegs[ID][1] != INVAL_REG && ( srcRegs[ID][1] == dstRegs[WB][0] || srcRegs[ID][1] == dstRegs[WB][1] ) ) )


void mipsPipelined::run()
{
	while( true ) {
		step();
	}

}

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

	for (int i = WB; i > EX; --i) { 
		cmd[i]   = cmd[i-1];
		valid[i] = valid[i-1];
		srcRegs[i][0] = srcRegs[i-1][0];
		srcRegs[i][1] = srcRegs[i-1][1];
		dstRegs[i][0] = dstRegs[i-1][0];
		dstRegs[i][1] = dstRegs[i-1][1];
	}

	dependence = checkDependence();
	
	if( !dependence )
		for (int i = EX; i > IF; --i) { 
			cmd[i]   = cmd[i-1];
			valid[i] = valid[i-1];
			srcRegs[i][0] = srcRegs[i-1][0];
			srcRegs[i][1] = srcRegs[i-1][1];
			dstRegs[i][0] = dstRegs[i-1][0];
			dstRegs[i][1] = dstRegs[i-1][1];
		}
	

  //if dependance exists EX stage is always invalid
  //that is, operation located at ID stage does not progress
	if( dependence )
		valid[EX] = false;

	//execute each stage of the pipeline	
	if( valid[WB] )
		writeback();
	if( valid[MEM] )
		memory();
	if( valid[EX] )
		execute();
	if( valid[ID] )
		decode();
	fetch();

}

void mipsPipelined::fetch() {

	if( dependence ) 
		return;

	if ( pc > endAddr ) { // TODO : Add J-type case later...
		valid[IF] = false;
		return;
	}

	//set IFID intermediate register fields

	uint32_t temp = mem->loadWord( pc );
	innerRegs->IFID_setPC( temp );
	innerRegs->IFID_setNextPC( mem->loadWord( pc + 4 ) );

	//set status registers of IF stage.
	cmd[IF] = temp;
	valid[IF] = true;

	//set source and destination registers for each instruction
	//used for checking dependences
	if( OP( temp ) == RTYPE1 ) {

		switch( FUNCT( temp ) ) {

			case( MFHI ):
				srcRegs[IF][0] = HI_REG;
				srcRegs[IF][1] = INVAL_REG;
				dstRegs[IF][0] = RD( temp );
				dstRegs[IF][1] = INVAL_REG;
				break;
			
			case( MFLO ):
				srcRegs[IF][0] = LO_REG;
				srcRegs[IF][1] = INVAL_REG;
				dstRegs[IF][0] = RD( temp );
				dstRegs[IF][1] = INVAL_REG;
				break;

			case( MTHI ):
				srcRegs[IF][0] = RS( temp );
				srcRegs[IF][1] = INVAL_REG;
				dstRegs[IF][0] = HI_REG;
				dstRegs[IF][1] = INVAL_REG;
				break;

			case( MTLO ):
				srcRegs[IF][0] = RS( temp );
				srcRegs[IF][1] = INVAL_REG;
				dstRegs[IF][0] = LO_REG;
				dstRegs[IF][1] = INVAL_REG;
				break;

			case( DIV ):
			case( DIVU ):
				srcRegs[IF][0] = RS( temp );
				srcRegs[IF][1] = RT( temp );
				dstRegs[IF][0] = LO_REG;
				dstRegs[IF][1] = HI_REG;;
				break;				
			
			case( JALR ):
				srcRegs[IF][0] = RS( temp );
				srcRegs[IF][1] = INVAL_REG;
				dstRegs[IF][0] = RD( temp );
				dstRegs[IF][1] = INVAL_REG;
				break;

			case( SLL ):
			case( SRA ):
			case( SRL ):
				srcRegs[IF][0] = RS( temp );
				srcRegs[IF][1] = RT( temp );
				dstRegs[IF][0] = RD( temp );
				dstRegs[IF][1] = INVAL_REG;
				break;
			case( JR ):
				srcRegs[IF][0] = RS( temp );
				srcRegs[IF][1] = INVAL_REG;
				dstRegs[IF][0] = INVAL_REG;
				dstRegs[IF][1] = INVAL_REG;
				break;
	
			default:
				srcRegs[IF][0] = RS( temp );
				srcRegs[IF][1] = RT( temp ); 
				dstRegs[IF][0] = RD( temp );
				dstRegs[IF][1] = INVAL_REG;
				break;
		}

	} else if ( OP( temp ) == RTYPE2 ) {
		switch( FUNCT( temp ) ) {
			case( MADD ):
			case( MADDU ):
			case( MSUB ):
			case( MSUBU ):
			case( MUL ):
				srcRegs[IF][0] = RS( temp );
				srcRegs[IF][1] = RT( temp );
				dstRegs[IF][0] = LO_REG;
				dstRegs[IF][1] = HI_REG;;
				break;	

			case( CLZ ):
			case( CLO ):
				srcRegs[IF][0] = RS( temp );
				srcRegs[IF][1] = INVAL_REG;
				dstRegs[IF][0] = RD( temp );
				dstRegs[IF][1] = INVAL_REG;
				break;

			default:
				srcRegs[IF][0] = RS( temp );
				srcRegs[IF][1] = RT( temp ); 
				dstRegs[IF][0] = RD( temp );
				dstRegs[IF][1] = INVAL_REG;
				break;
		}

	} else if ( OP( temp ) == J ) {
		//do nothing
	} else if ( OP( temp ) == JAL ) {
		//do nothing	
	} else {
		switch( OP( temp ) ) {
			case( BEQ ):
			case( BNE ):
				srcRegs[IF][0] = RS( temp );
				srcRegs[IF][1] = RT( temp ); 
				dstRegs[IF][0] = INVAL_REG;
				dstRegs[IF][1] = INVAL_REG;
				break;

			case( BGEZ ): 	//also BGEZAL, BLTZ, BLTZAL
			case( BLEZ ):
			case( BGTZ ):
				srcRegs[IF][0] = RS( temp );
				srcRegs[IF][1] = INVAL_REG; 
				dstRegs[IF][0] = INVAL_REG;
				dstRegs[IF][1] = INVAL_REG;
				break;

			case( SB ):
			case( SH ):
			case( SW ):
			case( SWL ):
			case( SWR ):
				srcRegs[IF][0] = RS( temp );
				srcRegs[IF][1] = RT( temp ); 
				dstRegs[IF][0] = INVAL_REG;
				dstRegs[IF][1] = INVAL_REG;
				break;

			default:
				//All other ITYPE instructions left
				srcRegs[IF][0] = RS( temp );
				srcRegs[IF][1] = INVAL_REG; 
				dstRegs[IF][0] = RT( temp );
				dstRegs[IF][1] = INVAL_REG;
				break;

		}

	}

	pc += 4;
}


void mipsPipelined::decode() {

	if( !dependence ) {
		uint32_t temp = innerRegs->IFID_getPC();
		innerRegs->IDEX_setNextPC( innerRegs->IFID_getNextPC() );
	}

	//set fields of IDEX intermediate regiser fields
	innerRegs->IDEX_setRT( reg->getReg( RT( cmd[ID] ) ) );
	innerRegs->IDEX_setRS( reg->getReg( RS( cmd[ID] ) ) );
	innerRegs->IDEX_setImmed( IMMED( cmd[ID] ) );
	innerRegs->IDEX_setDestRegs( RD( cmd[ID] ), RT( cmd[ID] ) );
	innerRegs->IDEX_setShamt( SHAMT( cmd[ID] ) );

	//TODO: Add functionallity for J-type cases.
	innerRegs->IDEX_setLO( reg->getLO() );	
	innerRegs->IDEX_setHI( reg->getHI() );

}

bool mipsPipelined::checkDependence()
{
	if( (valid[EX] && DEP_ID_EX) || (valid[MEM] && DEP_ID_MEM) || (valid[WB] && DEP_ID_WB) )
		return true;

	return false;
}


/********************************************
 *-------------EXECUTION  STAGE-------------*
 * functionality of instruction in EX stage *
 ********************************************/


void mipsPipelined::execute() {
	uint32_t op = OP( cmd[EX] );
	if( op == RTYPE1 ) {

		//RTYPE1 operations
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
				throw "Unhandled operation EX";
		} 

	} else if ( op == RTYPE2 ) {
	
		//RTYPE2 operations
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
				throw "Unhandled operation EX";
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
				throw "Unhandled operation EX";
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
	innerRegs->EXMEM_setDestRegs( innerRegs->IDEX_getDestRegs() );
}

void mipsPipelined::executeSRL()
{
	uint32_t shamt = innerRegs->IDEX_getShamt();
	uint32_t rt = innerRegs->IDEX_getRT();
	innerRegs->EXMEM_setAluRes( rt >> shamt );
	innerRegs->EXMEM_setDestRegs( innerRegs->IDEX_getDestRegs() );
}

void mipsPipelined::executeSRA()
{
	uint32_t shamt = innerRegs->IDEX_getShamt();
	int32_t rt = (int32_t) innerRegs->IDEX_getRT();
	innerRegs->EXMEM_setAluRes( rt >> shamt );
	innerRegs->EXMEM_setDestRegs( innerRegs->IDEX_getDestRegs() );
}

void mipsPipelined::executeSLLV()
{
	uint32_t rs = innerRegs->IDEX_getRS();
	uint32_t rt = innerRegs->IDEX_getRT();
	innerRegs->EXMEM_setAluRes( rt << rs );
	innerRegs->EXMEM_setDestRegs( innerRegs->IDEX_getDestRegs() );
}

void mipsPipelined::executeSRLV()
{
	uint32_t rs = innerRegs->IDEX_getRS();
	uint32_t rt = innerRegs->IDEX_getRT();
	innerRegs->EXMEM_setAluRes( rt >> rs );
	innerRegs->EXMEM_setDestRegs( innerRegs->IDEX_getDestRegs() );
}

void mipsPipelined::executeSRAV()
{
	uint32_t rs = innerRegs->IDEX_getRS();
	int32_t rt = innerRegs->IDEX_getRT();
	innerRegs->EXMEM_setAluRes( rt >> rs );
	innerRegs->EXMEM_setDestRegs( innerRegs->IDEX_getDestRegs() );
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
	innerRegs->EXMEM_setDestRegs( innerRegs->IDEX_getDestRegs() );
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
	innerRegs->EXMEM_setDestRegs( innerRegs->IDEX_getDestRegs() );
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

	innerRegs->EXMEM_setAluRes( add(rt,rs) );
	innerRegs->EXMEM_setDestRegs( innerRegs->IDEX_getDestRegs() );
}

void mipsPipelined::executeADDU()
{
	uint32_t rs = innerRegs->IDEX_getRS();
	uint32_t rt = innerRegs->IDEX_getRT();
	
	innerRegs->EXMEM_setAluRes( rt + rs );
	innerRegs->EXMEM_setDestRegs( innerRegs->IDEX_getDestRegs()) ;
}

void mipsPipelined::executeSUB()
{
	int32_t rs = (int32_t) innerRegs->IDEX_getRS();
	int32_t rt = (int32_t) innerRegs->IDEX_getRT();

	innerRegs->EXMEM_setAluRes( subtract(rs,rt) );
	innerRegs->EXMEM_setDestRegs( innerRegs->IDEX_getDestRegs() );
}

void mipsPipelined::executeSUBU()
{
	uint32_t rs = innerRegs->IDEX_getRS();
	uint32_t rt = innerRegs->IDEX_getRT();
	
	innerRegs->EXMEM_setAluRes( rs - rt );
	innerRegs->EXMEM_setDestRegs( innerRegs->IDEX_getDestRegs() );
}

void mipsPipelined::executeAND()
{
	uint32_t rs = innerRegs->IDEX_getRS();
	uint32_t rt = innerRegs->IDEX_getRT();
	
	innerRegs->EXMEM_setAluRes( rs & rt );
	innerRegs->EXMEM_setDestRegs( innerRegs->IDEX_getDestRegs() );
}

void mipsPipelined::executeOR()
{
	uint32_t rs = innerRegs->IDEX_getRS();
	uint32_t rt = innerRegs->IDEX_getRT();
	
	innerRegs->EXMEM_setAluRes( rs | rt );
	innerRegs->EXMEM_setDestRegs( innerRegs->IDEX_getDestRegs() );
}

void mipsPipelined::executeXOR()
{
	uint32_t rs = innerRegs->IDEX_getRS();
	uint32_t rt = innerRegs->IDEX_getRT();
	
	innerRegs->EXMEM_setAluRes( rs ^ rt );
	innerRegs->EXMEM_setDestRegs( innerRegs->IDEX_getDestRegs() );	
}

void mipsPipelined::executeNOR()
{
	uint32_t rs = innerRegs->IDEX_getRS();
	uint32_t rt = innerRegs->IDEX_getRT();
	
	innerRegs->EXMEM_setAluRes( ~(rs | rt) );
	innerRegs->EXMEM_setDestRegs( innerRegs->IDEX_getDestRegs() );
}


void mipsPipelined::executeSLT()
{
	int32_t rs = (int32_t) innerRegs->IDEX_getRS();
	int32_t rt = (int32_t) innerRegs->IDEX_getRT();
	
	innerRegs->EXMEM_setAluRes( (rs<rt) ? 1U : 0 );
	innerRegs->EXMEM_setDestRegs( innerRegs->IDEX_getDestRegs() );
}

void mipsPipelined::executeSLTU()
{
	uint32_t rs = innerRegs->IDEX_getRS();
	uint32_t rt = innerRegs->IDEX_getRT();
	
	innerRegs->EXMEM_setAluRes( (rs<rt) ? 1U : 0  );
	innerRegs->EXMEM_setDestRegs( innerRegs->IDEX_getDestRegs() );
}


void mipsPipelined::executeMADD()
{
	//TODO -> Tsekare an einai swsta ta orismata signed - unsigned (dimitris)
	int32_t rs = (int32_t) innerRegs->IDEX_getRS();
	int32_t rt = (int32_t) innerRegs->IDEX_getRT();
	uint64_t lo = (uint64_t) innerRegs->IDEX_getLO();
	uint64_t hi =  ( (uint64_t) innerRegs->IDEX_getHI() ) << 32  ;

	int64_t result = (int64_t) rs * rt;
	int64_t hi_lo = ( hi | lo );
	result += hi_lo;

	innerRegs->EXMEM_setAluRes(result && 0xffffffff);  // Stores into that register the result for LO
	innerRegs->EXMEM_setAluRes2((result >> 32 ) && 0xffffffff); // Stores into that register the result for HI
}

void mipsPipelined::executeMADDU()
{
	uint32_t rs = innerRegs->IDEX_getRS();
	uint32_t rt = innerRegs->IDEX_getRT();
	uint64_t lo = (uint64_t) innerRegs->IDEX_getLO();
	uint64_t hi =  ( (uint64_t) innerRegs->IDEX_getHI() ) << 32  ;

	uint64_t result = rs * rt;
	uint64_t hi_lo = ( hi | lo );
	result += hi_lo;

	innerRegs->EXMEM_setAluRes(result && 0xffffffff);  // Stores into that register the result for LO
	innerRegs->EXMEM_setAluRes2((result >> 32 ) && 0xffffffff); // Stores into that register the result for HI
}

void mipsPipelined::executeMUL()
{
	int32_t rs = (int32_t) innerRegs->IDEX_getRS();
	int32_t rt = (int32_t) innerRegs->IDEX_getRT();

	int64_t result = rs * rt;

	innerRegs->EXMEM_setDestRegs( innerRegs->IDEX_getDestRegs() );
	innerRegs->EXMEM_setAluRes(result && 0xffffffff);  // Stores into that register the result for register rd
}

void mipsPipelined::executeMSUB()
{
	//TODO -> Tsekare an einai swsta ta orismata signed - unsigned (dimitris)

	int32_t rs = (int32_t) innerRegs->IDEX_getRS();
	int32_t rt = (int32_t) innerRegs->IDEX_getRT();
	uint64_t lo = (uint64_t) innerRegs->IDEX_getLO();
	uint64_t hi =  ( (uint64_t) innerRegs->IDEX_getHI() ) << 32  ;

	int64_t result = (int32_t) rs * rt;
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
	uint32_t count = 0;

	__asm__("again1:");
	rs<<1;
	__asm__("jc stop1");
	count++;
	__asm__("jmp again1");
	__asm__("stop1:");

	innerRegs->EXMEM_setDestRegs(innerRegs->IDEX_getDestRegs() );
	innerRegs->EXMEM_setAluRes( count ); // Stores into that register the result for register rd
}

void mipsPipelined::executeCLO()
{
	uint32_t rs = innerRegs->IDEX_getRS();
	uint32_t count = 0;

	__asm__("again2:");
	rs<<1;
	__asm__("jnc stop2");
	count++;
	__asm__("jmp again2");
	__asm__("stop2:");

	innerRegs->EXMEM_setDestRegs(innerRegs->IDEX_getDestRegs() );
	innerRegs->EXMEM_setAluRes( count );
}

void mipsPipelined::executeMOVZ()
{
	uint32_t rs = innerRegs->IDEX_getRS();
	uint32_t rt = innerRegs->IDEX_getRT();

	innerRegs->EXMEM_setAluRes( (rt == 0) ? 1U :0 ) ; // Stores the comparison's result, in order to have it at WB stage
	innerRegs->EXMEM_setAluRes2(rs); 				// Stores rs' value in order to have it at WB stage.
	innerRegs->EXMEM_setDestRegs(innerRegs->IDEX_getDestRegs()) ;
}

void mipsPipelined::executeMOVN()
{
	uint32_t rs = innerRegs->IDEX_getRS();
	uint32_t rt = innerRegs->IDEX_getRT();

	innerRegs->EXMEM_setAluRes( (rt != 0) ? 1U :0 ) ; // Stores the comparison's result, in order to have it at WB stage
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
	innerRegs->EXMEM_setAluRes( add( signExtend( (int16_t) innerRegs->IDEX_getImmed() ), rs ) );
	innerRegs->EXMEM_setDestRegs( innerRegs->IDEX_getDestRegs() );
}

void mipsPipelined::executeADDIU()
{
	uint32_t rs = innerRegs->IDEX_getRS();
	innerRegs->EXMEM_setAluRes( rs + signExtend( (int16_t) innerRegs->IDEX_getImmed() ) );
	innerRegs->EXMEM_setDestRegs( innerRegs->IDEX_getDestRegs() );
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
	innerRegs->EXMEM_setAluRes( ( rs ^ (uint32_t) innerRegs->IDEX_getImmed() ) );
	innerRegs->EXMEM_setDestRegs(innerRegs->IDEX_getDestRegs() );
}

void mipsPipelined::executeLUI()
{
	uint32_t res = (uint32_t) innerRegs->IDEX_getImmed();
	innerRegs->EXMEM_setAluRes( res << 16 );
	innerRegs->EXMEM_setDestRegs(innerRegs->IDEX_getDestRegs() );
}

void mipsPipelined::executeLB()
{
	int32_t base = innerRegs->IDEX_getRS();
	int32_t offset = (int32_t) signExtend( (int16_t) innerRegs->IDEX_getImmed() );
	innerRegs->EXMEM_setAluRes( base + offset );  // pass byte's address at EXMEM register
	innerRegs->EXMEM_setDestRegs( innerRegs->IDEX_getDestRegs() ); 
}

void mipsPipelined::executeLH()
{
	int32_t base = innerRegs->IDEX_getRS();
	int32_t offset = (int32_t) signExtend( (int16_t) innerRegs->IDEX_getImmed() );
	innerRegs->EXMEM_setAluRes( base + offset );  // pass halfword's address at EXMEM register
	innerRegs->EXMEM_setDestRegs( innerRegs->IDEX_getDestRegs() ); 
}

void mipsPipelined::executeLWL()
{
	int32_t base = innerRegs->IDEX_getRS();
	int32_t offset = (int32_t) signExtend( (int16_t) innerRegs->IDEX_getImmed() );
	innerRegs->EXMEM_setAluRes( base + offset );  // pass word's address at EXMEM register
	innerRegs->EXMEM_setDestRegs(innerRegs->IDEX_getDestRegs() ); 
}


void mipsPipelined::executeLW()
{
	int32_t base = innerRegs->IDEX_getRS();
	int32_t offset = (int32_t) signExtend( (int16_t) innerRegs->IDEX_getImmed() );
	innerRegs->EXMEM_setAluRes( base + offset );  // pass word's address at EXMEM register
	innerRegs->EXMEM_setDestRegs(innerRegs->IDEX_getDestRegs() ); 
}


void mipsPipelined::executeLBU()
{
	int32_t base = innerRegs->IDEX_getRS();
	int32_t offset = (int32_t) signExtend( (int16_t) innerRegs->IDEX_getImmed() );
	innerRegs->EXMEM_setAluRes( base + offset );  // pass byte's address at EXMEM register
	innerRegs->EXMEM_setDestRegs(innerRegs->IDEX_getDestRegs() ); 
}


void mipsPipelined::executeLHU()
{
	int32_t base = innerRegs->IDEX_getRS();
	int32_t offset = (int32_t) signExtend( (int16_t) innerRegs->IDEX_getImmed() );
	innerRegs->EXMEM_setAluRes( base + offset );  // pass halfword's address at EXMEM register
	innerRegs->EXMEM_setDestRegs(innerRegs->IDEX_getDestRegs() ); 
}


void mipsPipelined::executeLWR()
{
	int32_t base = innerRegs->IDEX_getRS();
	int32_t offset = (int32_t) signExtend( (int16_t) innerRegs->IDEX_getImmed() );
	innerRegs->EXMEM_setAluRes( base + offset );  // pass words's address at EXMEM register
	innerRegs->EXMEM_setDestRegs(innerRegs->IDEX_getDestRegs() ); 
}

void mipsPipelined::executeSB()
{
	int32_t base = innerRegs->IDEX_getRS();
	int32_t offset = (int32_t) signExtend( (int16_t) innerRegs->IDEX_getImmed() );
	innerRegs->EXMEM_setAluRes( base + offset );  // pass byte's address at EXMEM register
	uint32_t rt = innerRegs->IDEX_getRT();
	innerRegs->EXMEM_setStoreData( rt && 0xff );
}

void mipsPipelined::executeSH()
{
	int32_t base = innerRegs->IDEX_getRS();
	int32_t offset = (int32_t) signExtend( (int16_t) innerRegs->IDEX_getImmed() );
	innerRegs->EXMEM_setAluRes( base + offset );  // pass byte's address at EXMEM register
	uint32_t rt = innerRegs->IDEX_getRT();
	innerRegs->EXMEM_setStoreData( rt && 0xffff );
}

void mipsPipelined::executeSWL()
{
	int32_t base = innerRegs->IDEX_getRS();
	int32_t offset = (int32_t) signExtend( (int16_t) innerRegs->IDEX_getImmed() );
	innerRegs->EXMEM_setAluRes( base + offset );  // pass byte's address at EXMEM register
	uint32_t rt = innerRegs->IDEX_getRT();
	innerRegs->EXMEM_setStoreData( rt );
}

void mipsPipelined::executeSW()
{
	int32_t offset = (int32_t) signExtend( (int16_t) innerRegs->IDEX_getImmed() );
	int32_t base = innerRegs->IDEX_getRS();
	innerRegs->EXMEM_setAluRes( base + offset );  
	uint32_t rt = innerRegs->IDEX_getRT();
	innerRegs->EXMEM_setStoreData( rt );
}

void mipsPipelined::executeSWR()
{
	int32_t base = innerRegs->IDEX_getRS();
	int32_t offset = (int32_t) signExtend( (int16_t) innerRegs->IDEX_getImmed() );
	innerRegs->EXMEM_setAluRes( base + offset );  // pass byte's address at EXMEM register
	uint32_t rt = innerRegs->IDEX_getRT();
	innerRegs->EXMEM_setStoreData( rt );
}

void mipsPipelined::executeLL()
{
	int32_t base = innerRegs->IDEX_getRS();
	int32_t offset = (int32_t) signExtend( (int16_t) innerRegs->IDEX_getImmed() );
	innerRegs->EXMEM_setAluRes( base + offset );  // pass byte's address at EXMEM register
	innerRegs->EXMEM_setDestRegs(innerRegs->IDEX_getDestRegs() ); 
}

void mipsPipelined::executeSC()
{
	int32_t addr = (int32_t) signExtend( (int16_t) innerRegs->IDEX_getImmed() );
	innerRegs->EXMEM_setAluRes( addr );  
	uint32_t rt = innerRegs->IDEX_getRT();
	innerRegs->EXMEM_setStoreData( rt );
}


/*********************************************
 *---------------MEMORY STAGE----------------*
 * functionality of instruction in MEM stage *
 ********************************************/

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
				throw "Unhandled operation MEM";
		}
			
	} else if ( op == RTYPE2 ) {
	
		//RTYPE2 operations
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
				throw "Unhandled operation MEM";
		}

	} else if ( op == J ) { //the two JTYPE operations are following
		
	} else if ( op == JAL ) {

	// TODO -> simplirwse J kai JAL


	} else { //ITYPE here

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
			default:
				throw "Unhandled operation MEM";
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
		res = res | ( ( (uint32_t) mem->loadByte( addr ) ) << ( (4-i)*8 ) ); 
		addr++;
	}
	innerRegs->MEMWB_setMem( res );
	innerRegs->MEMWB_setDestRegs( innerRegs->EXMEM_getDestRegs() );
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
	for( int i=0; i<bytes; ++i ) {
		res = res | ( mem->loadByte( addr ) << (i*8) );
		addr--;
	}
	innerRegs->MEMWB_setMem( res );
	innerRegs->MEMWB_setDestRegs( innerRegs->EXMEM_getDestRegs() );
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
	uint32_t addr = innerRegs->EXMEM_getAluRes();
	uint32_t data = innerRegs->EXMEM_getStoreData();
	uint8_t bytes = 4 - addr%4;
	for( int i=1; i<=bytes; ++i ) {
		mem->storeByte( addr, ( data >> ((4-i)*8) ) & 0xff );
		addr++;
	}
}
void mipsPipelined::memorySW()
{
	uint32_t addr = innerRegs->EXMEM_getAluRes();
	uint32_t data = innerRegs->EXMEM_getStoreData();
	mem->storeWord( addr,data );
}
void mipsPipelined::memorySWR()
{
	uint32_t addr = innerRegs->EXMEM_getAluRes();
	uint32_t data = innerRegs->EXMEM_getStoreData();
	uint8_t bytes = 1 + addr%4;
	for( int i=0; i<bytes; ++i ) {
		mem->storeByte( addr, ( data >> (i*8) ) & 0xff );
		addr--;
	}
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

		//RTYPE1 operations
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
				throw "Unhandled operation WB";
		} 

	} else if ( op == RTYPE2 ) {
	
		//RTYPE2 operations
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
				throw "Unhandled operation WB";
		}

	} else if ( op == J ) { //the two JTYPE operations are following
		
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
			case( SB ): writebackSB(); break;
			case( SH ): writebackSH(); break;
			case( SWL ): writebackSWL(); break;
			case( SW ): writebackSW(); break;
			case( SWR ): writebackSWR(); break;
			default:
				throw "Unhandled operation WB";
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

den tis eixe kai o mpampis epeidi den kanoun tpt sto WB
alla rwta teammates an toulaxiston eksasfalizw orthotita

*/


void mipsPipelined::writebackSLL()
{
	reg->setReg( innerRegs->MEMWB_getDestRegRD(), innerRegs->MEMWB_getAlu() );
}

void mipsPipelined::writebackSRL()
{
	reg->setReg( innerRegs->MEMWB_getDestRegRD(), innerRegs->MEMWB_getAlu() );
}

void mipsPipelined::writebackSRA()
{	
	reg->setReg( innerRegs->MEMWB_getDestRegRD(), innerRegs->MEMWB_getAlu() );
}

void mipsPipelined::writebackSLLV()
{
	reg->setReg( innerRegs->MEMWB_getDestRegRD(), innerRegs->MEMWB_getAlu() );
}

void mipsPipelined::writebackSRLV()
{
	reg->setReg( innerRegs->MEMWB_getDestRegRD(), innerRegs->MEMWB_getAlu() );
}

void mipsPipelined::writebackSRAV()
{
	reg->setReg( innerRegs->MEMWB_getDestRegRD(), innerRegs->MEMWB_getAlu() );
}

void mipsPipelined::writebackMFHI()
{
	reg->setReg( innerRegs->MEMWB_getDestRegRD(), innerRegs->MEMWB_getAlu() );
}

void mipsPipelined::writebackMTHI()
{
	reg->setHI( innerRegs->MEMWB_getAlu() );
}

void mipsPipelined::writebackMFLO()
{
	reg->setReg( innerRegs->MEMWB_getDestRegRD(), innerRegs->MEMWB_getAlu() );
}

void mipsPipelined::writebackMTLO()
{
	reg->setLO( innerRegs->MEMWB_getAlu() );
}

void mipsPipelined::writebackMULT()
{
	reg->setLO( innerRegs->MEMWB_getAlu() );
	reg->setHI( innerRegs->MEMWB_getAlu2() );
}

void mipsPipelined::writebackMULTU()
{
	reg->setLO( innerRegs->MEMWB_getAlu() );
	reg->setHI( innerRegs->MEMWB_getAlu2() );
}

void mipsPipelined::writebackDIV()
{
	reg->setLO( innerRegs->MEMWB_getAlu() );
	reg->setHI( innerRegs->MEMWB_getAlu2() );

}

void mipsPipelined::writebackDIVU()
{
	reg->setLO( innerRegs->MEMWB_getAlu() );
	reg->setHI( innerRegs->MEMWB_getAlu2() );
}

void mipsPipelined::writebackADD()
{
	reg->setReg( innerRegs->MEMWB_getDestRegRD(), innerRegs->MEMWB_getAlu() );
}

void mipsPipelined::writebackADDU()
{
	reg->setReg( innerRegs->MEMWB_getDestRegRD(), innerRegs->MEMWB_getAlu() );
}

void mipsPipelined::writebackSUB()
{
	reg->setReg( innerRegs->MEMWB_getDestRegRD(), innerRegs->MEMWB_getAlu() );
}

void mipsPipelined::writebackSUBU()
{
	reg->setReg( innerRegs->MEMWB_getDestRegRD(), innerRegs->MEMWB_getAlu() );
}

void mipsPipelined::writebackAND()
{
	reg->setReg( innerRegs->MEMWB_getDestRegRD(), innerRegs->MEMWB_getAlu() );
}

void mipsPipelined::writebackOR()
{
	reg->setReg( innerRegs->MEMWB_getDestRegRD(), innerRegs->MEMWB_getAlu() );
}

void mipsPipelined::writebackXOR()
{
	reg->setReg( innerRegs->MEMWB_getDestRegRD(), innerRegs->MEMWB_getAlu() );
}

void mipsPipelined::writebackNOR()
{
	reg->setReg( innerRegs->MEMWB_getDestRegRD(), innerRegs->MEMWB_getAlu() );
}

void mipsPipelined::writebackSLT()
{
	reg->setReg( innerRegs->MEMWB_getDestRegRD(), innerRegs->MEMWB_getAlu() );
}

void mipsPipelined::writebackSLTU()
{
	reg->setReg( innerRegs->MEMWB_getDestRegRD(), innerRegs->MEMWB_getAlu() );
}

void mipsPipelined::writebackMADD()
{
	reg->setLO( innerRegs->MEMWB_getAlu() );
	reg->setHI( innerRegs->MEMWB_getAlu2() );

}

void mipsPipelined::writebackMADDU()
{
	reg->setLO( innerRegs->MEMWB_getAlu() );
	reg->setHI(  innerRegs->MEMWB_getAlu2() );
}

void mipsPipelined::writebackMUL()
{
	reg->setReg( innerRegs->MEMWB_getDestRegRD(), innerRegs->MEMWB_getAlu() );
}

void mipsPipelined::writebackMSUB()
{
	reg->setLO( innerRegs->MEMWB_getAlu() );
	reg->setHI( innerRegs->MEMWB_getAlu2() );
}


void mipsPipelined::writebackMSUBU()
{
	reg->setLO( innerRegs->MEMWB_getAlu() );
	reg->setHI( innerRegs->MEMWB_getAlu2() );
}

void mipsPipelined::writebackCLZ()
{
	reg->setReg( innerRegs->MEMWB_getDestRegRD(), innerRegs->MEMWB_getAlu() );
	
}

void mipsPipelined::writebackCLO()
{
	reg->setReg( innerRegs->MEMWB_getDestRegRD(), innerRegs->MEMWB_getAlu() );
}


void mipsPipelined::writebackMOVZ()
{
	if ( innerRegs->MEMWB_getAlu() == 1 )
		reg->setReg( innerRegs->MEMWB_getDestRegRD(), innerRegs->MEMWB_getAlu2() );
}

void mipsPipelined::writebackMOVN()
{
	if ( innerRegs->MEMWB_getAlu() == 1 )
		reg->setReg( innerRegs->MEMWB_getDestRegRD(), innerRegs->MEMWB_getAlu2() );
}

void mipsPipelined::writebackADDI()
{
	reg->setReg( innerRegs->MEMWB_getDestRegRT(), innerRegs->MEMWB_getAlu() );
}

void mipsPipelined::writebackADDIU()
{
	reg->setReg( innerRegs->MEMWB_getDestRegRT(), innerRegs->MEMWB_getAlu() );
}

void mipsPipelined::writebackSLTI()
{
	reg->setReg( innerRegs->MEMWB_getDestRegRT(), innerRegs->MEMWB_getAlu() );
}

void mipsPipelined::writebackSLTIU()
{
	reg->setReg( innerRegs->MEMWB_getDestRegRT(), innerRegs->MEMWB_getAlu() );
}

void mipsPipelined::writebackANDI()
{
	reg->setReg( innerRegs->MEMWB_getDestRegRT(), innerRegs->MEMWB_getAlu() );
}

void mipsPipelined::writebackORI()
{
	reg->setReg( innerRegs->MEMWB_getDestRegRT(), innerRegs->MEMWB_getAlu() );
}

void mipsPipelined::writebackXORI()
{
	reg->setReg( innerRegs->MEMWB_getDestRegRT(), innerRegs->MEMWB_getAlu() );
}


void mipsPipelined::writebackLUI()
{
	reg->setReg( innerRegs->MEMWB_getDestRegRT(), innerRegs->MEMWB_getAlu() );
}

void mipsPipelined::writebackLB()
{
	reg->setReg( innerRegs->MEMWB_getDestRegRT(), innerRegs->MEMWB_getMem() );
}

void mipsPipelined::writebackLH()
{
	reg->setReg( innerRegs->MEMWB_getDestRegRT(), innerRegs->MEMWB_getMem() );
}

void mipsPipelined::writebackLWL() 
{
	reg->setReg( innerRegs->MEMWB_getDestRegRT(), innerRegs->MEMWB_getMem() );
}

void mipsPipelined::writebackLW()
{
	reg->setReg( innerRegs->MEMWB_getDestRegRT(), innerRegs->MEMWB_getMem() );
}

void mipsPipelined::writebackLBU()
{
	reg->setReg( innerRegs->MEMWB_getDestRegRT(), innerRegs->MEMWB_getMem() );
}

void mipsPipelined::writebackLHU()
{
	reg->setReg( innerRegs->MEMWB_getDestRegRT(), innerRegs->MEMWB_getMem() );

}

void mipsPipelined::writebackLWR()
{
	reg->setReg( innerRegs->MEMWB_getDestRegRT(), innerRegs->MEMWB_getMem() );
}

void mipsPipelined::writebackLL() 
{
	reg->setReg( innerRegs->MEMWB_getDestRegRT(), innerRegs->MEMWB_getMem() );
}

void mipsPipelined::writebackSC()
{
	reg->setReg( innerRegs->MEMWB_getDestRegRT(), innerRegs->MEMWB_getAlu() );
}

/*
 * Functions below this point are trivial 'cause
 * they do nothing during WB stage
 */
void mipsPipelined::writebackSB(){}
void mipsPipelined::writebackSH(){}
void mipsPipelined::writebackSWR(){}
void mipsPipelined::writebackSW(){}
void mipsPipelined::writebackSWL(){}
