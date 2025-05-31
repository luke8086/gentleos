OUTPUT_FORMAT(elf32-i386)
ENTRY(krn_core_entry)

krn_link_start = 0x00100000;

SECTIONS {

    . = krn_link_start;

    krn_link_start = .;

    .text : {
        krn_link_text_start = .;
        KEEP(*(.multiboot))
        *(.text)
    }
    krn_link_text_end = .;

    .data : {
        krn_link_data_start = .;
        *(.data)
        *(.rodata)
        *(.eh_frame)
    }
    krn_link_data_end = .;

    .bss : ALIGN(0x1000) {
        krn_link_bss_start = .;
        *(.bss)
    }
    krn_link_bss_end = .;

    krn_link_end = .;

    /DISCARD/ : {
        *(.comment)
    }

}
