//See [C:/Demo/Games/Stadium/StadiumGen.CPP.Z,1].

U0 ViewScreenCapture(U8 *filename)
{
  CDC	*dc=DCAlias(,Fs),
	*dc2=GRARead(filename);
  GrBlot(dc,0,0,dc2);
  DCDel(dc2);
  DCDel(dc);
}

U0 Main()
{
  "Capture screen...\n";
  PressAKey;
  GRACaptureScreen("::/Home/ScreenShot");

  "View captured screen...\n";
  PressAKey;
  ViewScreenCapture("::/Home/ScreenShot");

  PressAKey;
  DCFill;
}

Main;
