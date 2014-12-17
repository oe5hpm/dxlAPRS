#PROCEDURE [2] X2C_STRCMP_PROC(a: xmRTS.X2C_pVOID; alen: size_t;
#                              b: xmRTS.X2C_pVOID; blen: size_t): int;

.text
.align 16

.global  X2C_STRCMP_PROC
.global _X2C_STRCMP_PROC

 X2C_STRCMP_PROC:
_X2C_STRCMP_PROC:
                pushl    %ebx
                pushl    %esi
                movl     12(%esp), %ebx
                movl     20(%esp), %edx
                movl     16(%esp), %esi
                cmpl     24(%esp), %esi
                jle      _loop
                movl     24(%esp), %esi
_loop:
                cmpl    $4, %esi
                jc      str_end
                movl    (%ebx), %eax
                movl    (%edx), %ecx
                cmpl    %eax,%ecx
                jne     str_neq
                notl    %ecx
                addl    $0x0FEFEFEFF, %eax
                andl    %ecx,%eax
                andl    $0x080808080, %eax
                jne     str_equal
                cmpl    $8,%esi
                jc      str_end4
                movl    4(%ebx),%eax
                movl    4(%edx),%ecx
                cmpl    %eax,%ecx
                jne     str_neq
                notl    %ecx
                addl    $0x0FEFEFEFF, %eax
                andl    %ecx, %eax
                andl    $0x80808080, %eax
                jne     str_equal
                cmpl    $12, %esi
                jc      str_end8
                movl    8(%ebx), %eax
                movl    8(%edx),  %ecx
                cmpl    %eax, %ecx
                jne     str_neq
                notl    %ecx
                addl    $0x0FEFEFEFF, %eax
                andl    %ecx,%eax
                andl    $0x80808080, %eax
                jne     str_equal
                cmpl    $16, %esi
                jc      str_end12
                movl    12(%ebx), %eax
                movl    12(%edx), %ecx
                cmpl    %eax, %ecx
                jne     str_neq
                addl    $16, %ebx
                addl    $16, %edx
                subl    $16, %esi
                notl    %ecx
                addl    $0x0FEFEFEFF, %eax
                andl    %ecx, %eax
                andl    $0x80808080, %eax
                je      _loop
str_equal:
                popl    %esi
                subl    %eax,%eax
                popl    %ebx
                ret
str_neq:
                cmpb    %cl, %al
                jne     byte_neq
                cmpb    $0, %al
                je      str_equal
                cmpb    %ch, %ah
                jne     byte_neq
                cmpb    $0, %ah
                je      str_equal
                shrl    $16, %eax
                shrl    $16, %ecx
                cmpb    %cl, %al
                jne     byte_neq
                cmpb    $0, %al
                je      str_equal
                cmpb    %ch, %ah        # Redundante code.
byte_neq:
                popl    %esi
                sbbl    %eax, %eax
                popl    %ebx
                orb     $1, %al
                ret

# Осталось сравнить менее 4 байтов

str_end12:
                addl    $4, %edx
                addl    $4, %ebx
                subl    $4, %esi
str_end8:
                addl    $4, %edx
                addl    $4, %ebx
                subl    $4, %esi
str_end4:
                addl    $4, %edx
                addl    $4, %ebx
                subl    $4, %esi
str_end:
                testl   %esi, %esi
                je      end_end

		movb (%ebx), %al
		movb (%edx), %cl
                cmpb    %cl, %al
                jne     byte_neq
                cmpb    $0, %al
                je      str_equal
                incl    %ebx
                incl    %edx
                cmpl    $1, %esi
                je      end_end

		movb (%ebx), %ah
		movb (%edx), %ch
                cmpb    %ch, %ah
                jne     byte_neq
                cmpb    $0, %ah
                je      str_equal
                incl    %ebx
                incl    %edx
                cmpl    $2, %esi
                je      end_end

		movb (%ebx), %al
		movb (%edx), %cl
                cmpb    %cl, %al
                jne     byte_neq
                cmpb    $0, %al
                je      str_equal
                incl    %ebx
                incl    %edx
end_end:
                movl    16(%esp), %esi
                cmpl    24(%esp), %esi
                je      str_equal
                jg      first_longer
                cmpb    $0, (%edx)
                je      str_equal
                popl    %esi
                movl    $-1, %eax
                popl    %ebx
                ret
first_longer:
                cmpb    $0, (%ebx)
                je      str_equal
                popl    %esi
                movl    $1, %eax
                pop     %ebx
                ret
