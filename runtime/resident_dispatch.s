    .code16
    .text
    .global XtosInt60Dispatch

XtosInt60Dispatch:
    pushw %si
    pushw %di
    pushw %bp
    movw %sp, %bp
    pushw %es
    pushw %bx

    movw 10(%bp), %bx
    movw 12(%bp), %ax
    movw %bx, %dx
    orw %ax, %dx
    jz .bad_pointer
    movw %ax, %es

    movw %es:(%bx), %ax
    cmpw $1, %ax
    je .ping
    cmpw $2, %ax
    je .log
    cmpw $4, %ax
    je .display_set_mode
    cmpw $5, %ax
    je .display_current_mode
    cmpw $6, %ax
    je .display_set_palette
    cmpw $7, %ax
    je .display_current_palette
    cmpw $8, %ax
    je .get_event
    cmpw $9, %ax
    je .prefs_load
    cmpw $10, %ax
    je .prefs_save
    cmpw $11, %ax
    je .prefs_current
    cmpw $12, %ax
    je .prefs_apply
    cmpw $13, %ax
    je .canvas_clear
    cmpw $14, %ax
    je .canvas_clear_rect
    cmpw $15, %ax
    je .canvas_rect
    cmpw $16, %ax
    je .canvas_dotted_rect
    cmpw $17, %ax
    je .canvas_fill_rect
    cmpw $18, %ax
    je .canvas_text
    cmpw $19, %ax
    je .canvas_text_width
    cmpw $20, %ax
    je .canvas_present
    cmpw $21, %ax
    je .restore_text
    cmpw $22, %ax
    je .status
    cmpw $23, %ax
    je .selftest
    cmpw $24, %ax
    je .font_count
    cmpw $25, %ax
    je .font_name
    cmpw $26, %ax
    je .font_width
    cmpw $27, %ax
    je .font_height
    cmpw $28, %ax
    je .font_glyph_count
    cmpw $29, %ax
    je .font_codepoint_at
    cmpw $30, %ax
    je .font_glyph_width_at
    cmpw $31, %ax
    je .pump_events
    cmpw $32, %ax
    je .mouse_init
    cmpw $33, %ax
    je .mouse_present
    cmpw $34, %ax
    je .cursor_show
    cmpw $35, %ax
    je .cursor_hide
    cmpw $36, %ax
    je .cursor_reset
    cmpw $37, %ax
    je .exec_request
    cmpw $38, %ax
    je .exec_get_next
    cmpw $39, %ax
    je .exec_clear_next
    cmpw $40, %ax
    je .uninstall
    jmp .unknown

.ping:
    call .write_status
    xorw %ax, %ax
    jmp .set_result

.log:
    movw %es:12(%bx), %ax
    orw %es:14(%bx), %ax
    jz .bad_parameter
    xorw %ax, %ax
    jmp .set_result

.display_set_mode:
    call .read_int_in_word
    jc .bad_parameter
    pushw %bx
    pushw %ax
    lcall $RuntimeDisplaySetMode@OZSEG16, $RuntimeDisplaySetMode
    addw $2, %sp
    popw %bx
    call .restore_pb_es
    xorw %ax, %ax
    jmp .set_result

.display_current_mode:
    call .check_int_out
    jc .bad_parameter
    pushw %bx
    lcall $RuntimeDisplayCurrentMode@OZSEG16, $RuntimeDisplayCurrentMode
    popw %bx
    call .restore_pb_es
    call .write_int_out_word
    xorw %ax, %ax
    jmp .set_result

.display_set_palette:
    call .read_int_in_word
    jc .bad_parameter
    pushw %bx
    pushw %ax
    lcall $RuntimeDisplaySetPalette@OZSEG16, $RuntimeDisplaySetPalette
    addw $2, %sp
    popw %bx
    call .restore_pb_es
    xorw %ax, %ax
    jmp .set_result

.display_current_palette:
    call .check_int_out
    jc .bad_parameter
    pushw %bx
    lcall $RuntimeDisplayCurrentPalette@OZSEG16, $RuntimeDisplayCurrentPalette
    popw %bx
    call .restore_pb_es
    call .write_int_out_word
    xorw %ax, %ax
    jmp .set_result

.get_event:
    call .check_addr_out
    jc .bad_parameter
    call .check_int_out
    jc .bad_parameter
    pushw %bx
    pushw $resident_event_temp
    lcall $EventGet@OZSEG16, $EventGet
    addw $2, %sp
    popw %bx
    call .restore_pb_es
    call .write_int_out_word
    cmpw $0, %ax
    je .get_event_done
    call .copy_event_to_addr_out
.get_event_done:
    xorw %ax, %ax
    jmp .set_result

.prefs_load:
    call .check_addr_out
    jc .bad_parameter
    call .check_int_out
    jc .bad_parameter
    movw $resident_prefs_temp, %ax
    pushw %bx
    pushw %ax
    lcall $RuntimeSystemPrefsLoad@OZSEG16, $RuntimeSystemPrefsLoad
    addw $2, %sp
    popw %bx
    call .restore_pb_es
    call .write_int_out_word
    movw resident_prefs_temp, %ax
    call .write_addr_out_word
    xorw %ax, %ax
    jmp .set_result

.prefs_save:
    call .read_addr_in_word
    jc .bad_parameter
    movw %ax, resident_prefs_temp
    call .check_int_out
    jc .bad_parameter
    movw $resident_prefs_temp, %ax
    pushw %bx
    pushw %ax
    lcall $RuntimeSystemPrefsSave@OZSEG16, $RuntimeSystemPrefsSave
    addw $2, %sp
    popw %bx
    call .restore_pb_es
    call .write_int_out_word
    xorw %ax, %ax
    jmp .set_result

.prefs_current:
    call .check_addr_out
    jc .bad_parameter
    pushw %bx
    lcall $RuntimeSystemPrefsCurrent@OZSEG16, $RuntimeSystemPrefsCurrent
    popw %bx
    call .restore_pb_es
    movw %ax, %di
    movw (%di), %ax
    call .write_addr_out_word
    xorw %ax, %ax
    jmp .set_result

.prefs_apply:
    call .read_addr_in_word
    jc .bad_parameter
    movw %ax, resident_prefs_temp
    movw $resident_prefs_temp, %ax
    pushw %bx
    pushw %ax
    lcall $RuntimeSystemPrefsApply@OZSEG16, $RuntimeSystemPrefsApply
    addw $2, %sp
    popw %bx
    call .restore_pb_es
    xorw %ax, %ax
    jmp .set_result

.canvas_clear:
    pushw %bx
    lcall $RuntimeCanvasClear@OZSEG16, $RuntimeCanvasClear
    popw %bx
    call .restore_pb_es
    xorw %ax, %ax
    jmp .set_result

.canvas_clear_rect:
    movw $4, %cx
    movw $resident_canvas_args, %di
    call .load_int_in_words
    jc .bad_parameter
    pushw %bx
    pushw resident_canvas_args+6
    pushw resident_canvas_args+4
    pushw resident_canvas_args+2
    pushw resident_canvas_args
    lcall $RuntimeCanvasClearRect@OZSEG16, $RuntimeCanvasClearRect
    addw $8, %sp
    popw %bx
    call .restore_pb_es
    xorw %ax, %ax
    jmp .set_result

.canvas_rect:
    movw $5, %cx
    movw $resident_canvas_args, %di
    call .load_int_in_words
    jc .bad_parameter
    pushw %bx
    pushw resident_canvas_args+8
    pushw resident_canvas_args+6
    pushw resident_canvas_args+4
    pushw resident_canvas_args+2
    pushw resident_canvas_args
    lcall $RuntimeCanvasRect@OZSEG16, $RuntimeCanvasRect
    addw $10, %sp
    popw %bx
    call .restore_pb_es
    xorw %ax, %ax
    jmp .set_result

.canvas_dotted_rect:
    movw $5, %cx
    movw $resident_canvas_args, %di
    call .load_int_in_words
    jc .bad_parameter
    pushw %bx
    pushw resident_canvas_args+8
    pushw resident_canvas_args+6
    pushw resident_canvas_args+4
    pushw resident_canvas_args+2
    pushw resident_canvas_args
    lcall $RuntimeCanvasDottedRect@OZSEG16, $RuntimeCanvasDottedRect
    addw $10, %sp
    popw %bx
    call .restore_pb_es
    xorw %ax, %ax
    jmp .set_result

.canvas_fill_rect:
    movw $5, %cx
    movw $resident_canvas_args, %di
    call .load_int_in_words
    jc .bad_parameter
    pushw %bx
    pushw resident_canvas_args+8
    pushw resident_canvas_args+6
    pushw resident_canvas_args+4
    pushw resident_canvas_args+2
    pushw resident_canvas_args
    lcall $RuntimeCanvasFillRect@OZSEG16, $RuntimeCanvasFillRect
    addw $10, %sp
    popw %bx
    call .restore_pb_es
    xorw %ax, %ax
    jmp .set_result

.canvas_text:
    movw $4, %cx
    movw $resident_canvas_args, %di
    call .load_int_in_words
    jc .bad_parameter
    movw %es:12(%bx), %ax
    orw %es:14(%bx), %ax
    jz .bad_parameter
    pushw %bx
    pushw %es:14(%bx)
    pushw %es:12(%bx)
    pushw resident_canvas_args+6
    pushw resident_canvas_args+4
    pushw resident_canvas_args+2
    pushw resident_canvas_args
    lcall $RuntimeCanvasTextId@OZSEG16, $RuntimeCanvasTextId
    addw $12, %sp
    popw %bx
    call .restore_pb_es
    xorw %ax, %ax
    jmp .set_result

.canvas_text_width:
    movw $1, %cx
    movw $resident_canvas_args, %di
    call .load_int_in_words
    jc .bad_parameter
    call .check_int_out
    jc .bad_parameter
    movw %es:12(%bx), %ax
    orw %es:14(%bx), %ax
    jz .bad_parameter
    pushw %bx
    pushw %es:14(%bx)
    pushw %es:12(%bx)
    pushw resident_canvas_args
    lcall $RuntimeCanvasTextWidthId@OZSEG16, $RuntimeCanvasTextWidthId
    addw $6, %sp
    popw %bx
    call .restore_pb_es
    call .write_int_out_word
    xorw %ax, %ax
    jmp .set_result

.canvas_present:
    pushw %bx
    lcall $RuntimeCanvasPresent@OZSEG16, $RuntimeCanvasPresent
    popw %bx
    call .restore_pb_es
    xorw %ax, %ax
    jmp .set_result

.restore_text:
    movw $3, %ax
    pushw %bx
    pushw %ax
    lcall $CgaSetVideoMode@OZSEG16, $CgaSetVideoMode
    addw $2, %sp
    popw %bx
    call .restore_pb_es
    xorw %ax, %ax
    jmp .set_result

.status:
    call .write_status
    xorw %ax, %ax
    jmp .set_result

.selftest:
    call .write_status
    call .write_ready
    xorw %ax, %ax
    jmp .set_result

.font_count:
    call .check_int_out
    jc .bad_parameter
    pushw %bx
    lcall $RuntimeFontCount@OZSEG16, $RuntimeFontCount
    popw %bx
    call .restore_pb_es
    call .write_int_out_word
    xorw %ax, %ax
    jmp .set_result

.font_name:
    movw $2, %cx
    movw $resident_canvas_args, %di
    call .load_int_in_words
    jc .bad_parameter
    call .check_addr_out
    jc .bad_parameter
    pushw %bx
    pushw resident_canvas_args
    lcall $RuntimeFontName@OZSEG16, $RuntimeFontName
    addw $2, %sp
    popw %bx
    call .restore_pb_es
    call .copy_near_string_to_addr_out
    xorw %ax, %ax
    jmp .set_result

.font_width:
    movw $1, %cx
    movw $resident_canvas_args, %di
    call .load_int_in_words
    jc .bad_parameter
    call .check_int_out
    jc .bad_parameter
    pushw %bx
    pushw resident_canvas_args
    lcall $RuntimeFontWidth@OZSEG16, $RuntimeFontWidth
    addw $2, %sp
    popw %bx
    call .restore_pb_es
    call .write_int_out_word
    xorw %ax, %ax
    jmp .set_result

.font_height:
    movw $1, %cx
    movw $resident_canvas_args, %di
    call .load_int_in_words
    jc .bad_parameter
    call .check_int_out
    jc .bad_parameter
    pushw %bx
    pushw resident_canvas_args
    lcall $RuntimeFontHeight@OZSEG16, $RuntimeFontHeight
    addw $2, %sp
    popw %bx
    call .restore_pb_es
    call .write_int_out_word
    xorw %ax, %ax
    jmp .set_result

.font_glyph_count:
    movw $1, %cx
    movw $resident_canvas_args, %di
    call .load_int_in_words
    jc .bad_parameter
    call .check_int_out
    jc .bad_parameter
    pushw %bx
    pushw resident_canvas_args
    lcall $RuntimeFontGlyphCount@OZSEG16, $RuntimeFontGlyphCount
    addw $2, %sp
    popw %bx
    call .restore_pb_es
    call .write_int_out_word
    xorw %ax, %ax
    jmp .set_result

.font_codepoint_at:
    movw $2, %cx
    movw $resident_canvas_args, %di
    call .load_int_in_words
    jc .bad_parameter
    call .check_int_out
    jc .bad_parameter
    pushw %bx
    pushw resident_canvas_args+2
    pushw resident_canvas_args
    lcall $RuntimeFontCodepointAt@OZSEG16, $RuntimeFontCodepointAt
    addw $4, %sp
    popw %bx
    call .restore_pb_es
    call .write_int_out_word
    xorw %ax, %ax
    jmp .set_result

.font_glyph_width_at:
    movw $2, %cx
    movw $resident_canvas_args, %di
    call .load_int_in_words
    jc .bad_parameter
    call .check_int_out
    jc .bad_parameter
    pushw %bx
    pushw resident_canvas_args+2
    pushw resident_canvas_args
    lcall $RuntimeFontGlyphWidthAt@OZSEG16, $RuntimeFontGlyphWidthAt
    addw $4, %sp
    popw %bx
    call .restore_pb_es
    call .write_int_out_word
    xorw %ax, %ax
    jmp .set_result

.pump_events:
    pushw %bx
    lcall $RuntimePumpEvents@OZSEG16, $RuntimePumpEvents
    popw %bx
    call .restore_pb_es
    xorw %ax, %ax
    jmp .set_result

.mouse_init:
    pushw %bx
    lcall $RuntimeMouseInit@OZSEG16, $RuntimeMouseInit
    popw %bx
    call .restore_pb_es
    xorw %ax, %ax
    jmp .set_result

.mouse_present:
    call .check_int_out
    jc .bad_parameter
    pushw %bx
    lcall $RuntimeMousePresent@OZSEG16, $RuntimeMousePresent
    popw %bx
    call .restore_pb_es
    call .write_int_out_word
    xorw %ax, %ax
    jmp .set_result

.cursor_show:
    pushw %bx
    lcall $MouseCursorShow@OZSEG16, $MouseCursorShow
    popw %bx
    call .restore_pb_es
    xorw %ax, %ax
    jmp .set_result

.cursor_hide:
    pushw %bx
    lcall $MouseCursorHide@OZSEG16, $MouseCursorHide
    popw %bx
    call .restore_pb_es
    xorw %ax, %ax
    jmp .set_result

.cursor_reset:
    pushw %bx
    lcall $MouseCursorReset@OZSEG16, $MouseCursorReset
    popw %bx
    call .restore_pb_es
    xorw %ax, %ax
    jmp .set_result

.exec_request:
    movw %es:12(%bx), %ax
    orw %es:14(%bx), %ax
    jz .bad_parameter
    pushw %bx
    pushw %es:14(%bx)
    pushw %es:12(%bx)
    lcall $RuntimeExecRequestFar@OZSEG16, $RuntimeExecRequestFar
    addw $4, %sp
    popw %bx
    call .restore_pb_es
    cmpw $0, %ax
    je .bad_parameter
    xorw %ax, %ax
    jmp .set_result

.exec_get_next:
    movw $1, %cx
    movw $resident_canvas_args, %di
    call .load_int_in_words
    jc .bad_parameter
    call .check_int_out
    jc .bad_parameter
    call .check_addr_out
    jc .bad_parameter
    pushw %bx
    pushw resident_canvas_args
    pushw $resident_exec_path
    lcall $RuntimeExecNext@OZSEG16, $RuntimeExecNext
    addw $4, %sp
    popw %bx
    call .restore_pb_es
    call .write_int_out_word
    movw resident_canvas_args, %ax
    movw %ax, resident_canvas_args+2
    movw $resident_exec_path, %ax
    call .copy_near_string_to_addr_out
    xorw %ax, %ax
    jmp .set_result

.exec_clear_next:
    pushw %bx
    lcall $RuntimeExecClear@OZSEG16, $RuntimeExecClear
    popw %bx
    call .restore_pb_es
    xorw %ax, %ax
    jmp .set_result

.uninstall:
    call .check_int_out
    jc .bad_parameter
    movw $3, %ax
    pushw %bx
    pushw %ax
    lcall $CgaSetVideoMode@OZSEG16, $CgaSetVideoMode
    addw $2, %sp
    lcall $XtosInt60ResidentUninstall@OZSEG16, $XtosInt60ResidentUninstall
    popw %bx
    call .restore_pb_es
    call .write_uninstall_reason
    cmpw $0, %ax
    je .uninstall_ok
    jmp .set_result
.uninstall_ok:
    xorw %ax, %ax
    jmp .set_result

.unknown:
    movw $1, %ax
    jmp .set_result

.bad_parameter:
    movw $2, %ax
    jmp .set_result

.bad_pointer:
    movw $2, %ax
    jmp .return

.set_result:
    movw %ax, %es:2(%bx)
.return:
    popw %bx
    popw %es
    movw %bp, %sp
    popw %bp
    popw %di
    popw %si
    lret

.write_status:
    movw %es:8(%bx), %di
    movw %es:10(%bx), %ax
    movw %di, %dx
    orw %ax, %dx
    jz .write_status_done
    pushw %es
    movw %ax, %es
    movw $0x5854, %es:(%di)
    movw $1, %es:2(%di)
    popw %es
.write_status_done:
    ret

.write_ready:
    movw %es:8(%bx), %di
    movw %es:10(%bx), %ax
    movw %di, %dx
    orw %ax, %dx
    jz .write_ready_done
    pushw %es
    movw %ax, %es
    movw $1, %es:4(%di)
    popw %es
.write_ready_done:
    ret

.restore_pb_es:
    pushw %ax
    movw 12(%bp), %ax
    movw %ax, %es
    popw %ax
    ret

.read_int_in_word:
    movw %es:4(%bx), %di
    movw %es:6(%bx), %ax
    movw %di, %dx
    orw %ax, %dx
    stc
    jz .read_int_in_done
    pushw %es
    movw %ax, %es
    movw %es:(%di), %ax
    popw %es
    clc
.read_int_in_done:
    ret

.check_int_out:
    movw %es:8(%bx), %di
    movw %es:10(%bx), %ax
    movw %di, %dx
    orw %ax, %dx
    stc
    jz .check_int_out_done
    clc
.check_int_out_done:
    ret

.write_int_out_word:
    movw %es:8(%bx), %di
    movw %es:10(%bx), %dx
    pushw %es
    movw %dx, %es
    movw %ax, %es:(%di)
    popw %es
    ret

.write_uninstall_reason:
    movw %es:8(%bx), %di
    movw %es:10(%bx), %dx
    pushw %es
    movw %dx, %es
    movw %ax, %es:(%di)
    popw %es
    ret

.read_addr_in_word:
    movw %es:12(%bx), %di
    movw %es:14(%bx), %ax
    movw %di, %dx
    orw %ax, %dx
    stc
    jz .read_addr_in_done
    pushw %es
    movw %ax, %es
    movw %es:(%di), %ax
    popw %es
    clc
.read_addr_in_done:
    ret

.check_addr_out:
    movw %es:16(%bx), %di
    movw %es:18(%bx), %ax
    movw %di, %dx
    orw %ax, %dx
    stc
    jz .check_addr_out_done
    clc
.check_addr_out_done:
    ret

.write_addr_out_word:
    movw %es:16(%bx), %di
    movw %es:18(%bx), %dx
    pushw %es
    movw %dx, %es
    movw %ax, %es:(%di)
    popw %es
    ret

.load_int_in_words:
    movw %es:4(%bx), %si
    movw %es:6(%bx), %ax
    movw %si, %dx
    orw %ax, %dx
    stc
    jz .load_int_in_words_done
    pushw %es
    movw %ax, %es
.load_int_in_words_loop:
    movw %es:(%si), %ax
    movw %ax, (%di)
    addw $2, %si
    addw $2, %di
    loop .load_int_in_words_loop
    popw %es
    clc
.load_int_in_words_done:
    ret

.copy_near_string_to_addr_out:
    movw %ax, %si
    movw resident_canvas_args+2, %cx
    movw %es:16(%bx), %di
    movw %es:18(%bx), %dx
    pushw %es
    movw %dx, %es
    cmpw $0, %cx
    je .copy_near_string_done
    decw %cx
.copy_near_string_loop:
    cmpw $0, %cx
    je .copy_near_string_truncate
    movb (%si), %al
    incw %si
    movb %al, %es:(%di)
    incw %di
    decw %cx
    testb %al, %al
    jne .copy_near_string_loop
    jmp .copy_near_string_done
.copy_near_string_truncate:
    movb $0, %es:(%di)
.copy_near_string_done:
    popw %es
    ret

.copy_event_to_addr_out:
    movw %es:16(%bx), %di
    movw %es:18(%bx), %dx
    pushw %es
    movw %dx, %es
    movw resident_event_temp, %ax
    movw %ax, %es:(%di)
    movw resident_event_temp+2, %ax
    movw %ax, %es:2(%di)
    movw resident_event_temp+4, %ax
    movw %ax, %es:4(%di)
    movw resident_event_temp+6, %ax
    movw %ax, %es:6(%di)
    movw resident_event_temp+8, %ax
    movw %ax, %es:8(%di)
    popw %es
    ret

    .data
resident_prefs_temp:
    .word 0
resident_canvas_args:
    .word 0, 0, 0, 0, 0
resident_event_temp:
    .word 0, 0, 0, 0, 0
resident_exec_path:
    .space 64, 0
