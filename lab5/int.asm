	.ORIG x1200
	ADD R6, R6, #-2
	STW R0, R6, #0
	ADD R6, R6, #-2
	STW R1, R6, #0 
	ADD R6, R6, #-2
	STW R2, R6, #0
	;;;;;;;;;;;;	
	LEA R0, ADDR
	LDW R0, R0, #0; R0 = x1000
	LEA R1, COUNT
	LDW R1, R1, #0; R1 = x0080
LOOP	LDW R2, R0, #0
	AND R2, R2, #-2
	STW R2, R0, #0	
	ADD R0, R0, #2
	ADD R1, R1, #-1
	BRp LOOP
	;;;;;;;;;;;;
	LDW R2, R6, #0
	ADD R6, R6, #2
	LDW R1, R6, #0
	ADD R6, R6, #2
	LDW R0, R6, #0
	ADD R6, R6, #2
	RTI
ADDR 	.FILL x1000
COUNT .FILL x0080
.END	
