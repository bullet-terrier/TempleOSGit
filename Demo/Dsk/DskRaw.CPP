U0 RawAccess(I64 drv_let=0)
{
  CDrv *dv=Let2Drv(drv_let);
  CBlkDev *bd=Let2BlkDev(drv_let);
  I64 blk,old_offset,old_size;
  U8 *buf=MAlloc(BLK_SIZE);

  DrvRep;
  PressAKey;

  ClassRep(dv);
  PressAKey;

  ClassRep(bd);
  PressAKey;

  //Temporarily change partition range to full device.
  //Normally, you are limited to just your partition.
  //This is obviously **dangerous**!
  old_offset=dv->base_offset;
  old_size  =dv->size;
  dv->base_offset=bd->min_blk;
  dv->size=bd->max_blk-bd->min_blk+1;

  try {
    blk=GetI64("Blk to Read: ",old_offset,bd->min_blk,bd->max_blk);
    RBlks(dv,buf,blk,1);
    D(buf,BLK_SIZE);
  } catch
    PutExcept;

  dv->base_offset=old_offset;
  dv->size  =old_size;

  Free(buf);
}

RawAccess;

//See [C:/Kernel/Dsk/DskA.CPP.Z,319] RBlks(), [C:/Kernel/Dsk/DskA.CPP.Z,356] WBlks(), [C:/Kernel/Dsk/DskA.CPP.Z,15] DrvLock(), [C:/Kernel/Dsk/DskA.CPP.Z,44] DrvUnlock()
//[C:/Adam/Dsk/ADskA.CPP.Z,3] DBlk(), [C:/Adam/Dsk/ADskA.CPP.Z,22] DCluster(), [C:/Kernel/Dsk/DskA.CPP.Z,1] BlkDevLock() and [C:/Kernel/Dsk/DskA.CPP.Z,28] BlkDevUnlock().
