AcctRegSetDftEntry("TempleOS/Wenceslas","F64 best_score=9999;\n");
AcctRegExeBranch("TempleOS/Wenceslas");

#define BORDER		5
#define KING_STEP	4



    /* <1> <1> (image) */




    /* <2> <2> (image) */







 


    /* <3> <3> (image) */




    /* <4> <4> (image) */






    /* <5> <5> (image) */


















    /* <6> <6> (image) */






    /* <7> <7> (image) */

#define NUM_FLAKES	0x8000
class Flake
{
  U16 x,y;
} flakes[NUM_FLAKES];

#define NUM_TREES	8
class Tree
{
  U16 x,y;
  I64 fire_frame_index;
} trees[NUM_TREES];

I64 snow_x,snow_y;
I64 king_x,king_y,not_stopped_cnt;
F64 king_theta,t0,tf,door_open_t0;

#define NUM_PEASANTS	16
class Peasant
{
  Peasant *next,*last;
  F64 x,y,theta,door_opened_t0;
  Bool stopped;
} peasant_root;

class Step
{
  Step *next,*last;
  U16 x,y;
  F64 t0;
} step_root,king_step_root;

U0 DrawIt(CTask *task,CDC *dc)
{
  I64 r[16],i,w=task->pix_width,h=task->pix_height;
  U8 *img;
  Step *temps;
  Peasant *tempp;
  F64 tt;
  Bool old_preempt=Preempt(OFF);

  dc->pen_width=1;
  for (i=0;i<NUM_TREES;i++) {
    if (trees[i].y<=h/2) {//Draw trees behind house
      if (trees[i].fire_frame_index) {
	if (trees[i].fire_frame_index++&1)
	  img=__BIN_4;
	else
	  img=__BIN_5;
      } else
	img=__BIN_3;
      Sprite3(dc,trees[i].x,trees[i].y,0,img);
    }
  }

  Sprite3(dc,w/2,h/2,0,__BIN_6);
  if (tS<door_open_t0+1.0)
    Sprite3(dc,w/2,h/2,0,__BIN_7);

  for (i=0;i<NUM_TREES;i++) {
    if (trees[i].y>h/2) {//Draw trees in front of house
      if (trees[i].fire_frame_index) {
	if (trees[i].fire_frame_index++&1)
	  img=__BIN_4;
	else
	  img=__BIN_5;
      } else
	img=__BIN_3;
      Sprite3(dc,trees[i].x,trees[i].y,0,img);
    }
  }

  temps=step_root.next;
  while (temps!=&step_root) {
    if (tS-temps->t0<2.0)
      dc->color=DKGRAY;
    else
      dc->color=LTGRAY;
    dc->pen_width=2;
    GrPlot3(dc,temps->x,temps->y,0);
    temps=temps->next;
  }

  dc->pen_width=1;
  tempp=peasant_root.next;
  while (tempp!=&peasant_root) {
    GrIdentEqu(r);
    GrRotYEqu(r,tempp->theta+pi/2);
    GrRotXEqu(r,pi/6);
    if (tempp->stopped)
      dc->color=LTCYAN;
    else
      dc->color=CYAN;
    SpriteMat3B(dc,tempp->x,tempp->y,0,__BIN_2,r);
    tempp=tempp->next;
  }
 
  dc->pen_width=1;
  GrIdentEqu(r);
  GrRotYEqu(r,king_theta+pi/2);
  GrRotXEqu(r,pi/6);
  SpriteMat3B(dc,king_x,king_y,0,__BIN_1,r);

  dc->pen_width=1;
  dc->color=WHITE;
  for (i=0;i<NUM_FLAKES;i++)
    GrPlot(dc,(flakes[i].x+snow_x)%task->pix_width,
		(flakes[i].y+snow_y)%task->pix_height);
 
  if (tf) {
    dc->color=LTRED;
    if (Blink)
      GrPrint(dc,w/2-(FONT_WIDTH*14)/2,h/2+FONT_HEIGHT,"Game Completed");
    tt=tf;
  } else {
    tt=tS;
  }
  dc->color=LTBLUE;
  GrPrint(dc,0,0,"Freezing Peasants:%d Time:%3.2f Best:%3.2f",
    not_stopped_cnt,tt-t0,best_score);

  Preempt(old_preempt);
}

U0 StepNew(CTask *task,I64 x,I64 y,F64 theta,Bool king)
{
  Step *temps;

  if (king) {
    temps=MAlloc(sizeof(Step),task);
    temps->x=x;
    temps->y=y;
    temps->t0=tS;
    QueIns(temps,king_step_root.last);
  }

  temps=MAlloc(sizeof(Step),task);
  temps->x=x-3.5*Sin(theta)+1.0*Cos(theta);
  temps->y=y+3.5*Cos(theta)+1.0*Sin(theta);
  temps->t0=tS;
  QueIns(temps,step_root.last);

  temps=MAlloc(sizeof(Step),task);
  temps->x=x+3.5*Sin(theta)-1.0*Cos(theta);
  temps->y=y-3.5*Cos(theta)-1.0*Sin(theta);
  temps->t0=tS;
  QueIns(temps,step_root.last);
}

U0 Init()
{
  I64 i,min_x,max_x,min_y,max_y,
      w=Fs->pix_width,h=Fs->pix_height;
  Peasant *tempp;
  snow_x=0;
  snow_y=0;
  for (i=0;i<NUM_FLAKES;i++) {
    flakes[i].x=RandU16;
    flakes[i].y=RandU16;
  }

  SpriteExtents(__BIN_3,&min_x,&max_x,&min_y,&max_y);
  for (i=0;i<NUM_TREES;i++) {
    trees[i].x=RandU16%(w-2*BORDER-(max_x-min_x+1))+BORDER-min_x;
    trees[i].y=RandU16%(h-2*BORDER-(max_y-min_y+1))+BORDER-min_y;
    trees[i].fire_frame_index=0;
  }

  QueInit(&step_root);
  QueInit(&king_step_root);

  SpriteExtents(__BIN_1,&min_x,&max_x,&min_y,&max_y);
  QueInit(&peasant_root);
  for (i=0;i<NUM_PEASANTS;i++) {
    tempp=MAlloc(sizeof(Peasant));
    tempp->x=RandU16%(w-2*BORDER-(max_x-min_x+1))+BORDER-min_x;
    tempp->y=RandU16%(h-2*BORDER-(max_y-min_y+1))+BORDER-min_y;
    tempp->theta=pi*2*Rand;
    tempp->door_opened_t0=0;
    tempp->stopped=FALSE;
    QueIns(tempp,peasant_root.last);
  }

  king_x=w/2;
  king_y=h/2;
  king_theta=-pi/2;
  door_open_t0=0;
  t0=tS;
  tf=0;
}

U0 CleanUp()
{
  QueDel(&peasant_root,TRUE);
  QueDel(&step_root,TRUE);
  QueDel(&king_step_root,TRUE);
}

U0 Follow(CTask *,Peasant *tempp)
{
  Step *temps=king_step_root.next;
  F64 d,best_d=MAX_F64;
  while (temps!=&king_step_root) {
    if ((d=Sqr(temps->x-tempp->x)+Sqr(temps->y-tempp->y)) &&
	d<15*15) {
      d+=1000*Sqr(tS-temps->t0);
      if (d<best_d) {
	best_d=d;
	tempp->theta=Arg(temps->x-tempp->x,temps->y-tempp->y);
      }
    }
    temps=temps->next;
  }
}

U0 AnimateTask(I64)
{//Preempt off by default after spawned.
  Step *temps,*temps1;
  Peasant *tempp;
  I64 i,animate_cnt=0,w,h;
  while (TRUE) {
    w=Fs->parent_task->pix_width;
    h=Fs->parent_task->pix_height;

    temps=step_root.next;
    while (temps!=&step_root) {
      temps1=temps->next;
      if (tS-temps->t0>4.0) {
	QueRem(temps);
	Free(temps);
      }
      temps=temps1;
    }

    temps=king_step_root.next;
    while (temps!=&king_step_root) {
      temps1=temps->next;
      if (tS-temps->t0>4.0) {
	QueRem(temps);
	Free(temps);
      }
      temps=temps1;
    }

    not_stopped_cnt=0;
    tempp=peasant_root.next;
    while (tempp!=&peasant_root) {
      if (tempp->stopped) {
	if (tempp->door_opened_t0 && tS>tempp->door_opened_t0+1.0) {
	  tempp->door_opened_t0=0;
	  tempp->y=tempp->x=-9999;
	}
      } else {
	if (Sqr(tempp->x-w/2)+Sqr(tempp->y-h/2)<20*20) {
	  tempp->stopped=TRUE;
	  tempp->door_opened_t0=door_open_t0=tS;
	}
	for (i=0;i<NUM_TREES;i++) //Hang-out by fire
	  if (trees[i].fire_frame_index && Sqr(tempp->x-trees[i].x)+Sqr(tempp->y-trees[i].y)<20*20) {
	    tempp->stopped=TRUE;
	    break;
	  }
	if (!tempp->stopped) {
	  Follow(Fs->parent_task,tempp);
	  tempp->x+=Cos(tempp->theta)/100;
	  tempp->y+=Sin(tempp->theta)/100;
	  if (!(BORDER/2<=tempp->x<w-BORDER/2) || !(BORDER/2<=tempp->y<h-BORDER/2)) {
	    tempp->theta+=pi;
	    tempp->x+=3*Cos(tempp->theta)/100;
	    tempp->y+=3*Sin(tempp->theta)/100;
	  }
	  if (!(animate_cnt&511))
	    StepNew(Fs->parent_task,tempp->x,tempp->y,tempp->theta,FALSE);
	  not_stopped_cnt++;
	}
      }
      tempp=tempp->next;
    }
    if (!not_stopped_cnt && !tf) {
      tf=tS;
      music.mute=TRUE;
      Snd(2000);Sleep(200);Snd(0);Sleep(100);
      if (tf-t0<best_score) {
	best_score=tf-t0;
	Snd(2000);Sleep(200);Snd(0);Sleep(100);
      }
      music.mute=FALSE;
    }

    snow_x+=RandU16%3-1;
    snow_y+=1-SignI64(RandU16&3);
    Sleep(1);
    animate_cnt++;
  }
}

U0 BurnTrees()
{
  I64 i;
  for (i=0;i<NUM_TREES;i++)
    if (Sqr(king_x-trees[i].x)+Sqr(king_y-trees[i].y)<10*10)
      trees[i].fire_frame_index=1;
}

U0 SongTask(I64)
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.902;
  while (TRUE) {
	Play("3eCCCDCC2qG3eA2G3ABqCC");
	Play("eCCCDCC2qG3eA2G3ABqCC");
	Play("eGFEDEDqCeA2G3ABqCC");
  }
}
 
U0 Wenceslas()
{
  I64 sc;

  MenuPush(
  "File {"
  "  Abort(,CH_SHIFT_ESC);"
  "  Exit(,CH_ESC);"
  "}"
  "Play {"
  "  Restart(,'\n');"
  "  BurnTree(,CH_SPACE);"
  "  Up(,,SC_CURSOR_UP);"
  "  Down(,,SC_CURSOR_DOWN);"
  "  Left(,,SC_CURSOR_LEFT);"
  "  Right(,,SC_CURSOR_RIGHT);"
  "}"
  );

  SettingsPush; //See [C:/Adam/TaskSettings.CPP.Z,3] SettingsPush
  WinMax;
  Preempt(OFF);
  Fs->song_task=Spawn(&SongTask,NULL,"Song",,Fs);

  PopUpOk("
Good King Wenceslas looked out
On the feast of Stephen
When the snow lay round about
Deep and crisp and even
Brightly shone the moon that night
Though the frost was cruel
When a poor man came in sight
Gath'ring winter fuel

\"Hither, page, and stand by me
If thou know'st it, telling
Yonder peasant, who is he?
Where and what his dwelling?\"
\"Sire, he lives a good league hence
Underneath the mountain
Right against the forest fence
By Saint Agnes' fountain.\"

\"Bring me flesh and bring me wine
Bring me pine logs hither
Thou and I will see him dine
When we bear him thither.\"
Page and monarch forth they went
Forth they went together
Through the rude wind's wild lament
And the bitter weather

");
PopUpOk("
\"Sire, the night is darker now
And the wind blows stronger
Fails my heart, I know not how,
I can go no longer.\"
\"Mark my footsteps, my good page
Tread thou in them boldly
Thou shalt find the winter's rage
Freeze thy blood less coldly.\"

In his master's steps he trod
Where the snow lay dinted
Heat was in the very sod
Which the Saint had printed
Therefore, Christian men, be sure
Wealth or rank possessing
Ye who now will bless the poor
Shall yourselves find blessing

");
PopUpOk("
$PURPLE$$TX+CX,\"Winceslas Game\"$$FG$

Start fires by pressing $GREEN$<SPACE>$FG$
on trees. (Yule logs)

Lead peasants to fires.
");

  Init;
  Fs->animate_task=Spawn(&AnimateTask,NULL,"Animate",,Fs);
  WordStat;

  DocCursor;
  DocClear;
  Fs->draw_it=&DrawIt;
  try {
    while (TRUE) {
      switch (GetKey(&sc)) {
	case '\n':
	  CleanUp;
	  Init;
	  break;
	case CH_ESC:
	case CH_SHIFT_ESC:
	  goto ws_done;
	case CH_SPACE:
	  BurnTrees;
	  break;
	case 0:
	  switch (sc.u8[0]) {
	    case SC_CURSOR_RIGHT:
	      if (king_x+KING_STEP<Fs->pix_width-BORDER)
		king_x+=KING_STEP;
	      king_theta=0;
	      StepNew(Fs,king_x,king_y,king_theta,TRUE);
	      break;
	    case SC_CURSOR_LEFT:
	      if (king_x-KING_STEP>=BORDER)
		king_x-=KING_STEP;
	      king_theta=pi;
	      StepNew(Fs,king_x,king_y,king_theta,TRUE);
	      break;
	    case SC_CURSOR_DOWN:
	      if (king_y+KING_STEP<Fs->pix_height-BORDER)
		king_y+=KING_STEP;
	      king_theta=-pi/2;
	      StepNew(Fs,king_x,king_y,king_theta,TRUE);
	      break;
	    case SC_CURSOR_UP:
	      if (king_y-KING_STEP>=BORDER)
		king_y-=KING_STEP;
	      king_theta=pi/2;
	      StepNew(Fs,king_x,king_y,king_theta,TRUE);
	      break;
	  }
      }
    }
ws_done:
  } catch
    PutExcept;
  SettingsPop;
  MenuPop;
  CleanUp;
  AcctRegWriteBranch("TempleOS/Wenceslas","F64 best_score=%5.4f;\n",best_score);
}

Wenceslas;