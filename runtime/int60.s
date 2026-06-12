    .code16
    .text
    .global XtosInt60Install
    .global XtosInt60Restore
    .global XtosInt60Call
    .global XtosInt60Handler
    .global RuntimeCgaReadByte

XtosInt60Install:
    pushw %bp
    movw %sp, %bp
    pushw %ds
    pushw %es
    pushw %bx
    pushw %dx

    movb $0x35, %ah
    movb $0x60, %al
    int $0x21
    movw %bx, xtos_old_int60_offset
    movw %es, xtos_old_int60_segment
    movw $1, xtos_int60_installed

    pushw %cs
    popw %ds
    movw $XtosInt60Handler, %dx
    movb $0x25, %ah
    movb $0x60, %al
    int $0x21

    popw %dx
    popw %bx
    popw %es
    popw %ds
    popw %bp
    lret

XtosInt60Restore:
    pushw %bp
    movw %sp, %bp
    pushw %ds
    pushw %bx
    pushw %dx
    cmpw $0, xtos_int60_installed
    je .restore_done

    movw xtos_old_int60_offset, %dx
    movw xtos_old_int60_segment, %bx
    movw $0, xtos_int60_installed
    movw %bx, %ds
    movb $0x25, %ah
    movb $0x60, %al
    int $0x21

.restore_done:
    popw %dx
    popw %bx
    popw %ds
    popw %bp
    lret

XtosInt60Call:
    pushw %bp
    movw %sp, %bp
    pushw %ds
    pushw %dx

    movw 6(%bp), %dx
    movw 8(%bp), %ds
    int $0x60

    popw %dx
    popw %ds
    popw %bp
    lret

XtosInt60Handler:
    pushw %bp
    movw %sp, %bp
    pushw %ds
    pushw %es
    pushw %bx
    pushw %cx
    pushw %dx
    pushw %si
    pushw %di

    pushw %ds
    pushw %dx
    lcall $XtosInt60Dispatch@OZSEG16, $XtosInt60Dispatch
    addw $4, %sp

    popw %di
    popw %si
    popw %dx
    popw %cx
    popw %bx
    popw %es
    popw %ds
    popw %bp
    iret

RuntimeCgaReadByte:
    pushw %bp
    movw %sp, %bp
    pushw %es
    pushw %bx

    movw $0xb800, %ax
    movw %ax, %es
    movw 6(%bp), %bx
    movb %es:(%bx), %al
    xorb %ah, %ah

    popw %bx
    popw %es
    popw %bp
    lret

    .data
xtos_old_int60_offset:
    .word 0
xtos_old_int60_segment:
    .word 0
xtos_int60_installed:
    .word 0
