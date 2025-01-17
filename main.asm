global _start

section .data
	msg db "Hello, Cruel World!", 0x0a
	len equ $ - msg


section .text
_start:
	mov	rax, 4
	mov rbx, 1
	mov rcx, msg
	mov rdx, len
	int 0x80
	mov	rax, 1
	mov rbx, 0
	int 0x80