//7 has words
U0 Song()
{
  CTask *task;
  Fs->task_end_cb=&SndTaskEndCB;
  music_tempo=2.5;
  try {
    task=PopUpViewStr(
	  "[C:/Misc/Bible.TXT.Z,81941]\n"
	  "\n"
	  "Prodigal Son\n"
	  "[C:/Misc/Bible.TXT.Z,81941]\n"
	  );
    TaskWait(task);
    SetWinHorz(Fs->win_left,Fs->win_right,task);
    SetWinVert(Fs->win_bottom+3,Fs->win_bottom+15,task);
    "$WW+H,1$";
    while (!ScanKey) {
      Play("3qGGsEAEAqAsF2G3F2G3qCeEFqD",
	    "Life, \0life, \0isn't \0 \0 \0 \0fair.  \0Isn't \0 \0 \0 \0fair.\n\0 \0 \0 \0");
      Play("GGsEAEAqAsF2G3F2G3qCeEFqD",
	    "Waaa, \0waaa, \0isn't \0 \0 \0 \0fair.  \0Isn't \0 \0 \0 \0fair.\n\0 \0 \0 \0");
      Play("eFCqEBDBE2eG3EsFEFE",
	    " \0 \0I \0 \0ne\0ver \0sin.\n\0 \0 \0 \0 \0 \0 \0");
      Play("eFCqEBDBE2eG3EsFEFE",
	    " \0 \0I \0 \0ne\0ver \0win.\n\0 \0 \0 \0 \0 \0 \0");
      "$GREEN$(A warning on presumption. 'Who loves more man -- forgiven big debt or little?'  See prodigal son.)$FG$\n";
    }
  } catch
    PutExcept;
  Snd(0);
}

Song;
