/*
 * processor.cpp
 * This is an implementation of our MIPS
 * processor.
 */

#include <iostream>
#include <iomanip>
#include <sstream>
#include "processor.h"
#include "safeops.h"

using namespace std;

void simpleProcessor::reset() 
{
	reg->reset();
}

void simpleProcessor::setTextArea( uint32_t startAddr, uint32_t endAddr )
{
	this->startAddr = startAddr;
	this->endAddr = endAddr;
}

void simpleProcessor::step()
{
	stringstream ex;
	
	//set console output to represent numbers as hexademicals with 0x in front of them.
	ex.setf( ios::hex, ios::basefield );
	ex.setf( ios::showbase );
	if( pc < startAddr || pc > endAddr ) {
			ex << "Error: pc out of range" << pc << endl;
			throw ex.str();
	}

	//if pc is in range load next instruction and execute it
	uint32_t cmd = mem->loadWord( pc );
	executeCmd( cmd );

	//update pc
	pc += 4;

}

void simpleProcessor::run()
{
	stringstream ex;
	
	//set console output to represent numbers as hexademicals with 0x in front of them.
	ex.setf( ios::hex, ios::basefield );
	ex.setf( ios::showbase );
	while( true ) {

		if( pc < startAddr || pc > endAddr ) {
			ex << "Error: pc out of range" << pc << endl;
			throw ex.str();
		}

		//if pc is in range load next instruction and execute it
		uint32_t cmd = mem->loadWord( pc );
		if( !executeCmd( cmd ) ) {
			ex << "Unknown RTYPE operation at pc " << pc << ". Funct " << FUNCT( cmd ); 
			throw ex.str();
		}
		//update pc
		pc += 4;
	}

}

int32_t simpleProcessor::signExtend( uint16_t halfword )
{
	if( halfword & 0x8000 )
		return 0xffff0000 | halfword;
	else
		return 0x0000ffff & halfword;
}

int32_t simpleProcessor::signExtend( uint8_t byte )
{
	if( byte & 0x80 )
		return 0xffffff00 | byte;
	else
		return 0x000000ff & byte;

}

bool simpleProcessor::executeCmd( uint32_t cmd )
{
	if( OP( cmd ) == RTYPE1 ) {
	
		//R-TYPE instructions
		int32_t rd =  RD(cmd);
		int32_t rs =  reg->getReg( RS(cmd) );
		int32_t rt =  reg->getReg( RT(cmd) );
		int32_t shamt = SHAMT( cmd );

		switch( FUNCT( cmd ) ) {
			case( ADD ):
				reg->setReg( rd, add( rs, rt ) );
				//what about overflow?
				break;
		
			case( ADDU ):
				reg->setReg( rd, add( rs, rt ) );
				//what about overflow?
				break;
		
			case( AND ):
				reg->setReg( rd, rs & rt );
		
			case( BREAK ):
				throw "BREAK unimplemented";
			
			case( DIV ):
				reg->setHI( rs / rt );
				reg->setLO( rs % rt );
				break;
			
			case( DIVU ):
				reg->setHI( (uint32_t) rs / rt );
				reg->setLO( (uint32_t) rs % rt );
				break;
			case( JALR ):
				reg->setReg( 31, pc+4 );
				pc = rs - 4;
				break;
			case( JR ):
				pc = rs - 4;
				break;

			case( MFHI ):
				reg->setReg( rd, reg->getHI() );
				break;

			case( MFLO ):
				reg->setReg( rd, reg->getLO() );
				break;

			case( MTHI ):
				reg->setHI( rd );
				break;
		
			case( MTLO ):
				reg->setLO( rd );
				break;

			case( MULT ):
				reg->setLO( multiply( rs,rt ) );
				break;

			case( MULTU ):
				reg->setLO( multiplyUnsigned( rs,rt ) ); 
				break;

			case( NOR ):
				reg->setReg( rd, ~( rs | rt ) );
				break;
	
			case( OR ):
				reg->setReg( rd, rs|rt );
				break;

			case( XOR ):
				reg->setReg( rd, rs^rt );
				break;

			case( SLL ):
				reg->setReg( rd, (uint32_t)rs << shamt );
				break;

			case( SLLV ):
				reg->setReg( rd, (uint32_t)rs << rt );
				break;

			case( SRL ):
				reg->setReg( rd, (uint32_t)rs >> shamt );
				break;

			case( SRLV ):
				reg->setReg( rd, (uint32_t)rs >> rt );
				break;

			case( SLT ):
				if( rs < rt ) 
					reg->setReg( rd, 1 );
				else
					reg->setReg( rd, 0 );
				break;

			case( SLTU ):
				if( (uint32_t) rs < (uint32_t) rt )
					reg->setReg( rd,1 );
				else
					reg->setReg( rd, 0 );
				break;

			case( SRA ):
				reg->setReg( rd, rs >> shamt );
				break;

			case( SUB ):
				reg->setReg( rd, subtract( rs, rt ) );
				break;

			case( SUBU ):
				reg->setReg( rd, subtract( rs, rt ) );
				break;

			default:
				return false;	
		}
	
	//next two instructions are JTYPE instructions
	} else if ( OP(cmd) == J ){
		int32_t target = TARG( cmd );
		pc = ( ( pc & 0xf0000000 ) | ( target << 2 ) ) - 4;
			
	} else if ( OP(cmd) == JAL ) {	
		//diafora me to niko
		int32_t target = TARG( cmd );
		reg->setReg( 31, pc + 8 );
		pc = ( ( pc & 0xf0000000 ) | ( target << 2 ) ) - 4;
			
	//and all next are ITYPE
	} else {
		int32_t rs = reg->getReg( RS( cmd ) );
		int32_t rt = RT( cmd );
		int32_t immed = IMMED( cmd );

		switch( OP(cmd) ) {
			case( ADDI ):
				reg->setReg( rt, add( rs, immed ) );
				break;

			case( ADDIU ):
				reg->setReg( rt, add( rs, immed ) );
				break;

			case( ANDI ):
				reg->setReg( rt, rs & immed );
				break;

			case( BEQ ):
				if( reg->getReg( rt ) == rs )
					pc += ( immed << 2 ) - 4;
				break;
			
			case( BGEZ ):		//BGEZ,	BGEZAL, BLTZAL, BLTZ all have opcode 0x01
				switch( rt ) {
					case( 1 ):
						//BGEZ
						break;
					case( 0x11 ):
						//BGEZAL
						break;
					case( 0x10 ):
						//BLTZAL
						break;
					case( 0 ):
						//BLTZ
						break;

				}

				break;

			case( BGTZ ):
				if( rt == 0 ) { //rt field must be 0

				}
				break;

			case( BLEZ ):
				if( rt == 0 ) { //rt field must be 0

				}
				break;

			case( BNE ):
				if( reg->getReg( rt ) != rs )
					pc += ( immed << 2 ) - 4;
				break;

			case( LB ):
				reg->setReg( rt, signExtend( (uint8_t) mem->loadByte( rs + signExtend( (uint16_t) immed ) ) ) ); //Sign Extension
				break;
	
			case( LBU ):
				reg->setReg( rt, mem->loadByte( rs + signExtend( (uint16_t) immed ) ) ); //TODO: Byte returned from mem must be sign extended.
				break; 

			case( LH ):
				reg->setReg( rt, signExtend( (uint16_t) mem->loadHalfWord( rs + signExtend( (uint16_t) immed ) ) ) );
				break;

			case( LHU ):
				reg->setReg( rt, mem->loadHalfWord( rs + signExtend( (uint16_t) immed ) ) );
				break; 

			case( LUI ):
				reg->setReg( rt, immed << 16 );
				break;
			
			case( LW ):
				reg->setReg( rt, mem->loadWord( rs + immed ) );
				break;
			
			case( ORI ):
				reg->setReg( rt, rs | immed );
				break; 

			case( SB ):
				mem->storeByte( rs+immed, reg->getReg( rt ) );
				break;
				
			case( SLTI ):
				if(  rs < immed )
					reg->setReg( rt, 1 );
				else
					reg->setReg( rt, 0 );
				break;

			case( SLTIU ):
				if( (uint32_t) rs < (uint32_t) immed )
					reg->setReg( rt,1 );
				else 
					reg->setReg( rt, 0 );
				break;

			case( SH ): 
				mem->storeHalfWord( rs+immed, reg->getReg( rt ) );
				break;

			case( SW ):
				mem->storeWord( rs+immed, reg->getReg( rt ) );
				break;

			case( XORI ):
				reg->setReg( rt, rs^immed );
				break;

			default:
				return false;

		}	

	}
	return true;
}
