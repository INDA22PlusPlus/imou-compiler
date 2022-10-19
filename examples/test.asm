# OUTPUT OF COMPILER WHEN COMPILING MATH EXPRESSION TREE, INSERTED INTO A X86-64 BODY
# FULLY WORKING EXAMPLE

# EXPRESSION	20*5*420/2+5*2+3*2*5;

.global	_start


.text
_start:
	pushq	%rbp
	movq	%rsp, %rbp

	movl $5, %r8d
	movl $20, %r10d
	mov %r8d, %eax
	mul %r10d
	mov %eax, %r10d
	movl $420, %r8d
	mov %r8d, %eax
	mul %r10d
	mov %eax, %r10d
	movl $2, %r8d
	mov %r10d, %eax
	xor %edx, %edx
	div %r8d
	mov %eax, %r10d
	movl %r10d, %eax
	pushq %rax
	movl $2, %r8d
	movl $5, %r10d
	mov %r8d, %eax
	mul %r10d
	mov %eax, %r8d
	popq %rax
	movl %eax, %r10d
	xor %r9d, %r9d
	movl %r8d, %r9d
	addl %r10d, %r9d
	movl %r9d, %r10d
	movl %r10d, %eax
	pushq %rax
	movl $2, %r8d
	movl $3, %r10d
	mov %r8d, %eax
	mul %r10d
	mov %eax, %r10d
	movl $5, %r8d
	mov %r8d, %eax
	mul %r10d
	mov %eax, %r8d
	popq %rax
	movl %eax, %r10d
	xor %r9d, %r9d
	movl %r8d, %r9d
	addl %r10d, %r9d
	movl %r9d, %r10d
	
	movl %r10d, -4(%rbp)
	
	mov	$1, %rax
	mov	$1, %rdi
	lea	-4(%rbp), %rsi
	mov	$4, %rdx
	syscall
	
	popq	%rbp
	
	movq	$60, %rax
	xor	%rdi, %rdi
	syscall