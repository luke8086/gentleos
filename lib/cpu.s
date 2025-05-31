; ---------------------------------------------------------------------------------------
; Copyright (c) 2014-2026 luke8086
; Distributed under the terms of GPL-2 License
; ---------------------------------------------------------------------------------------
; File: cpu.s - CPU-related functions in assembly
; ---------------------------------------------------------------------------------------

global cpu_get_eflags:function
cpu_get_eflags:
    pushfd
    pop eax
    ret

global cpu_set_eflags:function
cpu_set_eflags:
    mov eax, [esp + 4]
    push eax
    popfd
    ret

global cpu_cli:function
cpu_cli:
    cli
    ret

global cpu_hlt:function
cpu_hlt:
    hlt
    ret

global inb:function
inb:
    push ebp
    mov ebp, esp

    push edx
    mov dx, [ebp + 8]
    in al, dx
    pop edx

    mov esp, ebp
    pop ebp
    ret

global outb:function
outb:
    push ebp
    mov ebp, esp
    pusha

    mov dx, [ebp + 12]
    mov al, [ebp + 8]
    out dx, al

    popa
    mov esp, ebp
    pop ebp
    ret
