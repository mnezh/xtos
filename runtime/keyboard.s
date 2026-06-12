    .code16
    .text
    .global KeyboardHasKey
    .global KeyboardRead

KeyboardHasKey:
    pushw %bp
    movw %sp, %bp
    pushw %ds
    pushw %es

    movb $1, %ah
    int $0x16
    jz .no_key

    movw $1, %ax
    jmp .done

.no_key:
    xorw %ax, %ax

.done:
    popw %es
    popw %ds
    popw %bp
    lret

KeyboardRead:
    pushw %bp
    movw %sp, %bp
    pushw %ds
    pushw %es

    xorw %ax, %ax
    int $0x16
    testb %al, %al
    jz .read_done
    xorb %ah, %ah

.read_done:
    popw %es
    popw %ds
    popw %bp
    lret
