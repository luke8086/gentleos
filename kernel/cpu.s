; ---------------------------------------------------------------------------------------
; Copyright (c) 2014-2026 luke8086
; Distributed under the terms of GPL-2 License
; ---------------------------------------------------------------------------------------
; File: cpu.s - CPU-related functions in assembly
; ---------------------------------------------------------------------------------------

[cpu 8086]

section _TEXT class=CODE

global _krn_cpu_get_flags
_krn_cpu_get_flags:
    pushf
    pop ax
    ret

global _krn_cpu_set_flags
_krn_cpu_set_flags:
    push bp
    mov bp, sp
    mov ax, [bp + 4]
    push ax
    popf
    pop bp
    ret

global _krn_cpu_cli
_krn_cpu_cli:
    cli
    ret

global _krn_cpu_hlt
_krn_cpu_hlt:
    hlt
    ret

global _krn_inb
_krn_inb:
    push bp
    mov bp, sp

    push dx
    mov dx, [bp + 4]
    in al, dx
    pop dx

    mov sp, bp
    pop bp
    ret

global _krn_outb
_krn_outb:
    push bp
    mov bp, sp
    push dx
    push ax

    mov dx, [bp + 6]
    mov al, [bp + 4]
    out dx, al

    pop ax
    pop dx
    mov sp, bp
    pop bp
    ret


global _krn_intr
_krn_intr:
    push bp
    mov bp, sp

    push si
    push di
    push bp

    mov ax, [bp+4]
    mov byte [krn_intr_int+1], al
    jmp short $+2

    mov si, [bp+6]
    mov ax, [si+0]
    mov bx, [si+2]
    mov cx, [si+4]
    mov dx, [si+6]
    mov bp, [si+8]
    mov di, [si+10]
    mov si, [si+12]

krn_intr_int:
    int 0

    pop bp

    mov si, [bp+6]
    mov [si+0], ax
    mov [si+2], bx
    mov [si+4], cx
    mov [si+6], dx

    pushf
    pop word [si+14]

    pop di
    pop si

    pop bp
    ret
