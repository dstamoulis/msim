#include <stdio.h>
#include "pipelineRegisters.h"
#include <iostream>



int main() {
	intermediateRegisters regs;

	regs.IDEX_setDestRegs( 0x1f, 0x1 );
	printf( "%x\n", regs.IDEX_getDestRegs() );
	
	return 0;

}
