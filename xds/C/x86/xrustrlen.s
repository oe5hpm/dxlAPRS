# COPYRIGHT (c) 1995 xTech Ltd. All Rights Reserved.
# Implementation for X2C_LENGTH function

.text
.align 16

#PROCEDURE [2] X2C_LENGTH (a: xmRTS.X2C_pVOID; alen: size_t): size_t;

.global  X2C_LENGTH
.global _X2C_LENGTH

 X2C_LENGTH:
_X2C_LENGTH:
                movl    4(%esp), %ecx
                movl    8(%esp), %edx
                pushl   %esi
                testl   $3, %ecx 
                jz      LLoop
ALoop:
                movb    (%ecx), %al
                incl    %ecx
                testb   %al, %al
                jz      ret_m1
                decl    %edx
                js      ret_len
                testb   $3, %cl
                jne     ALoop
LLoop:
                subl    $4, %edx
                js      tail
                movl    (%ecx), %esi
                movl    $0xFEFEFEFF, %eax
                addl    %esi, %eax
                notl    %esi
                andl    %esi, %eax
                addl    $4, %ecx
                testl   $0x80808080, %eax
                jz      LLoop

                testb   %al, %al
                js      ret_m4
                testb   %ah, %ah
                js      ret_m3
                testl   $0x00800000, %eax
                jnz     ret_m2
ret_m1:
                leal    -1(%ecx), %eax
                movl     8(%esp), %ecx
                subl     %ecx, %eax
                popl     %esi
                ret     
ret_m2:
                leal    -2(%ecx), %eax
                movl     8(%esp), %ecx
                subl     %ecx, %eax
                popl     %esi
                ret     
ret_m3:
                leal    -3(%ecx), %eax
                movl     8(%esp), %ecx
                subl     %ecx, %eax
                popl     %esi
                ret     
ret_m4:
                leal    -4(%ecx), %eax
                movl     8(%esp), %ecx
                subl     %ecx, %eax
                popl     %esi
                ret
tail:
                addl    $4, %edx
                jz      ret_len
TLoop:
                movb    (%ecx),%al
                incl    %ecx
                testb   %al, %al
                jz      ret_m1
                decl    %edx
                jne     TLoop
ret_len:
                movl    12(%esp),%eax
                popl    %esi
                ret
