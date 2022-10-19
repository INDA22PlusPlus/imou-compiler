
# --C lang (non-serious)
3 week assignment for the "++" programming group at KTH. Includes a manually written tokenizer, parser and compiler (in progress).
Parses a joke static typed language and converts it to x86-64 assembly (AT&T syntax) which can be fully compiled into a binary executable using GAS (`as` command in GNU binutils). The language itself allows math expressions without parenthesis, while loops with comparisons, `if`, `elif`, `else` statements and a `print` function.
<br>
Check `examples/` folder in the project tree, for language examples, AST serializer outputs and compiler outputs.

## Tokenizer
Reads a null-terminated string, and returns a pre-allocated array of tokens with `_token` type.

## Parser
Converts the given token array from the tokenizer function, into a pre-allocated Abstract Syntax Tree (AST).
<br>
The image below demonstrates the output of the parser function. Cred to [ivan111](https://github.com/ivan111/vtree), for the JSON tree visualizer. ![image](https://user-images.githubusercontent.com/68696386/196812284-0f8c6719-ff57-47d7-8efb-acf2e453dcf5.png)


## Compiler (in progress)
Final goal is to convert a lang file into a fully working x86-64 assembly. For now, math expressions are compiled into assembly instructions which work as they should.
<br>

Copied from `examples/test.asm`. Compilation of expression: `20*5*420/2+5*2+3*2*5`.
```assembly
.global	_start


.text
_start:
	pushq	%rbp
	movq	%rsp, %rbp

	# MATH EXPRESSION COMPILER OUTPUT BEGIN
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
	# MATH EXPRESSION COMPILER OUTPUT END
	
	
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
```
