	.code
	;https://github.com/cs1ime
_text SEGMENT

MySehPoc PROC

;rcx={pocaddr,funaddr},rdx=arg1,r8=arg2,r9=arg3

push rbx
push rsi
push rdi
push rbp
push r10
push r11
push r12
push r13
push r14
push r15


mov r10,rcx
mov rcx,[r10+16]

sub rsp,50h

lea rax,p2c2
push rax
sub rsp,18h ;

push qword ptr[r10]

jmp qword ptr[r10+8]

p2c2:
add rsp,50h
pop r15
pop r14
pop r13
pop r12
pop r11
pop r10
pop rbp
pop rdi
pop rsi
pop rbx

cmp eax,0
je set1
set0:
xor rax,rax
jmp reten
set1:
mov rax,1
reten:
ret

MySehPoc ENDP

_text ENDS

	END
