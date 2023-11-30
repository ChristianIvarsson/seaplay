OUTPUT_FORMAT ("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")

/*
80 should be well outside the ordinary vector locations
*/
MEMORY {
    ram   (rwx) : ORIGIN = 0x00000080, LENGTH = 32K
}

SECTIONS {
    .text : {
        _s_appBase = .;
        KEEP(*(.isr_vector .isr_vector.*))
        _e_appBase = .;

        . = ALIGN(4); 
        *(.text .text.*)

        . = ALIGN(4); 
        *(.rodata .rodata.* .rodata*)   
    } > ram

    . = ALIGN(4);

    .data : {
        *(.data .data.*)
        . = ALIGN(4);
    } > ram

    . = ALIGN(4);

    .bss (NOLOAD) : {
        _sbss = . ;
        *(.bss .bss.*)
        . = ALIGN(4);
        _ebss = . ;
    } > ram
}


