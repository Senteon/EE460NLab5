.ORIG x1200
      ADD R6, R6 #-2          ;push
      STW R1, R6, #0
      ADD R6, R6, #-2
      STW R2, R6, #0
	ADD R6, R6, #-2
      STW R3, R6, #0
	ADD R6, R6, #-2
      STW R4, R6, #0

      LEA R1, sta            	;load R1 with address of x1000
	LEA R2, siz			;initialize counter in R1
	LEA R4, mask		;load mask address

	LDW R1, R1, #0		;load x1000 into R1
      LDW R2, R2, #0          ;load R2 with #128
	LDW R4, R4, #0		;load mask

loo   LDW R3, R1, #0		;load R3 with PTE
	AND R3, R3, R4		;mask PTE
	STW R3, R1, #0		;store back into pagetable
	ADD R1, R1, #1		;next location
	ADD R2, R2, #-1		;fewer left
	BRNP loo		;branch back until hits zero

	LDW R4, R6, #0
      ADD R6, R6, #2
	LDW R3, R6, #0
      ADD R6, R6, #2
      LDW R2, R6, #0
      ADD R6, R6, #2
      LDW R1, R6, #0
      ADD R6, R6, #2         ;pop
      RTI

sta	.FILL x1000
siz	.FILL #128
mask	.FILL xFFFE			;bitmask for clearing reference bit
.END
