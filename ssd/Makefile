all: JN5121 JN5139R JN5139R1 shawn pacemate
	

JN5121:
	@echo "----- Building for JENNIC 5121 target ------"
	@make JENNIC_CHIP=JN5121  -s -f Makefile.jennic
	@echo "----- Done  ------"

JN5139R:
	@echo "----- Building for JENNIC 5139R target ------"
	@make JENNIC_CHIP=JN5139R -s -f Makefile.jennic
	@echo "----- Done  ------"

JN5139R1:
	@echo "----- Building for JENNIC 5139R1 target ------"
	@make JENNIC_CHIP=JN5139R1 -f Makefile.jennic
	@echo "----- Done  ------"
	
JN5148:
	@echo "----- Building for JENNIC 5148 target ------"
	@make JENNIC_CHIP=JN5148 -j4 -s -f Makefile.jennic.5148
	@echo "----- Done  ------"

shawn:
	@echo "----- Building for SHAWN target  ------"
	@make -s -f Makefile.shawn  
	@echo "----- Done  ------"

pacemate:
	@echo "----- Building for PACEMATE target  ------"
	@make -s -f Makefile.pacemate
	@echo "----- Done  ------"
	
open-source:
	@echo "----- Building for OPEN-SOURCE target  ------"
	@make -s -f Makefile.open-source
	@echo "----- Done  ------"

msp430:
	@echo "----- Building MSP430 target ------"
	@make -f Makefile.msp430
	@echo "----- Done  ------"
	
xmega:
	@echo "----- Building XMega target ------"
	@make -s -f Makefile.xmega
	@echo "----- Done  ------"

xmega128:
	@echo "----- Building XMega128 target ------"
	@make XMEGA_TYPE=128 -j4 -s -f Makefile.xmega
	@echo "----- Done  ------"

clean: clean-JN5121 clean-JN5139R clean-JN5139R1 clean-JN5148 clean-shawn clean-xmega clean-pacemate
	
clean-JN5121:
	@make JENNIC_CHIP=JN5121 -s -f Makefile.jennic clean
	@echo "----- JN5121 cleaned ------"

clean-JN5139R:
	@make JENNIC_CHIP=JN5139R -s -f Makefile.jennic clean
	@echo "----- JN5139R cleaned ------"

clean-JN5139R1:
	@make JENNIC_CHIP=JN5139R1 -s -f Makefile.jennic clean
	@echo "----- JN5139R1 cleaned ------"
	
clean-JN5148:
	@make JENNIC_CHIP=JN5148 -s -f Makefile.jennic.5148 clean
	@echo "----- JN5148 cleaned ------"

clean-shawn:
	@make -s -f Makefile.shawn clean
	@echo "----- shawn cleaned ------"
	
clean-pacemate:
	@make -s -f Makefile.pacemate clean
	@echo "----- pacemate cleaned ------"
	
clean-open-source:
	@make -s -f Makefile.open-source clean
	@echo "----- open-source clean  ------"
	
clean-msp430:
	@make -s -f Makefile.msp430 clean
	@echo "----- MSP430 cleaned ------"
	
clean-xmega:
	@make -s -f Makefile.xmega clean
	@echo "----- XMega cleaned ------"		
	
.PHONY: all buildbin debug buildlibs JN5121 JN5139R JN5139R1 JN5148 shawn pacemate clean-JN5121 clean-JN5139R clean-JN5139R1 clean-JN5148 clean-shawn clean-pacemate msp430 clean-msp430 xmega clean-xmega