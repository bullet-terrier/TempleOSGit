U0 PrsPush(CPrsStk *ps,I64 val)
{
  ps->stk[++ps->ptr]=val;
}

I64 PrsPop(CPrsStk *ps)
{
  return ps->stk[ps->ptr--];
}

U0 PrsPush2(CPrsStk *ps,I64 val)
{
  ps->stk2[++ps->ptr2]=val;
}

I64 PrsPop2(CPrsStk *ps)
{
  return ps->stk2[ps->ptr2--];
}

U0 PrsPopDeref(CPrsStk *ps)
{
  I64 i=PrsPop(ps);
  CHashClass *tempc=PrsPop(ps);
  if (i.u16[0]!=IC_DEREF) {
    PrsPush(ps,tempc);
    PrsPush(ps,i);
  }
}

I64 PrsKeyWord(CCmpCtrl *cc)
{//Cvt cur token to [C:/Compiler/OpCodes.TXT.Z,1] KEYWORD or -1.
  CHashGeneric *temph;
  if (cc->token==TK_IDENT &&(temph=cc->hash_entry) && temph->type&HTT_KEYWORD)
    return temph->user_data0;
  else
    return -1;
}

CHashClass *PrsClassNew()
{/*Ptrs to classes are handled by
allocating 5 structures for each
new class and representing a pointer
to a class by advancing 1 struct fwd
for one * and two fwd for two **.
*/
  I64 i;
  CHashClass *result=CAlloc(sizeof(CHashClass)*(MAX_PTR_STARS+1),Fs->code_heap),
	*tempc=result;
  for (i=0;i<=MAX_PTR_STARS;i++) {
    tempc->type=HTT_CLASS;
    tempc->raw_type=RT_PTR;
    tempc->size=sizeof(U8 *);
    tempc->ptr_stars_cnt=i;
    tempc++;
  }
  result->last_in_member_lst=&result->member_lst;
  result->size=0;
  return result;
}

CHashFun *PrsFunNew()
{
  I64 i;
  CHashFun *result=CAlloc(sizeof(CHashFun)*(MAX_PTR_STARS+1),Fs->code_heap),
	*tempf=result;
  for (i=0;i<=MAX_PTR_STARS;i++) {
    tempf->type=HTT_FUN;
    tempf->raw_type=RT_PTR;
    tempf->size=sizeof(U8 *);
    tempf->ptr_stars_cnt=i;
    tempf++;
  }
  result->last_in_member_lst=&result->member_lst;
  result->size=0;
  return result;
}

CIntermediateCode *ICAdd(CCmpCtrl *cc,
   I64 opcode_and_precedence,I64 arg, CHashClass *c,I64 flags=0)
{
  CIntermediateCode *tempi=MAlloc(sizeof(CIntermediateCode));
  tempi->ic_code=opcode_and_precedence.u16[0];
  tempi->ic_precedence=opcode_and_precedence.u16[1];
  tempi->ic_data=arg;
  tempi->ic_class=c;
  if (Bt(&cc->opts,OPTf_OPT_TRACE)) {
    Bts(&cc->flags,CCf_OPT_TRACE_PRESENT);
    flags|=ICF_TRACE;
  }
  if (cc->lock_cnt)
    flags|=ICF_LOCK;
  tempi->ic_flags=flags;
  tempi->ic_line=cc->last_line_num;
  QueIns(tempi,cc->coc.coc_root.last);
  return tempi;
}

U0 COCInit(CCmpCtrl *cc)
{
  CCodeCtrl *tempcbh=&cc->coc;
  QueInit(&tempcbh->coc_root.next);
  QueInit(&tempcbh->coc_next_misc);
  tempcbh->coc_root.ic_code=IC_END;
}

U0 COCPush(CCmpCtrl *cc)
{
  CCodeCtrl *tempcbh=MAlloc(sizeof(CCodeCtrl));
  MemCpy(tempcbh,&cc->coc,sizeof(CCodeCtrl));
  cc->coc.coc_next=tempcbh;
}

CCmpCtrl *COCPopNoFree(CCmpCtrl *cc)
{
  CCodeCtrl *tempcbh=cc->coc.coc_next;
  MemCpy(&cc->coc,tempcbh,sizeof(CCodeCtrl));
  return tempcbh;
}

U0 COCPop(CCmpCtrl *cc)
{
  Free(COCPopNoFree(cc));
}

U0 COCAppend(CCmpCtrl *cc, CCodeCtrl *tempcbh)
{
  if (tempcbh->coc_root.next!=&cc->coc.coc_root.next) {
    cc->coc.coc_root.last->next=tempcbh->coc_root.next;
    tempcbh->coc_root.next->last=cc->coc.coc_root.last;
    cc->coc.coc_root.last=tempcbh->coc_root.last;
    tempcbh->coc_root.last->next=&cc->coc.coc_root.next;
  }
  if (tempcbh->coc_next_misc!=&cc->coc.coc_next_misc) {
    cc->coc.coc_last_misc->next=tempcbh->coc_next_misc;
    tempcbh->coc_next_misc->last=cc->coc.coc_last_misc;
    cc->coc.coc_last_misc=tempcbh->coc_last_misc;
    tempcbh->coc_last_misc->next=&cc->coc.coc_next_misc;
  }
  Free(tempcbh);
}

CCodeMisc *COCMiscNew(CCmpCtrl *cc,I64 ty)
{
  CCodeMisc *result=CAlloc(sizeof(CCodeMisc));
  result->addr=INVALID_PTR;
  result->type=ty;
  QueIns(result,cc->coc.coc_last_misc);
  return result;
}

CCodeMisc *COCGoToLabelFind(CCmpCtrl *cc,U8 *name)
{
  CCodeMisc *cm=cc->coc.coc_next_misc;
  while (cm!=&cc->coc.coc_next_misc) {
    if ((cm->type==CMT_GOTO_LABEL||cm->type==CMT_ASM_LABEL) &&
	  !StrCmp(cm->str,name))
      return cm;
    cm=cm->next;
  }
  return NULL;
}

I64 COCFloatConstFind(CCmpCtrl *cc,F64 d)
{
  I64 i;
  CCodeMisc *cm=cc->coc.coc_next_misc;
  while (cm!=&cc->coc.coc_next_misc) {
    if (cm->type==CMT_FLOAT_CONSTS) {
      for (i=0;i<cm->num_consts;i++)
	if (cm->float_consts[i]==d)
	  return cm->addr+i*sizeof(F64);
      if (cm->num_consts<CM_MAX_CONSTS)  {
	cm->float_consts[cm->num_consts++]=d;
	return cm->addr+i*sizeof(F64);
      }
    }
    cm=cm->next;
  }
  cm=COCMiscNew(cc,CMT_FLOAT_CONSTS);
  cm->float_consts=MAlloc(CM_MAX_CONSTS*sizeof(F64));
  cm->float_consts[cm->num_consts++]=d;
  return cm->addr;
}

U0 COCDel(CCmpCtrl *cc,CCodeCtrl *coc)
{
  CCodeMisc *cm,*cm1;
  U8 *undef=NULL;
  QueDel(&coc->coc_root.next);
  cm=coc->coc_next_misc;
  while (cm!=&coc->coc_next_misc) {
    cm1=cm->next;
    switch (cm->type) {
      case CMT_GOTO_LABEL:
      case CMT_ASM_LABEL:
	if (!(cm->flags&CMF_DEFINED)) {
	  undef=cm->str;
	  cm->str=NULL;
	} else if (!cm->use_cnt) {
	  PrintWarn("Unused label %s\n",cm->str);
	  LexWarn(cc,"Unused label at ");
	}
	break;
      case CMT_JMP_TABLE:
	Free(cm->jmp_table);
	break;
      case CMT_FLOAT_CONSTS:
	Free(cm->float_consts);
	break;
      case CMT_ARRAY_DIM:
	LinkedLstDel(cm->dim);
	break;
      case CMT_HASH_ENTRY:
	HashDel(cm->h);
	break;
    }
    Free(cm->str);
    Free(cm);
    cm=cm1;
  }
  if (undef) {
    PrintErr("Undefined goto label %s\n",undef);
    Free(undef);
    LexExcept(cc,"Undefined goto label at ");
  }
}

U0 COCHeaderPut(CCmpCtrl *cc,I64 pass,Bool put)
{
  CIntermediateCode *tempi;
  if (Bt(&cc->flags,CCf_OPT_TRACE_PRESENT)) {
    if (put) {
      if (Bt(&cc->otrace_mask,pass-1)) {
	"$IV,1$Pass %d:$IV,0$\n",pass-1;
	tempi=cc->coc.coc_root.next;
	while (tempi->ic_code) {
	  if (tempi->ic_flags&ICF_TRACE)
	    ICPut(cc,tempi);
	  tempi=tempi->next;
	}
      }
    } else if (Bt(&cc->otrace_mask,pass))
      "$IV,1$Pass %d:$IV,0$\n",pass;
  }
  cc->pass=pass;
}

U8 *COCCompile(CCmpCtrl *cc,I64 *_code_size,CDbgInfo **_dbg,I64 *_type)
{
  U8 *result;
  CCodeMisc *lb;
  I64 i,code_size,last_code_size;

  COptReg reg_offsets[NUM_REGS];
  if (_dbg) *_dbg=NULL;
  cc->pass=0;
  COCHeaderPut(cc,1,TRUE);
  OptPass012(cc);
  COCHeaderPut(cc,2,TRUE);
  OptPass012(cc);
  COCHeaderPut(cc,3,TRUE);
  OptPass3(cc,reg_offsets);
  COCHeaderPut(cc,4,TRUE);
  OptPass4(cc,reg_offsets,_type);
  COCHeaderPut(cc,5,TRUE);
  OptPass5(cc);
  COCHeaderPut(cc,6,TRUE);
  OptPass6(cc);
  COCHeaderPut(cc,7,TRUE);

  lb=cc->coc.coc_next_misc;
  while (lb!=&cc->coc.coc_next_misc) {
    if (lb->type==CMT_JMP_TABLE) {
      for (i=0;i<lb->range;i++)
	lb->jmp_table[i]=OptLabelFwd(lb->jmp_table[i]);
      lb->dft=OptLabelFwd(lb->dft);
    }
    lb=lb->next;
  }

  COCHeaderPut(cc,7,FALSE);
  OptPass789A(cc,reg_offsets,NULL,NULL);
  COCHeaderPut(cc,8,FALSE);
  OptPass789A(cc,reg_offsets,NULL,NULL);
  COCHeaderPut(cc,9,FALSE);
  code_size=OptPass789A(cc,reg_offsets,NULL,NULL);
  do {
    last_code_size=code_size;
    COCHeaderPut(cc,9,FALSE);
    code_size=OptPass789A(cc,reg_offsets,NULL,NULL);
    if (code_size>last_code_size) {
      "Pass:9 Code Size\n";
      LexExcept(cc,"Compiler Optimization Error at ");
    }
  } while (code_size<last_code_size);

  if (cc->flags&CCF_AOT_COMPILE)
    result=MAlloc(code_size);
  else
    result=MAlloc(code_size,Fs->code_heap);
  COCHeaderPut(cc,10,FALSE);
  code_size=OptPass789A(cc,reg_offsets,result,_dbg);

  COCDel(cc,&cc->coc);
  if (Bt(&cc->opts,OPTf_COMPILE_TRACE)) {
    if (cc->flags&CCF_AOT_COMPILE) {
      if (cc->aotc->seg_size==16)
	Un(result,code_size,16);
      else if (cc->aotc->seg_size==64)
	Un(result,code_size,64);
      else
	Un(result,code_size,32);
    } else
      Un(result,code_size,64);
  }
  if (_code_size) *_code_size=code_size;
  return result;
}
