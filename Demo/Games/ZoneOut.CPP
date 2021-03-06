RegSetDftEntry("TempleOS/ZoneOut","F64 best_score=9999;\n");
RegExeBranch("TempleOS/ZoneOut");










		/* <1> <1> (image) */
























		/* <2> <2> (image) */













		   /* <3> <3> (image) */

/*  <4> (image) */










#define NUM_THEM	10

class Obj
{
  Obj *next,*last;
  F64 t0,theta;
  I64 x,y,z;
  Bool hit,pad[7];
} us,them[NUM_THEM],shots;
I64 num_them;
F64 t0,tf;

#define SCREEN_SCALE	512
#define TANK_HEIGHT	32

U0 ZOTransform(CDC *dc,I64 *x,I64 *y,I64 *z)
{
  I64 zz;
  Mat4x4MulXYZ(dc->r,x,y,z);
  zz=*z;
  if (zz<1) zz=1;
  *x=SCREEN_SCALE/2* *x/zz;
  *y=SCREEN_SCALE/2* (*y+TANK_HEIGHT)/zz;
  *x+=dc->x;
  *y+=dc->y;
  *z+=dc->z;
}

U0 DrawIt(CTask *task,CDC *dc)
{
  Obj *tempo;
  I64 i,dd,y,w=task->pix_width,h=task->pix_height,cx=w>>1,cy=h>>1;
  U8 *img;
  F64 tt,theta;
  Bool old_preempt=Preempt;

  theta=640*Wrap(2*us.theta)/pi;
  Sprite3(dc,theta-1280,90,0,__BIN_4);
  Sprite3(dc,theta	   ,90,0,__BIN_4);
  Sprite3(dc,theta+1280,90,0,__BIN_4);

  DCDepthBufAlloc(dc);
  dc->transform=&ZOTransform;
  dc->x=cx;
  dc->y=cy;
  Mat4x4TranslationEqu(dc->r,-us.x,-us.y,-us.z);
  Mat4x4RotY(dc->r,us.theta-pi/2);
  Mat4x4RotX(dc->r,pi/16);

  dc->flags|=DCF_TRANSFORMATION;
  for (i=0;i<NUM_THEM;i++) {
    y=them[i].y;
    tempo=shots.next;
    while (tempo!=&shots) {
      dd=SqrI64(them[i].x-tempo->x)+SqrI64(them[i].z-tempo->z);
      if (dd<SCREEN_SCALE/2*SCREEN_SCALE/2) {
	y-=Sqrt(dd);
	if (!them[i].hit) {
	  them[i].hit=TRUE;
	  if (!--num_them) {
	    tf=tS;
	    if (tf-t0<best_score)
	      best_score=tf-t0;
	  }
	}
      }
      tempo=tempo->next;
    }
    if (them[i].hit)
      img=__BIN_2;
    else
      img=__BIN_1;
    SpriteY3B(dc,them[i].x,y,them[i].z,img,-them[i].theta);
  }
  tempo=shots.next;
  while (tempo!=&shots) {
    SpriteY3B(dc,tempo->x,tempo->y,tempo->z,__BIN_3,-tempo->theta);
    tempo=tempo->next;
  }
  dc->flags&=~DCF_TRANSFORMATION;
  dc->color=LTGREEN;
  GrLine(dc,cx-5,cy,cx+5,cy);
  GrLine(dc,cx,cy-5,cx,cy+5);
  Preempt(old_preempt);
  dc->color=LTPURPLE;
  if (tf) {
    dc->color=LTRED;
    if (Blink)
      GrPrint(dc,cx-(FONT_WIDTH*14)/2,cy-FONT_HEIGHT/2,"Game Completed");
    tt=tf;
  } else {
    dc->color=LTGREEN;
    GrLine(dc,cx-5,cy,cx+5,cy);
    GrLine(dc,cx,cy-5,cx,cy+5);
    tt=tS;
  }
  GrPrint(dc,0,0,"Enemy:%d Time:%3.2f Best:%3.2f",num_them,tt-t0,best_score);
}

U0 Fire()
{
  Obj *tempo=MAlloc(sizeof(Obj));
  tempo->x=us.x;
  tempo->y=TANK_HEIGHT;
  tempo->z=us.z;
  tempo->theta=us.theta;
  tempo->t0=tS;
  QueIns(tempo,shots.last);
}

U0 MoveUs(F64 theta)
{
  us.x+=0.1*SCREEN_SCALE*Cos(theta);
  us.z+=0.1*SCREEN_SCALE*Sin(theta);
}

U0 AnimateTask(I64)
{//Preemption is off after Spawn
  I64 i;
  Obj *tempo,*tempo1;
  while (TRUE) {
    for (i=0;i<NUM_THEM;i++) {
      them[i].x+=SCREEN_SCALE/32*Cos(them[i].theta);
      them[i].z+=SCREEN_SCALE/32*Sin(them[i].theta);
      them[i].theta+=Rand/100.0;
    }
    tempo=shots.next;
    while (tempo!=&shots) {
      tempo1=tempo->next;
      if (tS-tempo->t0>1.0) {
	QueRem(tempo);
	Free(tempo);
      } else {
	tempo->x+=0.25*SCREEN_SCALE*Cos(tempo->theta);
	tempo->z+=0.25*SCREEN_SCALE*Sin(tempo->theta);
      }
      tempo=tempo1;
    }
    Sleep(20);
  }
}

U0 Init()
{
  I64 i;
  DocClear;
  "$BG,DKGRAY$%h12c",'\n';
  QueInit(&shots);
  MemSet(&us,0,sizeof(us));
  MemSet(them,0,sizeof(them));
  num_them=NUM_THEM;
  for (i=0;i<NUM_THEM;i++) {
    them[i].x=10000*Rand-5000;
    them[i].z=10000*Rand-5000;
    them[i].theta=2*pi*Rand;
    them[i].hit=FALSE;
  }
  tf=0;
  t0=tS;
}

U0 CleanUp()
{
  QueDel(&shots,TRUE);
}

U0 SongTask(I64)
{//Randomly generate (by God :-)
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  while (TRUE) {
    Play("3sDBDBqEsGBGBeD2G3sBFBFeDAqFG");
    Play("sDBDBqEsGBGBeD2G3sBFBFeDAqFG");
    Play("eGDsFGFGqDEeBE2sG3D2G3DqF2sG3A2G3A");
    Play("eGDsFGFGqDEeBE2sG3D2G3DqF2sG3A2G3A");
  }
}

U0 ZoneOut()
{
  I64 sc;

  PopUpOk(
	"I refuse to rip-off the original\n"
	"so this is intentionally crappy\n"
	"and included for demonstration\n"
	"purposes.\n\n"
	"Write games, don't play them.\n");

  MenuPush(
	"File {"
	"  Abort(,CH_SHIFT_ESC);"
	"  Exit(,CH_ESC);"
	"}"
	"Play {"
	"  Restart(,'\n');"
	"  Fwd(,,SC_CURSOR_UP);"
	"  Bwd(,,SC_CURSOR_DOWN);"
	"  Left(,,SC_CURSOR_LEFT);"
	"  Right(,,SC_CURSOR_RIGHT);"
	"  Fire(,CH_SPACE);"
	"}"
	);

  SettingsPush; //See [C:/Adam/TaskSettings.CPP.Z,3] SettingsPush
  Fs->text_attr=BLACK<<4+WHITE;
  WinMax;
  WinBorder;
  AutoComplete;
  Preempt;
  Init;
  Fs->animate_task=Spawn(&AnimateTask,NULL,"Animate",,Fs);
  Fs->song_task=Spawn(&SongTask,NULL,"Song",,Fs);
  DocCursor;
  Fs->draw_it=&DrawIt;
  try {
    while (TRUE) {
      switch (GetKey(&sc)) {
	case CH_SPACE:
	  Fire;
	  break;
	case '\n':
	  CleanUp;
	  Init;
	  break;
	case CH_ESC:
	case CH_SHIFT_ESC:
	  goto zo_done;
	case 0:
	  switch (sc.u8[0]) {
	    case SC_CURSOR_RIGHT:
	      us.theta-=pi/256;
	      break;
	    case SC_CURSOR_LEFT:
	      us.theta+=pi/256;
	      break;
	    case SC_CURSOR_UP:
	      MoveUs(us.theta);
	      break;
	    case SC_CURSOR_DOWN:
	      MoveUs(us.theta+pi);
	      break;
	  }
	  break;
      }
    }
zo_done:
  } catch
    PutExcept;
  CleanUp;
  DocClear;
  SettingsPop;
  MenuPop;
  RegWriteBranch("TempleOS/ZoneOut","F64 best_score=%5.4f;\n",best_score);
}

ZoneOut;
