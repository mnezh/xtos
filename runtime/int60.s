    .code16
    .text
    .global XtosInt60Install
    .global XtosInt60UseResidentDataSegment
    .global XtosInt60Restore
    .global XtosInt60Handler
    .global XtosInt60CallResident
    .global XtosInt60RestoreDataSegment
    .global XtosCopyFarString
    .global RuntimeCgaReadByte
    .extern __ia16_near_data_segment

XtosInt60UseResidentDataSegment:
    pushw %bp
    movw %sp, %bp
    movw $1, %cs:xtos_int60_use_resident_ds
    popw %bp
    lret

XtosInt60Install:
    pushw %bp
    movw %sp, %bp
    pushw %ds
    pushw %es
    pushw %bx
    pushw %dx

    movw %cs:__ia16_near_data_segment, %ax
    movw %ax, %cs:xtos_int60_data_segment
    movw %ax, %ds
    movb $0x35, %ah
    movb $0x60, %al
    int $0x21
    movw %bx, xtos_old_int60_offset
    movw %es, xtos_old_int60_segment
    movw %bx, %cs:xtos_old_int60_offset_cs
    movw %es, %cs:xtos_old_int60_segment_cs
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

XtosInt60Handler:
    cmpw $0, %cs:xtos_int60_use_resident_ds
    jne .handler_resident_stack

    pushw %bp
    movw %sp, %bp
    pushw %ds
    pushw %es
    pushw %bx
    pushw %cx
    pushw %dx
    pushw %si
    pushw %di

    movw %ds, %ax
    pushw %ax
    pushw %dx
    lcall $XtosInt60Dispatch@OZSEG16, $XtosInt60Dispatch
    addw $4, %sp
    movw %ax, %cs:xtos_int60_dispatch_result

    popw %di
    popw %si
    popw %dx
    popw %cx
    popw %bx
    popw %es
    popw %ds
    popw %bp
    iret

.handler_resident_stack:
    cli
    movw %ss, %cs:xtos_int60_caller_ss
    movw %sp, %cs:xtos_int60_caller_sp
    movw %ds, %ax
    movw %ax, %cs:xtos_int60_caller_ds
    movw %cs:xtos_int60_data_segment, %ax
    movw %ax, %ds
    movw %ax, %ss
    movw $xtos_int60_stack_top, %sp
    sti

    pushw %bp
    movw %sp, %bp
    movw %cs:xtos_int60_caller_ds, %ax
    pushw %ax
    pushw %es
    pushw %bx
    pushw %cx
    pushw %dx
    pushw %si
    pushw %di

    movw %cs:xtos_int60_caller_ds, %ax
    pushw %ax
    pushw %dx
    lcall $XtosInt60Dispatch@OZSEG16, $XtosInt60Dispatch
    addw $4, %sp

    popw %di
    popw %si
    popw %dx
    popw %cx
    popw %bx
    popw %es
    popw %ax
    popw %bp

    cli
    movw %cs:xtos_int60_caller_ss, %ss
    movw %cs:xtos_int60_caller_sp, %sp
    movw %cs:xtos_int60_caller_ds, %ds
    movw %cs:xtos_int60_dispatch_result, %ax
    sti
    iret

XtosInt60CallResident:
    pushw %bp
    movw %sp, %bp
    pushw %ds
    pushw %dx

    cmpw $0, xtos_int60_installed
    je .resident_missing
    movw %cs:xtos_old_int60_offset_cs, %ax
    orw %cs:xtos_old_int60_segment_cs, %ax
    jz .resident_missing

    movw 6(%bp), %dx
    movw 8(%bp), %ds
    pushf
    lcall *%cs:xtos_old_int60_offset_cs
    jmp .resident_done

.resident_missing:
    movw $1, %ax

.resident_done:
    popw %dx
    popw %ds
    popw %bp
    lret

XtosInt60RestoreDataSegment:
    movw %cs:xtos_int60_data_segment, %ds
    lret

XtosCopyFarString:
    pushw %bp
    movw %sp, %bp
    pushw %ds
    pushw %es
    pushw %si
    pushw %di
    pushw %cx

    movw 6(%bp), %di
    pushw %ds
    popw %es
    movw 8(%bp), %si
    movw 10(%bp), %ds
    movw 12(%bp), %cx
    cmpw $0, %cx
    je .copy_far_string_done
    decw %cx

.copy_far_string_loop:
    cmpw $0, %cx
    je .copy_far_string_truncate
    movb (%si), %al
    incw %si
    movb %al, %es:(%di)
    incw %di
    decw %cx
    testb %al, %al
    jne .copy_far_string_loop
    jmp .copy_far_string_done

.copy_far_string_truncate:
    movb $0, %es:(%di)

.copy_far_string_done:
    movw 6(%bp), %ax
    popw %cx
    popw %di
    popw %si
    popw %es
    popw %ds
    popw %bp
    lret

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

xtos_old_int60_offset_cs:
    .word 0
xtos_old_int60_segment_cs:
    .word 0

xtos_int60_data_segment:
    .word 0

xtos_int60_use_resident_ds:
    .word 0

xtos_int60_caller_ss:
    .word 0
xtos_int60_caller_sp:
    .word 0
xtos_int60_caller_ds:
    .word 0
xtos_int60_dispatch_result:
    .word 0

    .data
xtos_old_int60_offset:
    .word 0
xtos_old_int60_segment:
    .word 0
xtos_int60_installed:
    .word 0

    .bss
    .align 2
xtos_int60_stack:
    .skip 1024
xtos_int60_stack_top:
