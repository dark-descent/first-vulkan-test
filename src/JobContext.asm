[BITS 64]

global saveContext
global restoreContext

section .text

saveContext:
	mov r8, [rsp]
  	mov [rdi + (8 * 0)], r8 ; RIP
  	lea r8, [rsp + 8]
  	mov [rdi + (8 * 1)], r8	; RSP

  	; Save preserved registers.
  	mov [rdi + (8 * 2)], rbx
  	mov [rdi + (8 * 3)], rbp
  	mov [rdi + (8 * 4)], r12
  	mov [rdi + (8 * 5)], r13
  	mov [rdi + (8 * 6)], r14
  	mov [rdi + (8 * 7)], r15

	xor eax, eax
	ret

restoreContext:
	mov r8, [rdi + (8 * 0)]

	; Load new stack pointer.
	mov rsp, [rdi + (8 * 1)]

	; Load preserved registers.
	mov rbx, [rdi + (8 * 2)]
	mov rbp, [rdi + (8 * 3)]
	mov r12, [rdi + (8 * 4)]
	mov r13, [rdi + (8 * 5)]
	mov r14, [rdi + (8 * 6)]
	mov r15, [rdi + (8 * 7)]

	push r8

	xor eax, eax
	ret
