    .code16
    .text
    .global ResidentDosOpenRead
    .global ResidentDosOpenAppend
    .global ResidentDosCreate
    .global ResidentDosRead
    .global ResidentDosWrite
    .global ResidentDosClose

ResidentDosOpenRead:
    pushw %bp
    movw %sp, %bp
    movw 6(%bp), %dx
    movw $0x3d00, %ax
    int $0x21
    jnc .open_read_done
    movw $0xffff, %ax
.open_read_done:
    popw %bp
    lret

ResidentDosOpenAppend:
    pushw %bp
    movw %sp, %bp
    pushw %bx
    pushw %cx
    pushw %dx
    movw 6(%bp), %dx
    movw $0x3d01, %ax
    int $0x21
    jnc .append_seek
    movw $0xffff, %ax
    jmp .append_done
.append_seek:
    movw %ax, %bx
    xorw %cx, %cx
    xorw %dx, %dx
    movw $0x4202, %ax
    int $0x21
    movw %bx, %ax
.append_done:
    popw %dx
    popw %cx
    popw %bx
    popw %bp
    lret

ResidentDosCreate:
    pushw %bp
    movw %sp, %bp
    xorw %cx, %cx
    movw 6(%bp), %dx
    movw $0x3c00, %ax
    int $0x21
    jnc .create_done
    movw $0xffff, %ax
.create_done:
    popw %bp
    lret

ResidentDosRead:
    pushw %bp
    movw %sp, %bp
    pushw %bx
    movw 6(%bp), %bx
    movw 8(%bp), %dx
    movw 10(%bp), %cx
    movb $0x3f, %ah
    int $0x21
    jnc .read_done
    movw $0xffff, %ax
.read_done:
    popw %bx
    popw %bp
    lret

ResidentDosWrite:
    pushw %bp
    movw %sp, %bp
    pushw %bx
    movw 6(%bp), %bx
    movw 8(%bp), %dx
    movw 10(%bp), %cx
    movb $0x40, %ah
    int $0x21
    jnc .write_done
    movw $0xffff, %ax
.write_done:
    popw %bx
    popw %bp
    lret

ResidentDosClose:
    pushw %bp
    movw %sp, %bp
    pushw %bx
    movw 6(%bp), %bx
    movb $0x3e, %ah
    int $0x21
    popw %bx
    popw %bp
    lret
