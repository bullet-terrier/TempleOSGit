#help_index "Define;Char/Define"

U0 LoadDocDefines()
{
  CBinFile *bfh=sys_boot_base-sizeof(CBinFile);

  DefinePrint("DD_OS_NAME_VERSION","TempleOS V%0.2f",os_version);
  DefinePrint("DD_TEMPLEOS_AGE","%0.1f",
	(Now-Str2Date("8/1/2003"))/ToF64(1<<32)/CDATE_YEAR_DAYS);

  //This is cut and replaced when I generate a distro.
  //See [C:/Demo/AcctExample/TOSDistro.CPP,1] DD_TEMPLEOS_LOC.
  -] LineRep
      DefinePrint("DD_TEMPLEOS_LOC","122,806");
    DefinePrint("DD_TEMPLEOS_LOC_OFFICIAL","80,870");
    
  DefinePrint("DD_KERNEL","%08X",bfh);
  bfh(I64)+=bfh->file_size-1;
  DefinePrint("DD_KERNEL_END","%08X",bfh);
  DefinePrint("DD_PROTECTED_LOW","%08X",MEM_PROTECTED_LOW_LIMIT);

  //[C:/Adam/Boot/BootCD.CPP.Z,1] DD_BOOT_HIGH_LOC_CD

  DefinePrint("DD_MP_VECT","%08X",MP_VECT_ADDR);
  DefinePrint("DD_MP_VECT_END","%08X",
	MP_VECT_ADDR+AP_16BIT_INIT_END-AP_16BIT_INIT-1);

  DefinePrint("DD_SYS_FIXED_AREA_BASE","%08X",SYS_FIXED_AREA);
  DefinePrint("DD_SYS_FIXED_AREA_END","%08X",
	SYS_FIXED_AREA+sizeof(CSysFixedArea)-1);
  DefinePrint("DD_SYS_HEAP_BASE","%08X",sys_heap_base);
  DefinePrint("DD_SYS_HEAP_LIMIT","%08X",sys_heap_limit);
  DefinePrint("DD_MEM_MIN_MEG","%d Meg",MEM_MIN_MEG);
  DefinePrint("DD_MEM_MAPPED_SPACE_GIG","%d Gig",MEM_MAPPED_SPACE/0x40000000);
  DefinePrint("DD_MEM_MAPPED_SPACE_END","%010X",MEM_MAPPED_SPACE-1);
  DefinePrint("DD_JIFFY_HZ","%d Hz",JIFFY_FREQ);
}

LoadDocDefines;
