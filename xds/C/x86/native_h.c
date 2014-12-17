
/******************************************************************************/
/*                                                                            */
/*           Native code x86 run-time support for XDS Modula2/Oberon2         */
/*                                                                            */
/*           Compile by: wcc386 -ox -5s -zl -zld native.c                     */
/*                or by: sc -c -mx native.c                                   */
/*                                                                            */
/******************************************************************************/

unsigned X2C_HISETs [32] = {
        0xFFFFFFFF, 0xFFFFFFFE, 0xFFFFFFFC, 0xFFFFFFF8,
        0xFFFFFFF0, 0xFFFFFFE0, 0xFFFFFFC0, 0xFFFFFF80,
        0xFFFFFF00, 0xFFFFFE00, 0xFFFFFC00, 0xFFFFF800,
        0xFFFFF000, 0xFFFFE000, 0xFFFFC000, 0xFFFF8000,
        0xFFFF0000, 0xFFFE0000, 0xFFFC0000, 0xFFF80000,
        0xFFF00000, 0xFFE00000, 0xFFC00000, 0xFF800000,
        0xFF000000, 0xFE000000, 0xFC000000, 0xF8000000,
        0xF0000000, 0xE0000000, 0xC0000000, 0x80000000
};
