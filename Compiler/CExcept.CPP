U0 LexPutToken(CCmpCtrl *cc)
{//Print cur token to StdOut. (Crude)
  '"';
  if (cc->token==TK_IDENT || cc->token==TK_STR)
    "%s",cc->cur_str;
  else if (cc->token==TK_INTEGER)
    "INT:%X",cc->cur_i64;
  else if (cc->token==TK_CHAR_CONST)
    "U8:%X",cc->cur_i64;
  else if (cc->token==TK_F64)
    "FLOAT:%e",cc->cur_f64;
  else if (cc->token=='\n')
    "<NEW_LINE>";
  else if (Bt(chars_bmp_displayable,cc->token))
    '' cc->token;
  else {
    "T:%X",cc->token;
  }
  "\" ";
}

U8 *LexPutLine(CCmpCtrl *cc,U8 *start)
{//Print cur pos to end of line to StdOut.
  I64 ch;
  U8 *ptr;
  if (!start) return NULL;
  if (cc->lex_include_stk->flags&LFSF_DOC)
    return DocPutLine(cc->lex_include_stk->doc,start);
  else {
    ptr=start;
    while (ch=*ptr++) {
      if (ch=='\n') {
	if (*ptr=='\r')
	  ptr++;
	break;
      } else if (ch=='\r') {
	if (*ptr=='\n')
	  ptr++;
	break;
      }
      '' ch;
    }
    if (!ch) ptr--;
    '\n';
    return ptr;
  }
  return NULL;
}

U0 LexPutPos(CCmpCtrl *cc)
{//Print token, line link and, then, [C:/Compiler/CExcept.CPP.Z,22] LexPutLine().
  LexPutToken(cc);
  FixSet(cc->lex_include_stk->full_name,cc->lex_include_stk->line_num);
  if (IsRaw)
    "%s,%d ",cc->lex_include_stk->full_name,cc->lex_include_stk->line_num;
  else {
    PutFileLink(cc->lex_include_stk->full_name,cc->lex_include_stk->line_num);
    AdamErr("%s,%d\n",
	  cc->lex_include_stk->full_name,cc->lex_include_stk->line_num);
    '' CH_SPACE;
  }
  LexPutLine(cc,cc->lex_include_stk->line_start);
}

U0 LexWarn(CCmpCtrl *cc,U8 *str=NULL)
{//Print warn msg, then, [C:/Compiler/CExcept.CPP.Z,50] LexPutPos().
  if (str) PrintWarn(str);
  if (cc->htc.fun) {
    "in fun '%s'.\n",cc->htc.fun->str;
    if (IsRaw)
      "%s\n",cc->htc.fun->src_link;
    else {
      "$LK,\"%s\"$\n",cc->htc.fun->src_link;
      AdamErr("%s\n",cc->htc.fun->src_link);
    }
  } else
    LexPutPos(cc);
  cc->warning_cnt++;
}

U0 LexExcept(CCmpCtrl *cc,U8 *str=NULL)
{//Print err msg, [C:/Compiler/CExcept.CPP.Z,50] LexPutPos() and throw exception.
  if (!Bt(&sys_run_level,RLf_ADAM_SERVER)) {
    Preempt;
    Raw(ON);
    "Note: Still in boot phase.\n";
  }
  if (str) PrintErr(str);
  if (!IsRaw)
    AdamErr("Task:%08X %s\n",Fs,str);
  LexPutPos(cc);
  cc->error_cnt++;
  FlushMsgs;
  if (!Bt(&sys_run_level,RLf_ADAM_SERVER))
    Dbg("Type \"Fix;\"");
  throw('Compiler');
}

U0 UndefinedExtern()
{
  PrintErr("Undefined Extern\nat %P\n",Caller);
  throw('UndefExt');
}

U0 UnusedExternWarning(CCmpCtrl *cc,CHashClass *tempc)
{
  PrintWarn("Unused extern '%s'\n",tempc->str);
  cc->warning_cnt++;
}

U0 ParenWarning(CCmpCtrl *cc)
{
  if (Bt(&cc->opts,OPTf_WARN_PAREN) &&
	!(cc->lex_include_stk->flags&LFSF_DEFINE))
    LexWarn(cc,"Unnecessary parenthesis ");
}

U0 ICClassPut(CHashClass *c)
{
  I64 i;
  if (!c) return;
  if (c->ptr_stars_cnt>4) {
    PrintErr("put_class ptrcnt=%d\n",c->ptr_stars_cnt);
    while (TRUE)
//TODO:
      Yield;
  }
  for (i=0;i<c->ptr_stars_cnt;i++)
    '*';
  c-=c->ptr_stars_cnt;
  if (c->str)
    "%s",c->str;
  else {
    PrintErr("put_class str=NULL\n");
    while (TRUE)
//TODO:
      Yield;
  }
  '' CH_SPACE;
}

U0 ICArgPut(CICArg *a)
{
  if (a->type_pointed)
    "%Z %Z %Z ",
	  a->type&RT_MASK,"ST_RAW_TYPES",
	  a->type_pointed&RT_MASK,"ST_RAW_TYPES",
	  a->type>>4,"ST_TY_TYPES";
  else
    "%Z %Z ",
	  a->type&RT_MASK,"ST_RAW_TYPES",
	  a->type>>4,"ST_TY_TYPES";
  switch (a->type>>4) {
    case TY_STK>>4:
      "STK";
      break;
    case TY_IMM>>4:
      "#%X",a->disp;
      break;
    case TY_REG>>4:
      "%Z",a->reg,"ST_U64_REGS";
      break;
    case TY_DISP>>4:
      "%X[%Z]",a->disp,a->reg,"ST_U64_REGS";
      break;
    case TY_RIP_DISP32>>4:
      "[%X]",a->disp;
      break;
    case TY_SIB>>4:
      if (a->disp)
	"%X",a->disp;
      if (a->reg==REG_RIP)
	'[';
      else
	"[%Z+",a->reg&15,"ST_U64_REGS";
      "%Z",a->reg>>8&15,"ST_U64_REGS";
      switch (a->reg>>14) {
	case 0: "]";   break;
	case 1: "*2]"; break;
	case 2: "*4]"; break;
	case 3: "*8]"; break;
      }
      break;
  }
}

U0 ICPut(CCmpCtrl *cc,CIntermediateCode *tempi)
{
  I64 opcode=tempi->ic_code,i;
  if (opcode>=IC_END_EXP && opcode!=IC_NOP2) {
    "%15ts %016X ",intermediate_code_table[opcode].name,tempi->ic_data;
    if (cc->pass) {
      if (tempi->r.type&TY_MASK) {
	ICArgPut(&tempi->r);
	"<-";
      }
      if (tempi->a1.type&TY_MASK)
	ICArgPut(&tempi->a1);
      ',';
      if (tempi->a2.type&TY_MASK)
	ICArgPut(&tempi->a2);
      '' CH_SPACE;
    }
    ICClassPut(tempi->ic_class);
    if (tempi->ic_flags & ICF_LOCK)
      "$BROWN$lock$FG$ ";
    if (tempi->ic_flags & ICF_A2_TO_F64)
      "$LTBLUE$a2d$FG$ ";
    if (tempi->ic_flags & ICF_A2_TO_INT)
      "$GREEN$a2i$FG$ ";
    if (tempi->ic_flags & ICF_A1_TO_F64)
      "$LTBLUE$a1d$FG$ ";
    if (tempi->ic_flags & ICF_A1_TO_INT)
      "$GREEN$a1i$FG$ ";
    if (tempi->ic_flags & ICF_R_TO_F64)
      "$LTBLUE$rd$FG$ ";
    if (tempi->ic_flags & ICF_R_TO_INT)
      "$GREEN$ri$FG$ ";
    if (tempi->ic_flags & ICF_USE_F64)
      "[F64] ";
    if (tempi->ic_flags & ICF_USE_UNSIGNED)
      "[unsigned] ";
    if (tempi->ic_flags & ICF_USE_INT)
      "[int] ";
    if (tempi->ic_flags & ICF_NO_DEPEND_RESULT)
      "-DEP ";
    if (tempi->ic_flags & ICF_DEPEND_RESULT)
      "+DEP ";
    if (tempi->ic_flags & ICF_NOT_ADDR)
      "NOT_ADDR ";
    if (tempi->ic_flags & ICF_PUSH_RESULT)
      "PUSH ";
    if (tempi->ic_flags & ICF_PUSH_CMP)
      "PUSH_CMP ";
    if (tempi->ic_flags & ICF_POP_CMP)
      "POP_CMP ";
    if (tempi->ic_flags & ICF_DEL_PREVIOUS_INS)
      "DELPREV ";
    if (tempi->ic_flags & ICF_PREVIOUS_DELETED)
      "PREVDEL ";
    for (i=0;i<3;i++) {
      if (Bt(&tempi->ic_flags,ICf_DONT_PUSH_FLOAT0+i))
	"DONT_PUSH#%d ",i;
      if (Bt(&tempi->ic_flags,ICf_DONT_POP_FLOAT0+i))
	"DONT_POP#%d ",i;
    }
    if (tempi->ic_flags & ICF_ALTERNATE_TEMPLATE)
      "ALT_TEMP ";
    '\n';
  }
}
