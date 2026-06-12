    .code16
    .text
    .global MouseReset
    .global MouseRead

MouseReset:
    pushw %bp
    movw %sp, %bp
    pushw %ds
    pushw %es

    xorw %ax, %ax
    int $0x33

    popw %es
    popw %ds
    popw %bp
    lret

MouseRead:
    pushw %bp
    movw %sp, %bp
    pushw %ds
    pushw %es
    pushw %bx
    pushw %cx
    pushw %dx
    pushw %si

    movw $3, %ax
    int $0x33

    movw 6(%bp), %si
    movw %cx, (%si)

    movw 8(%bp), %si
    movw %dx, (%si)

    movw 10(%bp), %si
    movw %bx, (%si)

    movw %bx, %ax

    popw %si
    popw %dx
    popw %cx
    popw %bx
    popw %es
    popw %ds
    popw %bp
    lret
