OUTPUT_ARCH(or32)
__DYNAMIC  =  0;

MEMORY
{
	vectors : ORIGIN = 0, LENGTH = 0x2000
	ram     : ORIGIN = 0x2000, LENGTH = (128M - 0x2000)
}

SECTIONS
{	
	.vectors :
	{
		*(.vectors)
	} > vectors
	.text : {
		_stext = .;
		*(.text)
		_etext = .;
		*(.lit)
		*(.shdata)
		_endtext = .;
	}  > ram

	 __u_boot_cmd_start = .;
	 .u_boot_cmd : { *(.u_boot_cmd) } > ram
	 __u_boot_cmd_end = .;

	.rodata : {
		*(.rodata);
		*(.rodata.*)
	} > ram

	.shbss :
	{
		*(.shbss)
	} > ram

	.talias :
	{
	}  > ram

	.data : {
		sdata = .;
		_sdata = .;
		*(.data)
		edata = .;
		_edata = .;
	} > ram

	.bss :
	{
		_bss_start = .;
		*(.bss)
		*(COMMON)
		_bss_end = .;
	} > ram

	/* No stack specification - done manually */

	.stab  0 (NOLOAD) :
	{
		[ .stab ]
	}

	.stabstr  0 (NOLOAD) :
	{
		[ .stabstr ]
	}
}
