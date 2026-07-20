; ---------------------------------------------------------------------------------------
; Copyright (c) 2019-2026 luke8086
; Distributed under the terms of GPL-2 License
; ---------------------------------------------------------------------------------------
; File: start.s - startup code
; ---------------------------------------------------------------------------------------

[cpu 8086]

extern _krn_main

global _krn_marker_text_start
global _krn_marker_text_end
global _krn_marker_data_start
global _krn_marker_data_end
global _krn_marker_bss_start
global _krn_marker_bss_end

global _krn_main_segment
global _krn_magic_number

section _TEXT class=CODE
_krn_marker_text_start:

resb 0x100
..start:
    mov [_krn_main_segment], ds
    jmp _krn_main

section _TEXTEND class=ENDCODE
_krn_marker_text_end:

section _DATA class=DATA
_krn_marker_data_start:

_krn_main_segment:
    dw 0

section _DATAEND class=DATAEND
_krn_magic_number:
    dq 0xf0cacc1a
_krn_marker_data_end:

section _BSS class=BSS
_krn_marker_bss_start:

section _BSSEND class=BSSEND
_krn_marker_bss_end:

group DGROUP _TEXT _TEXTEND _DATA _DATAEND _BSS _BSSEND
