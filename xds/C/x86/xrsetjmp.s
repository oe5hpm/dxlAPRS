# COPYRIGHT (c) 1999 Excelsior. All Rights Reserved.
# COPYRIGHT (c) 1995,99 XDS. All Rights Reserved.
#
# Implementation for X2C_setjmp and X2C_longjmp functions
#

.text
.align 16
#PROCEDURE X2C_setjmp(VAR point: X2C_jmp_buf): INTEGER;

.global X2C_setjmp
X2C_setjmp:
        movl 4(%esp), %eax
        movl %ebx, (%eax)
        movl %ecx,0x4(%eax)
        movl %edx,0x8(%eax)
        movl %esi,0xc(%eax)
        movl %edi,0x10(%eax)
        movl %ebp,0x14(%eax)
        popl 0x18(%eax)
        mov %esp, %ecx
        addl $4, %ecx
        movl %ecx, 0x1c(%eax)
        pushl 0x18(%eax)
        movw %es,    0x20(%eax)
        movw %ds,    0x22(%eax)
        movw %cs,    0x24(%eax)
        movw %fs,    0x26(%eax)
        movw %gs,    0x28(%eax)
        movw %ss,    0x2a(%eax)
        xorl     %eax,%eax
        ret

#PROCEDURE X2C_longjmp(VAR point: X2C_jmp_buf; n: INTEGER);
.global X2C_longjmp
X2C_longjmp:
        popl     %eax     # drop return EIP
        popl     %eax     # jmp_buf pointer
        popl     %edx     # argument
        movw     0x2a(%eax),%ss        # switch to new stack

        movl     0x1c(%eax), %esp       #
        pushl    %eax         
        pushl    0x18(%eax) # return EIP
        orl      %edx,%edx
        jne     L1
        incl     %edx
    L1: pushl    %edx
        movl         (%eax),%ebx
        movl     0x4 (%eax) ,%ecx
        movl     0xc (%eax),%esi
        movl     0x10(%eax),%edi
        movl     0x14(%eax),%ebp
        movw     0x20(%eax),%dx
        verr    %dx
        je      L2
        xorl     %edx,%edx
    L2: movw     %dx,%es
        movl     0x8(%eax),%edx
        mov      0x22(%eax),%ds
        popl     %eax
        ret
