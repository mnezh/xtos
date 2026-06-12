    .code16
    .text
    .global XtosDosExec

XtosDosExec:
    pushw %bp
    movw %sp, %bp
    pushw %es
    pushw %bx
    pushw %dx

    movw 4(%bp), %dx
    pushw %ds
    popw %es

    movw %ds, exec_cmd_tail_segment
    movw %ds, exec_fcb1_segment
    movw %ds, exec_fcb2_segment
    movw $exec_param_block, %bx
    movw $0x4b00, %ax
    int $0x21
    jc .done
    xorw %ax, %ax

.done:
    popw %dx
    popw %bx
    popw %es
    popw %bp
    ret

    .data
exec_cmd_tail:
    .byte 0
    .byte 13

exec_fcb1:
    .space 16, 0
exec_fcb2:
    .space 16, 0

exec_param_block:
    .word 0
    .word exec_cmd_tail
exec_cmd_tail_segment:
    .word 0
    .word exec_fcb1
exec_fcb1_segment:
    .word 0
    .word exec_fcb2
exec_fcb2_segment:
    .word 0
