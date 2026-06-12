    .code16
    .text
    .global XtosDosStayResident

XtosDosStayResident:
    pushw %bp
    movw %sp, %bp
    movw 6(%bp), %dx
    xorw %ax, %ax
    movb $0x31, %ah
    int $0x21
    popw %bp
    lret
