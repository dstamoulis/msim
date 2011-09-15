/*
 * mipsISA.h
 * definition of all MIPS's instruction opcodes
 */

#ifndef _MIPSISA_H
#define _MIPSISA_H

//all opcodes
typedef enum {
	//all R-TYPE instructions have opcode 0000 00
	RTYPE1 = 0x00,
	RTYPE2 = 0X1C,

	//I-TYPE
	BGEZ = 0x01,
	BGEZAL = 0x01,
	BLTZAL = 0x01,
	BLTZ = 0x01,
	BEQ = 0x04,
	BNE = 0x05,
	BLEZ = 0x06,
	BGTZ = 0x07,
	ADDI = 0x08,
	ADDIU = 0x09,
	SLTI = 0x0A,
	SLTIU = 0x0B,
	ANDI = 0x0C,
	ORI = 0x0D,
	XORI = 0x0E,
	LUI = 0x0F,
	LB = 0x20,
	LH = 0x21,
	LW = 0x23,
	LBU = 0x24,
	LHU = 0x25,
	SB = 0x28,
	SH = 0x29,
	SW = 0x2B,
	LWC1 = 0x31,
	SWC1 = 0x39,

	//J-TYPE
	J = 0x02,
	JAL = 0x03

} opcode;


/*
 * function codes. Only matter in case of
 * R-TYPE instructions
 */
typedef enum {
	//those with opcode 0x00
	SLL = 0x00,
	SRL = 0x02,
	SRA = 0x03,
	SLLV = 0x04,
	SRLV = 0x06,
	SRAL = 0x07,
	JR = 0x08,
	JALR = 0x09,
	BREAK = 0x0D,
	MFHI = 0x10,
	MTHI = 0x11,
	MFLO = 0x12,
	MTLO = 0x13,
	MULT = 0x18,
	MULTU = 0x19,
	DIV = 0x1A,
	DIVU = 0x1B,
	ADD = 0X20,
	ADDU = 0x21,
	SUB = 0x22,
	SUBU = 0x23,
	AND = 0x24,
	OR	= 0x25,
	XOR = 0x26,
	NOR = 0x27,
	SLT = 0x2A,
	SLTU = 0x2B,

	//those with opcode 0x1c
	MADD = 0x00,
	MADDU = 0x01,
	MUL = 0x02,
	MSUB = 0x04,
	MSUBU = 0x05,
	CLZ = 0x20,
	CLO = 0x21
} function;


#endif
