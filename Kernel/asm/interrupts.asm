EXTERN beep_asm

GLOBAL _cli
GLOBAL _sti
GLOBAL picMasterMask
GLOBAL picSlaveMask
GLOBAL haltcpu
GLOBAL _hlt

GLOBAL _irq00Handler
GLOBAL _irq01Handler
GLOBAL _irq02Handler
GLOBAL _irq03Handler
GLOBAL _irq04Handler
GLOBAL _irq05Handler
GLOBAL _irq128Handler

GLOBAL _exception0Handler
GLOBAL _exception6Handler
GLOBAL _regsInterrupt

GLOBAL regsBuf
GLOBAL regs_saved
GLOBAL _setUser

EXTERN irqDispatcher
EXTERN syscall_handler
EXTERN exceptionDispatcher
EXTERN getStackBase

SECTION .text

%macro pushState 0
	push rax
	push rbx
	push rcx
	push rdx
	push rbp
	push rdi
	push rsi
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
%endmacro

%macro popState 0
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rsi
	pop rdi
	pop rbp
	pop rdx
	pop rcx
	pop rbx
	pop rax
%endmacro

%macro irqHandlerMaster 1
	pushState

	mov rdi, %1 ; pasaje de parametro
	call irqDispatcher

	; signal pic EOI (End of Interrupt)
	mov al, 20h
	out 20h, al

	popState
	iretq
%endmacro

%macro saveRegs 0
	mov [regsBuf+8], rbx
	mov [regsBuf+8*2], rcx
	mov [regsBuf+8*3], rdx
	mov [regsBuf+8*4], rsi
	mov [regsBuf+8*5], rdi
	mov [regsBuf+8*6], rbp
	mov [regsBuf+8*7], rsp
	mov [regsBuf+8*8], r8
	mov [regsBuf+8*9], r9 
	mov [regsBuf+8*10], r10
	mov [regsBuf+8*11], r11
	mov [regsBuf+8*12], r12
	mov [regsBuf+8*13], r13
	mov [regsBuf+8*14], r14
	mov [regsBuf+8*15], r15

	mov rax, [rsp + 8]			; RIP
	mov [regsBuf+8*16], rax			

	mov rax, [rsp+8*3]		; RFLAGS
	mov [regsBuf+8*17], rax
%endmacro


%macro exceptionHandler 1
    push rax
    saveRegs
    pop rax
    mov [regsBuf], rax

	call beep_asm
    mov rax, userland_direc 
    mov [rsp], rax          ; hard-code goes brrrrrr

    mov rax, 0x8
    mov [rsp + 8], rax      ; CS de userland

    mov rax, 0x202
    mov [rsp + 8*2], rax    ; RFLAGS

    call getStackBase       
    mov [rsp + 8*3], rax    ; sp ahora esta en la base 

    mov rax, 0x0
    mov [rsp + 8*4], rax    ; SS de userland

	pushState

	mov rdi, %1              ; pasaje de parametro
	call exceptionDispatcher

    popState

	iretq
	
%endmacro

;================================================================================================================================
;_setUser realiza el seteo de entorno y salta a userland
;================================================================================================================================
;================================================================================================================================
_setUser:

	sub rsp, 32             ; no hace falta esto pero es buena practica

    mov rax, userland_direc 
    mov [rsp], rax          ; preparo el salto a userland 

    mov rax, 0x8
    mov [rsp + 8], rax      ; CS de userland

    mov rax, 0x202
    mov [rsp + 8*2], rax    ; RFLAGS

    call getStackBase       
    mov [rsp + 8*3], rax    ; sp ahora esta en la base 

    mov rax, 0x0
    mov [rsp + 8*4], rax    ; SS de userland
	
	iretq

_hlt:
	sti
	hlt
	ret

_cli:
	cli
	ret


_sti:
	sti
	ret

picMasterMask:
	push    rbp
    mov     rbp, rsp
    mov     ax, di
    out	    21h,al
    pop     rbp
    retn

picSlaveMask:
	push    rbp
    mov     rbp, rsp
    mov     ax, di  ; ax = mascara de 16 bits
    out	    0A1h,al
    pop     rbp
    retn


;8254 Timer (Timer Tick)
_irq00Handler:
	irqHandlerMaster 0

;Keyboard
_irq01Handler:
    push rax

    mov rax, 0
    in al, 60h 
    cmp al, 0x38
    jne .handle

    ; Aca llego si toque el alt
    saveRegs
    mov [regs_saved], byte 1
    mov rax, [rsp]
    mov [regsBuf], rax

; handle as usual
.handle:
    pop rax 
	irqHandlerMaster 1

;Cascade pic never called
_irq02Handler:
	irqHandlerMaster 2

;Serial Port 2 and 4
_irq03Handler:
	irqHandlerMaster 3

;Serial Port 1 and 3
_irq04Handler:
	irqHandlerMaster 4

;USB
_irq05Handler:
	irqHandlerMaster 5

;Syscall
_irq128Handler:
    mov r9, rax
    call syscall_handler
    iretq

;Zero Division Exception
_exception0Handler:
	exceptionHandler 0

;Invalid Op Code Exception
_exception6Handler:
	exceptionHandler 6

haltcpu:
	cli
	hlt
	ret


_regsInterrupt:
    mov rax, regsBuf
	ret 

SECTION .data
    regs_saved db 0

SECTION .bss
	aux resq 1
	regsBuf resq 18

section .rodata
	userland_direc equ 0x400000
