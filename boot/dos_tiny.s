    .code16
    .text
    .global XtosDosPrintLine
    .global XtosDosLogLine
    .global XtosDosLogLine2

XtosDosPrintLine:
    pushw %bp
    movw %sp, %bp
    pushw %bx
    movw $1, %bx
    movw 4(%bp), %dx
    call dos_write_cstr
    movw $dos_newline, %dx
    call dos_write_fixed_newline
    popw %bx
    popw %bp
    ret

XtosDosLogLine:
    pushw %bp
    movw %sp, %bp
    pushw %bx
    pushw %dx

    call dos_log_open
    jc .log_line_done
    movw %ax, %bx
    movw 4(%bp), %dx
    call dos_write_cstr
    movw $dos_newline, %dx
    call dos_write_fixed_newline
    call dos_close

.log_line_done:
    popw %dx
    popw %bx
    popw %bp
    ret

XtosDosLogLine2:
    pushw %bp
    movw %sp, %bp
    pushw %bx
    pushw %dx

    call dos_log_open
    jc .log_line2_done
    movw %ax, %bx
    movw 4(%bp), %dx
    call dos_write_cstr
    movw $dos_space, %dx
    call dos_write_fixed_space
    movw 6(%bp), %dx
    call dos_write_cstr
    movw $dos_newline, %dx
    call dos_write_fixed_newline
    call dos_close

.log_line2_done:
    popw %dx
    popw %bx
    popw %bp
    ret

dos_log_open:
    movw $dos_log_file, %dx
    movw $0x3d01, %ax
    int $0x21
    jnc .log_seek_end

    xorw %cx, %cx
    movw $dos_log_file, %dx
    movw $0x3c00, %ax
    int $0x21
    jc .log_open_done

.log_seek_end:
    movw %ax, %bx
    xorw %cx, %cx
    xorw %dx, %dx
    movw $0x4202, %ax
    int $0x21
    movw %bx, %ax
    clc

.log_open_done:
    ret

dos_close:
    movb $0x3e, %ah
    int $0x21
    ret

dos_write_cstr:
    pushw %ax
    pushw %cx
    pushw %si

    movw %dx, %si
    xorw %cx, %cx
.strlen_loop:
    cmpb $0, (%si)
    je .strlen_done
    incw %si
    incw %cx
    jmp .strlen_loop

.strlen_done:
    cmpw $0, %cx
    je .write_done
    movb $0x40, %ah
    int $0x21

.write_done:
    popw %si
    popw %cx
    popw %ax
    ret

dos_write_fixed_space:
    pushw %ax
    pushw %cx
    movw $1, %cx
    movb $0x40, %ah
    int $0x21
    popw %cx
    popw %ax
    ret

dos_write_fixed_newline:
    pushw %ax
    pushw %cx
    movw $2, %cx
    movb $0x40, %ah
    int $0x21
    popw %cx
    popw %ax
    ret

    .data
dos_log_file:
    .asciz "XTOS.LOG"
dos_newline:
    .byte 13, 10
dos_space:
    .byte ' '
