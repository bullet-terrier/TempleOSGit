//7 has graphics

/* <1> <1> (image) */






U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.902;
  try {
    while (!ScanKey) {
      "$GREEN$Jesus talking...$FG$\n";
      Play("2etG3BCqGeDBqC2G3sGAGAeEFsGGGG",
	    "It \0is \0 \0finished.\n\0 \0 \0 \0 \0"
	    " \0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
      '\n';
      Sprite(__BIN_1);
      "%h7c",'\n';
      Play("2etG3BCqGeDBqC2G3sGAGAeEFsGGGG",
	    "It \0is \0 \0finished.\n\0 \0 \0 \0 \0 \0"
	    " \0 \0 \0 \0 \0 \0 \0 \0 \0");
      Play("etGCFqDetBEB2G3DBeEAsEGEG2qGG",
	    "That \0 \0was \0rough!\n\0 \0 \0 \0 \0 \0"
	    " \0 \0 \0 \0 \0 \0 \0 \0 \0");
      Play("3etGCFqDetBEB2G3DBeEAsEGEG2qGG",
	    "But \0 \0I'm \0tough.\n\0 \0 \0 \0 \0 \0"
	    " \0 \0 \0 \0 \0 \0 \0 \0 \0");
    }
  } catch
    PutExcept;
  Snd(0);
}

Song;
