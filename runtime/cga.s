    .code16
    .text
    .global CgaSetVideoMode
    .global CgaSetColors

CgaSetVideoMode:
    pushw %bp
    movw %sp, %bp
    pushw %ds
    pushw %es

    movb 6(%bp), %al
    xorb %ah, %ah
    int $0x10

    popw %es
    popw %ds
    popw %bp
    lret

CgaSetColors:
    pushw %bp
    movw %sp, %bp
    pushw %ds
    pushw %es

    movb $0x0b, %ah
    xorb %bh, %bh
    movb 6(%bp), %bl
    int $0x10

    movb $0x0b, %ah
    movb $1, %bh
    movb 8(%bp), %bl
    int $0x10

    popw %es
    popw %ds
    popw %bp
    lret
