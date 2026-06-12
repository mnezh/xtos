    .code16
    .text
    .global XtosInt60Call
    .global XtosInt60VectorPresent

XtosInt60Call:
    pushw %bp
    movw %sp, %bp
    pushw %ds
    pushw %dx

    movw 4(%bp), %dx
    movw 6(%bp), %ds
    int $0x60

    popw %dx
    popw %ds
    popw %bp
    ret

XtosInt60VectorPresent:
    pushw %bp
    movw %sp, %bp
    pushw %es
    pushw %bx

    movb $0x35, %ah
    movb $0x60, %al
    int $0x21

    movw %es, %ax
    orw %bx, %ax
    jz .missing
    movw $1, %ax
    jmp .done

.missing:
    xorw %ax, %ax

.done:
    popw %bx
    popw %es
    popw %bp
    ret
