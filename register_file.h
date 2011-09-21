/*
 *	register_file.h
 * 
 * Class containing registers 
 * and functions manipulating them
 */


#ifndef __REGISTER_FILE_H__
#define __REGISTER_FILE_H__

#include <stdint.h>
#include <cstring>

//number of registers
#define REG_NR 32
#define STACK_MAX 0x7ffffffc
#define GLOBAL_INIT 0x10008000

#define INVAL_REG -1
#define LO_REG 32
#define HI_REG 33

class Register_File {

public:
	//Here goes the interface of the Register_File class

	//constructors
	Register_File(); 
	
	//register handlers
	int32_t getReg( unsigned int ) const;
	void setReg( unsigned int, int32_t );

	//HI, LO registers
	int32_t getHI() const { return HI; }
	void setHI( int32_t val ) { HI = val; }

	int32_t getLO() const { return LO; }
	void setLO( int32_t val ) { LO = val; }; 

	//EX register
	void clearEX() { EX = 0; }
	int32_t getEX() const { return EX; }
	void setEX( int32_t val ) { EX = val; }
	
	//BRK register
	int32_t getBRK() { return BRK; }
	void setBRK( int32_t val ) { BRK = val; }

	//reset Register File. Restore default values in registers;
	void reset();

private:
	//..and here its implementation
	int32_t registers[ REG_NR ];
	int32_t HI, LO;
	int32_t EX;		//exceptions bitmap		?? maybe uint32_t
	int32_t BRK;	//determines the upper limit of the current data segment.
};

#endif /* __REGISTER_FILE__ */
