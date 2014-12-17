# Run-time support for XDS optimizing compiler (Linux ELF version)

.equ    X2C_INDEX       ,     0
.equ    X2C_RANGE       ,     1
.equ    X2C_NIL         ,     3
.equ    X2C_OVERFLOW    ,     5
.equ    X2C_DIVISION    ,     6
.equ    X2C_FLT_OVERFL  ,     7
.equ    X2C_FLT_DIV     ,     8


                .global  X2C_FLT_USED
                .global  SEEK_SET
                .global  SEEK_CUR
                .global  SEEK_END
                .global  P_WAIT
                .global  O_BINARY
                .global  O_CREAT
                .global  O_RDONLY
                .global  O_WRONLY
                .global  O_RDWR
                .global  S_IREAD
                .global  S_IWRITE

                .type  X2C_FLT_USED,@object
                .type  SEEK_SET    ,@object
                .type  SEEK_CUR    ,@object
                .type  SEEK_END    ,@object
                .type  P_WAIT      ,@object
                .type  O_BINARY    ,@object
                .type  O_CREAT     ,@object
                .type  O_RDONLY    ,@object
                .type  O_WRONLY    ,@object
                .type  O_RDWR      ,@object
                .type  S_IREAD     ,@object
                .type  S_IWRITE    ,@object

                .local fp_env 
                .local ErrCode
                .local ErrEIP 
                .local UnableStr
                .local FPP_Init
                .local code_from
                .local code_to


.data
.align 4

X2C_FLT_USED:  .long      0
SEEK_SET    :  .long      0
SEEK_CUR    :  .long      1
SEEK_END    :  .long      2
P_WAIT      :  .long      0
O_BINARY    :  .long      00000  # don't know what to put here, there is no such flag in linux
O_CREAT     :  .long      00100
O_RDONLY    :  .long      00000
O_WRONLY    :  .long      00001
O_RDWR      :  .long      00002
S_IREAD     :  .long      00400
S_IWRITE    :  .long      00200
fp_env      :  .zero      28 
ErrCode     :  .long      0
ErrEIP      :  .long      0
UnableStr   :  .asciz    "Unable to process exception\020"
FPP_Init    :  .long      0
code_from   :  .long      1  # when code_from>code_to no pointer referencing
code_to     :  .long      0  # occurs in X2C_IS_CALL.


.text
.align 16


# ==============================================================================

#               .global  X2C_FINALLY
#X2C_FINALLY:   jmp X2C_FINALEXE

# ==============================================================================

                .global  X2C_TRAP_NIL
X2C_TRAP_NIL:
                popl     %eax
                pushl    $X2C_NIL
                pushl    %eax
                jmp      X2C_TRAP_F


                .global  X2C_TRAP_NIL_C
X2C_TRAP_NIL_C:
                popl     %eax
                pushl    $X2C_NIL
                pushl    %eax
                jmp      X2C_TRAP_FC

# ==============================================================================

                .global  X2C_TRAP_DIV
X2C_TRAP_DIV:
                popl     %eax
                pushl    $X2C_DIVISION
                pushl    %eax
                jmp      X2C_TRAP_F


                .global  X2C_TRAP_DIV_C
X2C_TRAP_DIV_C:
                popl     %eax
                pushl    $X2C_DIVISION
                pushl    %eax
                jmp      X2C_TRAP_FC

# ==============================================================================

                .global  X2C_TRAP_OVERFL
X2C_TRAP_OVERFL: 
                popl     %eax
                pushl    $X2C_OVERFLOW
                pushl    %eax
                jmp      X2C_TRAP_F


                .global  X2C_TRAP_OVERFL_C
X2C_TRAP_OVERFL_C: 
                popl     %eax
                pushl    $X2C_OVERFLOW
                pushl    %eax
                jmp      X2C_TRAP_FC

# ==============================================================================

                .global  X2C_TRAP_RANGE
X2C_TRAP_RANGE:
                popl     %eax
                pushl    $X2C_RANGE
                pushl    %eax
                jmp      X2C_TRAP_F


                .global  X2C_TRAP_RANGE_C
X2C_TRAP_RANGE_C:
                popl     %eax
                pushl    $X2C_RANGE
                pushl    %eax
                jmp      X2C_TRAP_FC

# ==============================================================================

                .global  X2C_TRAP_INDEX
X2C_TRAP_INDEX:
                popl     %eax
                pushl    $X2C_INDEX
                pushl    %eax
                jmp      X2C_TRAP_F


                .global  X2C_TRAP_INDEX_C
X2C_TRAP_INDEX_C:
                popl     %eax
                pushl    $X2C_INDEX
                pushl    %eax
                jmp      X2C_TRAP_FC

# ==============================================================================

Exception:
                cmpl    $0, X2C_FLT_USED
                je      L1
                fstenv  fp_env
                andw    $0x0c700,  fp_env+4
                movw    $0x0ffff,  fp_env+8
                fldenv  fp_env
L1:             pushl   ErrCode
                pushl   ErrEIP
                jmp     X2C_TRAP_F

# ==============================================================================


filter:
                pushl   %ebx
                pushl   %esi
                pushl   %edi
                movl    16(%esp), %ebx           # ExceptionInfo
                movl    (%ebx), %eax             # ExceptionRecord
                movl    4(%eax), %edi            # ExceptionFlags
                testl   %edi, %edi
                je      filter_cont

                pushl    $UnableStr
                call    printf
                addl    $4, %esp
                jmp     filter_err
filter_cont:
                movl   (%eax), %eax              # ExceptionCode

                movl    $X2C_FLT_DIV, %edi
                cmpl    $0x0c000008e, %eax
                je      filter_ok

                movl     $X2C_FLT_OVERFL, %edi
                cmpl     $0x0c000008d, %eax
                je       filter_ok
                cmpl     $0x0c000008f, %eax
                je       filter_ok
                cmpl     $0x0c0000090, %eax
                je       filter_ok
                cmpl     $0x0c0000091, %eax
                je       filter_ok
                cmpl     $0x0c0000092, %eax
                je       filter_ok
                cmpl     $0x0c0000093, %eax
                je       filter_ok

                movl     $X2C_DIVISION, %edi
                cmpl     $0x0c0000094, %eax
                je       filter_ok

                movl     $X2C_OVERFLOW, %edi
                cmpl     $0x0c0000095, %eax
                je       filter_ok

                movl     $X2C_INDEX, %edi
                cmpl     $0x0c000008c, %eax
                je       filter_ok
filter_err:
                movl     $1, %eax
                jmp      filter_ret
filter_ok:
                movl     %edi, ErrCode
                movl     4(%ebx),%eax                 # ContextRecord
                movl     0xb8(%eax),%edx                 # Eip
                movl     %edx,ErrEIP
                movl     $Exception,  0x0B8(%eax)
                movl     $0x0ffffffff, %eax
filter_ret:
                popl     %edi
                popl     %esi
                popl     %ebx
                ret      $4

# ==============================================================================

                .global  X2C_NATIVE_BEGIN
X2C_NATIVE_BEGIN :

                ret

# ==============================================================================
# Determines wether given pointer points to a procedure call or not.
# Returns pointer to call instruction if any, otherwise zero
# void*  X2C_IS_CALL ( void* )

.global  X2C_IS_CALL
.global _X2C_IS_CALL
.global  X2C_SET_CODE_EXTENT
.global _X2C_SET_CODE_EXTENT


#PROCEDURE / X2C_SET_CODE_EXTENT(from,to: SYSTEM.ADDRESS);
 X2C_SET_CODE_EXTENT:
_X2C_SET_CODE_EXTENT:
                        movl    4(%esp), %eax
                        addl    $12,%eax        #to be sure that x2c_is_call won't attempt to access memory ot of [code_from..code_to] range.
                        movl    %eax,code_from
                        movl    8(%esp),%eax
                        movl    %eax,code_to
                        ret

#PROCEDURE / X2C_IS_CALL(i: LONGINT): LONGINT;
 X2C_IS_CALL:
_X2C_IS_CALL:
                movl     4(%esp), %eax

# Прежде всего: адрес в кодовом сегменте?

                movl     code_from, %ecx
                cmpl     %ecx, %eax
                jb       quit
                cmpl     code_to, %eax
                jbe      check
quit:           xorl     %eax, %eax
                ret

# Сначала проверяем на call [reg]

check:          decl     %eax
                decl     %eax
                cmpb    $0x0FF, (%eax)
                jne     not0
                movb    1(%eax),%cl
                cmpb    $0x14, %cl                 # не бывает [esp]
                je      not0
                cmpb    $0x15, %cl                 # и [ebp]
                je      not0
                cmpb    $0x0D4, %cl
                je      not0
                andb    $0x0F8, %cl
                cmpb    $0x0D0, %cl
                je      true
                cmpb    $0x10, %cl
                je      true

# Теперь проверяем на call d8 [reg]

not0:           decl    %eax
                cmpb    $0x0FF, (%eax)
                jne     not1
                movb    1(%eax), %cl
                cmpb    $0x54, %cl                 # не бывает d8 [esp]
                je      not1
                andb    $0x0F8, %cl
                cmpb    $0x50, %cl
                je      true

# Проверяем на call [reg1 + scale * reg2]

                cmpb    $0x14, 1(%eax)
                jne     not1
                movb    2(%eax), %cl
                andb    $7, %cl
                cmpb    $5, %cl                    # не бывает [ebp + scale * reg2]
                jne     true

# Теперь проверяем на call d8 [reg1 + scale * reg2]

not1:           decl    %eax
                cmpb    $0x0FF, (%eax)
                jne     not2
                movb    1(%eax),%cl
                andb    $0x0F8, %cl
                cmpb    $0x050, %cl
                je      true

# Теперь проверяем на call relative

not2:           decl    %eax
                cmpb    $0x0E8, (%eax)
                je      true

# Теперь проверяем на call d32 и на d32 [reg]

                decl    %eax
                cmpb    $0x0FF, (%eax)
                jne     not3
                movb    -5(%eax), %cl
                cmpb    $0x094, %cl                 # не бывает d32 [esp]
                je      not3
                andb    $0x0B8, %cl
                cmpb    $0x090, %cl
                je      true

# И наконец проверяем на call d32 [reg1 + scale * reg2]

not3:           decl    %eax
                cmpb    $0x0FF, (%eax)
                jne     false
                cmpb    $0x94, 1(%eax)
                je      true

false:          xorl    %eax, %eax
true:           ret


#===============================================================================
# Clear exception flags, empty FP stack & set control word.
# Set FPP control word:
#     - all exceptions, except precision loss and denormalized operand, enabled.
#     - presision control set to 11 -- 64 bit extended presision
#     - rounding control set to 00 -- round to nearest or even

                .global  X2C_InitFPP
X2C_InitFPP :
                 cmpl    $0, FPP_Init
                 jne     X2C_InitFPP_L
                 fninit
                 pushl    $0x0332
                 fldcw   (%esp)
                 pop     %eax
                 incl    FPP_Init
 X2C_InitFPP_L:  ret




# Grabbed from gcc's output of the folowing:
#  long double X2C_EXPRR(long double base,long double ex)
#  {
#     return pow(base,ex);
#  }

                .global X2C_EXPRR
        .type    X2C_EXPRR,@function
X2C_EXPRR:
        pushl %ebp
        movl %esp,%ebp
        pushl 28(%ebp)
        pushl 24(%ebp)
        pushl 20(%ebp)
        pushl 16(%ebp)
        pushl 12(%ebp)
        pushl 8(%ebp)
        call pow
        addl $24,%esp
        movl %eax,%eax
        pushl %eax
        fildl (%esp)
        addl $4,%esp
        jmp .L1
        .align 16
.L1:
        movl %ebp,%esp
        popl %ebp
        ret
