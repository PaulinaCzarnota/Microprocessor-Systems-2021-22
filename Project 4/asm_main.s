; ****** RAM *********
	AREA DATA
target SPACE 20
; *****************

; ****** ROM *********

; The following symols are in the CODE section (ROM,Executable, readonly)
    AREA THUMB,CODE,READONLY
; EXPORTED Symbols can be linked against
    EXPORT Reset_Handler
    EXPORT __Vectors
; Minimal interrupt vector table follows
; First entry is initial stack pointer (top of stack)
; second entry is the address of the reset handler
__Vectors
    DCD 0x20000000+8192
    DCD Reset_Handler 

; 'Main' program goes here
; Calling convention : R0 used to pass data to a function
Reset_Handler
	LDR R0,=TestString
	BL mystrlen

	LDR R0,=target ; destination
	LDR R1,=TestString  	; source
	BL mystrcpy
	
	LDR R0,=target ; destination
	LDR R1,=TestString ; source
	MOVS R2,#7 ; random value for 'n'
	BL mystrncpy
	
 
	LDR R0,=target
	MOVS R1,#6  ; random Value
	MOVS R2,#10 ; do it 10 times (n)
	BL memset
	
 
stop  B stop
; incoming parameter in R0 i.e. the address of the string
; result should be return in R0
mystrlen
	PUSH{R1-R2}
	MOVS R1,R0		; switch the pointer in to R1 as we need R0 to count
	MOVS R0,#0		; count = 0;
mystrlen_loop
	LDRB R2,[R1]	; read next byte
	CMP R2,#0		; is it null?
	BEQ mystrlen_exit ; if so then leave
	ADDS R0,R0,#1	; else increment the counter
	ADDS R1,R1,#1	; increment the pointer (to char)
	B mystrlen_loop
mystrlen_exit
	POP{R1-R2}   ;Raead next Byte
	BX LR ; return to the caller

mystrcpy
; on entry R0 points to the destination , R1 points to the source
mystrcpy_loop
	LDRB R2,[R1] ; read next byte
	STRB R2,[R0] ; write to the next byte
	CMP R2,#0 ; checking if it is zero
	BEQ mystrcpy_exit ; if so then leave
	ADDS R2,R2,#1; else increment the source
	ADDS R1,R1,#1 ; increment the pointer (destination)
	B mystrcpy_loop
mystrcpy_exit
	BX LR ; return to the call

mystrncpy

mystrncpy_loop
	LDRB R3,[R1] ; reading next Byte
	STRB R3,[R0] ; writing next byte
	CMP R3,#0 ; cheking if the value is zero or not for while
	BEQ mystrncpy_exit ; if its 0 then leave
	
	CMP R2,#0  ; checking if n = 0 or not for if ststement
	BEQ mystrncpy_exit ; is so then leave
	ADDS R3,R3, #1 ; increamenting the source
	ADDS R1,R1,#1 ; increment the pointer
	SUBS R2,R2,#1  ; decreament the 'n' 
	B mystrncpy_loop
mystrncpy_exit
	BX LR ; return to the call
	
memset 

memset_loop
	CMP R2,#0  ; checking if its zero or not
	BEQ memset_exit ; if zero exit the program
	STRB R1,[R0] ; store  the value of R0 in R1
	ADDS R0,R0,#1 ; add + 1 to R0
	SUBS R2,R2,#1 ; subtract n by 1
	B memset_loop
memset_exit
	
	BX LR ; return to the call
	
	
; *****************
; Constant data is stored in Flash ROM
TestString DCB "Hello World\0"
	end