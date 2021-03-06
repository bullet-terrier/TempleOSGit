//7 has graphics

#define X	330
#define Y	15

#define NUM_BEAMS	256

F64 beams_r[NUM_BEAMS],beams_theta[NUM_BEAMS];

U0 DrawIt(CTask *,CDC *dc)
{
  I64 i;
  F64 r,x1,y1,x2,y2;
  dc->color=YELLOW;
  GrCircle(dc,X,Y,10);
  GrFloodFill(dc,X,Y,10);
  dc->color=BROWN;
  GrCircle(dc,X,Y,10);
  dc->color=YELLOW;
  for (i=0;i<NUM_BEAMS;i++) {
    r=125.0*((0.25*Beat+beams_r[i])%1.0);
    P2R(&x1,&y1,r     ,beams_theta[i]);
    P2R(&x2,&y2,r+10.0,beams_theta[i]);
    GrLine(dc,x1+X,y1+Y,x2+X,y2+Y);
  }
}

U0 Song()
{
  I64 i;
  CTask *task;
  SettingsPush; //See [C:/Adam/TaskSettings.CPP.Z,3] SettingsPush
  Fs->text_attr=LTCYAN<<4+BLACK;
  Fs->draw_it=&DrawIt;
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 2.480;
  music_stacatto_factor= 0.902;
  for (i=0;i<NUM_BEAMS;i++) {
    beams_r[i]=Rand;
    beams_theta[i]=2*pi*Rand;
  }
  try {
    task=PopUpViewStr(
	  "[C:/Misc/Bible.TXT.Z,12005] Numbers,6:25\n"
	  "[C:/Misc/Bible.TXT.Z,46190] Psalms,31:16\n"
	  "[C:/Misc/Bible.TXT.Z,46190] Psalms,67:1\n"
	  "[C:/Misc/Bible.TXT.Z,46190] Psalms,80:3\n"
	  "[C:/Misc/Bible.TXT.Z,46190] Psalms,80:19\n"
	  "[C:/Misc/Bible.TXT.Z,46190] Psalms,119:135\n"
	  "[C:/Misc/Bible.TXT.Z,71804] Daniel,9:17\n"
	  );
    TaskWait(task);
    SetWinHorz(Fs->win_left,Fs->win_right,task);
    SetWinVert(Fs->win_bottom+3,Fs->win_bottom+15,task);
    "$BG+H,LTCYAN$$FG+H,BLACK$";
    while (!ScanKey) {
      Play("2qG3eCB2hGqG3eCB2qG3A",
	    "Umm... \0God?\n\0 \0 \0Let \0Your \0 \0 \0 \0");
      Play("CeBA2G3DqEeDCBA2qG3A",
	    "face \0shine \0on \0down\n\0on \0us, \0Your \0 \0luck\0y \0 \0peo\0");
      Play("eBA2h.G3qECAF",
	    " \0ple.\n\0 \0Fath\0e\0r \0a\0");
      Play("FDAeEDqECAF",
	    "bove,\n\0 \0 \0I \0 \0feel \0 \0 \0Your \0");
      Play("FDAeED",
	    "love.\n\0 \0 \0 \0 \0");
    }
  } catch
    PutExcept;
  SettingsPop;
}

Song;
