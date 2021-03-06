U0 PaletteDemo()
{
  I64 i,j;
  CBGR48 bgr;
  for (i=0;i<NUM_COLORS;i++)
    "$BLACK$$BG,%d$__%-8Z__$FG$$BG$\n",i,i,"ST_COLORS";

  SettingsPush; //See [C:/Adam/TaskSettings.CPP.Z,3] SettingsPush
//Alternatively, use [C:/Adam/Gr/GrPalette.CPP.Z,83] GrPaletteSet(gr_palette_gray);
  for (i=0;i<NUM_COLORS;i++) {
    j=0xFFFF*i/(NUM_COLORS-1);
    bgr.b=j;
    bgr.g=j;
    bgr.r=j;
    GrPaletteColorSet(i,bgr);
  }
  PressAKey;

  //You can do it this way, too.
  GrPaletteColorSet(WHITE,0xFFFFFFFF0000);
  PressAKey;

  //[C:/Adam/TaskSettings.CPP.Z,88] SettingsPop() restores original palette
  SettingsPop;
}

PaletteDemo;
