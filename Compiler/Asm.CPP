Bool PrsAsmImm(CCmpCtrl *cc,CAsmArg *arg)
{
  if (arg->imm_or_off_present)
    LexExcept(cc,"already one immediate at ");
  arg->imm_or_off_present=TRUE;
  arg->num.local_asm_undef_hash=NULL;
  arg->num.glbl_asm_undef_hash=NULL;
  cc->asm_undef_hash=NULL;
  cc->abs_cnts=0;
  cc->flags&=~(CCF_UNRESOLVED+CCF_LOCAL);
  if (!IsLexExpression2Bin(cc,&arg->num.machine_code))
    LexSkipEol(cc);
  else {
    if (cc->abs_cnts.externs)
      LexExcept(cc,"Extern Not Allowed at ");
    if (cc->flags & CCF_UNRESOLVED) {
      if (cc->flags & CCF_LOCAL) {
	arg->num.local_asm_undef_hash=cc->asm_undef_hash;
	cc->asm_undef_hash=NULL;
      } else {
	arg->num.glbl_asm_undef_hash=cc->asm_undef_hash;
	cc->asm_undef_hash=NULL;
      }
    } else {
      arg->num.i=Call(arg->num.machine_code);
      arg->num.glbl_asm_undef_hash=cc->asm_undef_hash;
      cc->asm_undef_hash=NULL;
      Free(arg->num.machine_code);
      arg->num.machine_code=NULL;
    }
  }
  return TRUE;
}

U0 PrsAsmArg(CCmpCtrl *cc,CAsmArg *arg,Bool rel)
{
  CHashGeneric *temph,*temph1;
  CHashReg *tempr;
  MemSet(arg,0,sizeof(CAsmArg));
  arg->seg=REG_NONE;
  arg->reg1=REG_NONE;
  arg->reg2=REG_NONE;
  arg->fstk=REG_NONE;
  arg->scale=1;
  while (TRUE) {
    if (cc->token==TK_IDENT) {
      if (temph=cc->hash_entry) {
	if (temph->type&HTG_TYPE_MASK==HTT_REG) {
	  tempr=temph;
	  switch (tempr->reg_type) {
	    sub_switch_start:
	      case REGT_R8:
		arg->size=1;
		break;
	      case REGT_R16:
		arg->size=2;
		break;
	      case REGT_R32:
		arg->size=4;
		break;
	      case REGT_R64:
		arg->size=8;
		break;
	    sub_switch_end:
	      arg->reg1=tempr->reg_num;
	      Lex(cc);
	      return;
	    case REGT_SEG:
	      arg->seg=tempr->reg_num;
	      if (Lex(cc)!=':') {
		arg->just_seg=TRUE;
		return;
	      } else
		Lex(cc); //skip ":"
	      break;
	    case REGT_FSTK:
	      arg->size=8;
	      arg->fstk=tempr->reg_num;
	      Lex(cc);
	      return;
	  }
	} else {
	  if ((temph->type&HTG_TYPE_MASK==HTT_CLASS||
		temph->type&HTG_TYPE_MASK==HTT_INTERNAL_TYPE) &&
		(temph1=HashFind(cc->cur_str,cmp.asm_hash,HTT_ASM_KEYWORD)))
	    temph=temph1;
	  if (temph->type&HTG_TYPE_MASK==HTT_ASM_KEYWORD) {
	    switch (temph->user_data0) {
	      case AKW_I8:
	      case AKW_U8:
		arg->size=1;
		break;
	      case AKW_I16:
	      case AKW_U16:
		arg->size=2;
		break;
	      case AKW_I32:
	      case AKW_U32:
		arg->size=4;
		break;
	      case AKW_I64:
	      case AKW_U64:
		arg->size=8;
		break;
	      default:
		LexExcept(cc,"syntax error at ");
	    }
	    Lex(cc); //skip keyword
	  } else
	    goto pa_asm_direct_imm;
	}
      } else {
pa_asm_direct_imm:
	PrsAsmImm(cc,arg);
	arg->num.abs_cnts=cc->abs_cnts;
	if (arg->size<=1 && !rel && arg->num.abs_cnts&1) {
	  if (cc->aotc->seg_size==16)
	    arg->size=2;
	  else
	    arg->size=4;
	}
	if (cc->token!='[')
	  return;
      }
    } else if (cc->token=='[') {
      arg->indirect=TRUE;
      Lex(cc); // skip [
      while (cc->token && cc->token!=']') {
	if (cc->token==TK_IDENT) {
	  if (temph=cc->hash_entry) {
	    if (temph->type&HTG_TYPE_MASK==HTT_REG &&
		  REGT_R16<=temph(CHashReg *)->reg_type<=REGT_R64) {
	      tempr=temph;
	      if (arg->reg1==REG_NONE) {
		if (tempr->reg_num&7==REG_RSP) {
		  arg->reg1=4;
		  arg->reg2=tempr->reg_num;
		} else
		  arg->reg1=tempr->reg_num;
	      } else
		arg->reg2=tempr->reg_num;
	      Lex(cc);
	    } else
	      goto pa_asm_indirect_imm;
	  } else
	    goto pa_asm_indirect_imm;
	} else if (cc->token=='*') {
	  Lex(cc);
	  if (cc->token!=TK_INTEGER)
	    LexExcept(cc,"expecting scale factor at ");
	  arg->scale=cc->cur_i64;
	  Lex(cc); //skip scale
	  if (arg->reg2!=REG_NONE)
	    SwapI64(&arg->reg1,&arg->reg2);
	} else if (cc->token=='+') {
	  Lex(cc); //skip '+'
	} else {
pa_asm_indirect_imm:
	  PrsAsmImm(cc,arg);
	  arg->num.abs_cnts=cc->abs_cnts;
	}
      }
      if (cc->token!=']')
	LexExcept(cc,"missing ']' at ");
      Lex(cc); //skip ]
      return;
    } else
      goto pa_asm_direct_imm;
  }
}

I64 AsmMakeArgMask(CCmpCtrl *cc,CAsmArg *arg)
{
  CAOTCtrl *aotc=cc->aotc;
  I64 result;
  if (arg->just_seg) {
    switch (arg->seg) {
      case 0: result=1<<ARGT_ES|1<<ARGT_SREG; break;
      case 1: result=1<<ARGT_CS|1<<ARGT_SREG; break;
      case 2: result=1<<ARGT_SS|1<<ARGT_SREG; break;
      case 3: result=1<<ARGT_DS|1<<ARGT_SREG; break;
      case 4: result=1<<ARGT_FS|1<<ARGT_SREG; break;
      case 5: result=1<<ARGT_GS|1<<ARGT_SREG; break;
    }
    goto mm_done;
  }
  if (arg->fstk!=REG_NONE) {
    if (arg->fstk)
      result=1<<ARGT_STI;
    else
      result=1<<ARGT_ST0|1<<ARGT_STI;
    goto mm_done;
  }
  result=cmp.size_arg_mask[arg->size];
  if (aotc->seg_size==64)
    result&=0xFF0FFFFFFF;

  if (arg->reg1!=REG_NONE && arg->imm_or_off_present && !arg->num.i &&
	!arg->num.glbl_asm_undef_hash && !arg->num.local_asm_undef_hash)
    arg->imm_or_off_present=FALSE;  //Zero displacement

  if (arg->reg2!=REG_NONE || arg->scale!=1) {
    result&=0x0000FF0000;
    goto mm_done;
  }

  if (arg->indirect) {
    if (arg->imm_or_off_present)
      result&=0x00FFFF0000;
    else
      result&=0x000FFF0000;
  } else {
    if (arg->imm_or_off_present)
      result&=0x000F000FFE;
    else
      result&=0x3F0FFFF000;
  }
  if (arg->seg!=REG_NONE)
    result&=0x00FFFF0000;
  if (arg->reg1==REG_NONE) {
    if (arg->indirect)
      result&=0x00FFFF0000;
    else if (arg->num.i<0) {
      if (arg->num.i>=MIN_I8)
	result&=0x8FE;
      else if (arg->num.i>=MIN_I16)
	result&=0x8EE;
      else if (arg->num.i>=MIN_I32)
	result&=0x8CE;
      else
	result&=0x88E;
    } else {
      if (arg->num.i<=MAX_I8)
	result&=0xFFE;
      else if (arg->num.i<=MAX_U8)
	result&=0xFEE;
      else if (arg->num.i<=MAX_I16)
	result&=0xEEE;
      else if (arg->num.i<=MAX_U16)
	result&=0xECE;
      else if (arg->num.i<=MAX_I32)
	result&=0xCCE;
      else if (arg->num.i<=MAX_U32)
	result&=0xC8E;
      else
	result&=0x88E;
    }
  } else {
    result&=  0x3F00FFF000;
    if (!arg->indirect) //M8-M64
      result&=0xFFFF0FFFFF;
  }
  switch (arg->reg1) {
    case REG_RAX: result&=~0x3000000000; break;
    case REG_RCX: result&=~0x2F00000000; break;
    case REG_RDX: result&=~0x1F00000000; break;
    default:	  result&=~0x3F00000000;
  }
mm_done:
  return result;
}

Bool AsmStoreNum(CCmpCtrl *cc,CAsmNum2 *num2,I64 cnt,Bool U8_avail)
{
  CAOTCtrl *aotc=cc->aotc;
  I64 i;
  CAOTAbsAddr *tempa;

  if (!num2->imm_flag)
    num2->num.i-=num2->rel;
  for (i=0;i<cnt;i++) {
    if (num2->U8_cnt==1) {
      if (num2->num.local_asm_undef_hash||num2->num.glbl_asm_undef_hash)
	AsmUnresolvedAdd(cc,num2->num.machine_code,IET_REL_I8+num2->imm_flag,
	      aotc->ip,num2->rel,num2->num.local_asm_undef_hash,
	      num2->num.glbl_asm_undef_hash,cc->lex_include_stk->line_num,
	      U8_avail);
      else if (!num2->imm_flag && !(MIN_I8<=num2->num.i<=MAX_I8))
	LexExcept(cc,"branch out of range at ");
      if (num2->imm_flag) {
	if (num2->num.abs_cnts.abs_addres&1) {
	  tempa=CAlloc(sizeof(CAOTAbsAddr));
	  tempa->next=aotc->abss;
	  aotc->abss=tempa;
	  tempa->ip=aotc->ip;
	  tempa->type=AAT_ADD_U8;
	}
      } else {
	if (num2->num.abs_cnts.c_addres&1) {
	  tempa=CAlloc(sizeof(CAOTAbsAddr));
	  tempa->next=aotc->abss;
	  aotc->abss=tempa;
	  tempa->ip=aotc->ip;
	  tempa->type=AAT_SUB_U8;
	}
      }
      AOTStoreCodeU8(cc,num2->num.i);
    } else {
      if (num2->U8_cnt==2) {
	if (num2->num.local_asm_undef_hash||num2->num.glbl_asm_undef_hash)
	  AsmUnresolvedAdd(cc,num2->num.machine_code,IET_REL_I16+num2->imm_flag,
		aotc->ip,num2->rel,num2->num.local_asm_undef_hash,
		num2->num.glbl_asm_undef_hash,cc->lex_include_stk->line_num,
		U8_avail);
	else if (!num2->imm_flag && !(MIN_I16<=num2->num.i<=MAX_I16))
	  LexExcept(cc,"branch out of range at ");
	if (num2->imm_flag) {
	  if (num2->num.abs_cnts.abs_addres&1) {
	    tempa=CAlloc(sizeof(CAOTAbsAddr));
	    tempa->next=aotc->abss;
	    aotc->abss=tempa;
	    tempa->ip=aotc->ip;
	    tempa->type=AAT_ADD_U16;
	  }
	} else {
	  if (num2->num.abs_cnts.c_addres&1) {
	    tempa=CAlloc(sizeof(CAOTAbsAddr));
	    tempa->next=aotc->abss;
	    aotc->abss=tempa;
	    tempa->ip=aotc->ip;
	    tempa->type=AAT_SUB_U16;
	  }
	}
	AOTStoreCodeU8(cc,num2->num.i.u8[0]);
	AOTStoreCodeU8(cc,num2->num.i.u8[1]);
      } else if (num2->U8_cnt==4) {
	if (num2->num.local_asm_undef_hash||num2->num.glbl_asm_undef_hash)
	  AsmUnresolvedAdd(cc,num2->num.machine_code,IET_REL_I32+num2->imm_flag,
		aotc->ip,num2->rel,num2->num.local_asm_undef_hash,
		num2->num.glbl_asm_undef_hash,cc->lex_include_stk->line_num,
		U8_avail);
	else if (!num2->imm_flag && !(MIN_I32<=num2->num.i<=MAX_I32))
	  LexExcept(cc,"branch out of range at ");
	if (num2->imm_flag) {
	  if (num2->num.abs_cnts.abs_addres&1) {
	    tempa=CAlloc(sizeof(CAOTAbsAddr));
	    tempa->next=aotc->abss;
	    aotc->abss=tempa;
	    tempa->ip=aotc->ip;
	    tempa->type=AAT_ADD_U32;
	  }
	} else {
	  if (num2->num.abs_cnts.c_addres&1) {
	    tempa=CAlloc(sizeof(CAOTAbsAddr));
	    tempa->next=aotc->abss;
	    aotc->abss=tempa;
	    tempa->ip=aotc->ip;
	    tempa->type=AAT_SUB_U32;
	  }
	}
	AOTStoreCodeU32(cc,num2->num.i);
      } else if (num2->U8_cnt==8) {
	if (num2->num.local_asm_undef_hash||num2->num.glbl_asm_undef_hash)
	  AsmUnresolvedAdd(cc,num2->num.machine_code,IET_REL_I64+num2->imm_flag,
		aotc->ip,num2->rel,num2->num.local_asm_undef_hash,
		num2->num.glbl_asm_undef_hash,cc->lex_include_stk->line_num,
		U8_avail);
	if (num2->imm_flag) {
	  if (num2->num.abs_cnts.abs_addres&1) {
	    tempa=CAlloc(sizeof(CAOTAbsAddr));
	    tempa->next=aotc->abss;
	    aotc->abss=tempa;
	    tempa->ip=aotc->ip;
	    tempa->type=AAT_ADD_U64;
	  }
	} else {
	  if (num2->num.abs_cnts.c_addres&1) {
	    tempa=CAlloc(sizeof(CAOTAbsAddr));
	    tempa->next=aotc->abss;
	    aotc->abss=tempa;
	    tempa->ip=aotc->ip;
	    tempa->type=AAT_SUB_U64;
	  }
	}
	AOTStoreCodeU64(cc,num2->num.i);
      }
      if (U8_avail && !num2->num.local_asm_undef_hash &&
	    !num2->num.glbl_asm_undef_hash &&
	    !num2->imm_flag && -124<=num2->num.i<=123) {
	LexWarn(cc,"could use I8 displacement at ");
	return FALSE;
      }
    }
  }
  return TRUE;
}

U8 asm_seg_prefixes[6]={0x26,0x2E,0x36,0x3E,0x64,0x65};

Bool PrsAsmInstruction(CCmpCtrl *cc,CHashOpcode *tempo,I64 argcnt)
{
  CAOTCtrl *aotc=cc->aotc;
  I64 i,j,a1,a2,om,seg,arg1mask,arg2mask;
  CAsmArg *tempa1,*tempa2;
  Bool ModrM_complete,U8_avail=FALSE,found_second_possible=FALSE;
  CInstruction *tempi;

  CAsmIns cur,best;
  best.U8_cnt=255;

  if (argcnt>0)
    arg1mask=AsmMakeArgMask(cc,&aotc->arg1);
  else
    arg1mask=1;
  if (argcnt>1)
    arg2mask=AsmMakeArgMask(cc,&aotc->arg2);
  else
    arg2mask=1;
  for (i=0;i<tempo->instruction_entry_cnt;i++) {
    tempi=&tempo->ins[i];
    if (tempi->arg1==ARGT_REL8 || tempi->arg2==ARGT_REL8)
      U8_avail=TRUE;
    if (Bt(&arg1mask,tempi->arg1) && Bt(&arg2mask,tempi->arg2) &&
	(!(tempi->flags&IEF_NOT_IN_64_BIT) || aotc->seg_size!=64)) {
      MemSet(&cur,0,sizeof(CAsmIns));
      cur.tempi=tempi;
      ModrM_complete=FALSE;
      cur.is_dft=ToBool(tempi->flags & IEF_DFT);
      if (aotc->seg_size==64) {
	if (tempi->flags & IEF_48_REX)
	  cur.REX=0x48;
	else
	  cur.REX=0x40;
      }
      cur.disp.imm_flag=TRUE;
      cur.imm.imm_flag=TRUE;
      om=tempi->opcode_modifier;
      a1=tempi->arg1;
      a2=tempi->arg2;
      tempa1=&aotc->arg1;
      tempa2=&aotc->arg2;
      cur.last_opcode_U8=tempi->opcode[tempi->opcode_cnt-1];

      if (tempi->slash_val<8) {
	cur.ModrM|=tempi->slash_val<<3;
	cur.has_ModrM=TRUE;
      }

      if (aotc->seg_size==16 && tempi->flags & IEF_OP_SIZE32 ||
	    aotc->seg_size!=16 && tempi->flags & IEF_OP_SIZE16)
	cur.has_operand_prefix=TRUE;

      if (om==OM_IB)	  cur.imm.U8_cnt=1;
      else if (om==OM_IW) cur.imm.U8_cnt=2;
      else if (om==OM_ID) cur.imm.U8_cnt=4;

      if (om==OM_CB) {
	cur.imm.U8_cnt=1;
	cur.imm.imm_flag=FALSE;
      } else if (om==OM_CW) {
	cur.imm.U8_cnt=2;
	cur.imm.imm_flag=FALSE;
      } else if (om==OM_CD) {
	cur.imm.U8_cnt=4;
	cur.imm.imm_flag=FALSE;
      }

      if (argcnt==1) {
	if (best.U8_cnt!=255 && !found_second_possible && !best.is_dft) {
	  found_second_possible=TRUE;
	  if (!aotc->arg1.size)
	    PrintWarn("no size specified at %s,%04d\n",
		  cc->lex_include_stk->full_name,
		  cc->lex_include_stk->line_num-1);
	}
	if (tempi->flags & IEF_PLUS_OPCODE) {
	  if (tempi->slash_val==SV_R_REG) {
	    cur.last_opcode_U8|=tempa1->reg1&7;
	    if (tempa1->reg1&15>7)
	      cur.REX|=1;
	    if (tempa1->reg1>=20) //RBPu8,RSPu8,RSIu8,RDIu8?
	      cur.has_REX=TRUE;
	  } else {//SV_I_REG
	    if (tempa1->fstk!=REG_NONE)
	      cur.last_opcode_U8+=tempa1->fstk;
	  }
	}
	if (a1==ARGT_R64 || a1==ARGT_RM64 || a1==ARGT_M64)
	  cur.REX|=8;
	if (ARGT_RM8<=a1<=ARGT_RM64 || ARGT_M8<=a1<=ARGT_M64) {
	  if (aotc->seg_size==16)
	    cur.has_addr_prefix=TRUE;

	  cur.has_ModrM=TRUE;
	  if (tempa1->imm_or_off_present && tempa1->indirect &&
		tempa1->reg1==REG_NONE) {
	    cur.ModrM=cur.ModrM+5;
	    MemCpy(&cur.disp.num,&tempa1->num,sizeof(CAsmNum));
	    cur.disp.U8_cnt=4;
	    if (aotc->seg_size==64)
	      cur.disp.imm_flag=FALSE;
	  } else {
	    if (tempa1->reg2==REG_NONE && tempa1->scale==1) {
	      cur.ModrM|=tempa1->reg1&7;
	      if (tempa1->reg1&15>7)
		cur.REX|=1;
	      if (tempa1->reg1>=20) //RBPu8,RSPu8,RSIu8,RDIu8?
		cur.has_REX=TRUE;
	    } else {
	      cur.ModrM|=4;
	      cur.has_SIB=TRUE;
	      if (tempa1->scale==1)
		cur.SIB=0;
	      else if (tempa1->scale==2)
		cur.SIB=0x40;
	      else if (tempa1->scale==4)
		cur.SIB=0x80;
	      else if (tempa1->scale==8)
		cur.SIB=0xC0;
	      if (tempa1->reg2==REG_NONE) {
		ModrM_complete=TRUE;
		cur.SIB|=(tempa1->reg1&7)<<3+REG_RBP;
		if (tempa1->reg1&15>7)
		  cur.REX|=2;
		if (tempa1->reg1>=20) //RBPu8,RSPu8,RSIu8,RDIu8?
		  cur.has_REX=TRUE;
		MemCpy(&cur.disp.num,&tempa1->num,sizeof(CAsmNum));
		cur.disp.U8_cnt=4;
	      } else {
		cur.SIB|=(tempa1->reg1&7)<<3+tempa1->reg2&7;
		if (tempa1->reg1&15>7)
		  cur.REX|=2;
		if (tempa1->reg1>=20) //RBPu8,RSPu8,RSIu8,RDIu8?
		  cur.has_REX=TRUE;
		if (tempa1->reg2&15>7)
		  cur.REX|=1;
		if (tempa1->reg2>=20) //RBPu8,RSPu8,RSIu8,RDIu8?
		  cur.has_REX=TRUE;
		if (tempa1->reg2&7==REG_RBP &&
		      !tempa1->imm_or_off_present && tempa1->indirect) {
		  cur.ModrM|=0x40;
		  cur.disp.U8_cnt=1;
		  ModrM_complete=TRUE;
		}
	      }
	    }
	    if (!ModrM_complete) {
	      if (tempa1->imm_or_off_present) {
		MemCpy(&cur.disp.num,&tempa1->num,sizeof(CAsmNum));
		if (!cur.disp.num.machine_code &&
		      MIN_I8<=cur.disp.num.i<=MAX_I8) {
		  cur.ModrM|=0x40;
		  cur.disp.U8_cnt=1;
		} else if (aotc->seg_size==16) {
		  cur.ModrM|=0x80;
		  cur.disp.U8_cnt=2;
		} else {
		  cur.ModrM|=0x80;
		  cur.disp.U8_cnt=4;
		}
	      } else if (!tempa1->indirect) {
		cur.has_addr_prefix=FALSE;
		cur.ModrM|=0xC0;
	      } else {
		if (tempa1->reg1&7==REG_RBP) {
		  cur.ModrM|=0x40;
		  cur.disp.U8_cnt=1;
		}
	      }
	    }
	  }
	} else if (ARGT_REL8<=a1<=ARGT_REL32 ||
	      ARGT_IMM8<=a1<=ARGT_IMM64 ||
	      ARGT_UIMM8<=a1<=ARGT_UIMM64) {
	  if (a1==ARGT_IMM64 || a2==ARGT_UIMM64)
	    cur.REX|=8;
	  MemCpy(&cur.imm.num,&tempa1->num,sizeof(CAsmNum));
	}
      } else if (argcnt==2) {
	if (best.U8_cnt!=255 && !found_second_possible && !best.is_dft) {
	  found_second_possible=TRUE;
	  if (!aotc->arg1.size && !aotc->arg2.size)
	    PrintWarn("no size specified at %s,%04d\n",
		  cc->lex_include_stk->full_name,
		  cc->lex_include_stk->line_num-1);
	}
	if (tempi->flags & IEF_PLUS_OPCODE) {
	  if (tempi->slash_val==SV_R_REG) {
	    if (ARGT_AL<=a1<=ARGT_RAX) {
	      cur.last_opcode_U8|=tempa2->reg1&7;
	      if (tempa2->reg1&15>7)
		cur.REX|=1;
	      if (tempa2->reg1>=20) //RBPu8,RSPu8,RSIu8,RDIu8?
		cur.has_REX=TRUE;
	    } else {
	      cur.last_opcode_U8|=tempa1->reg1&7;
	      if (tempa1->reg1&15>7)
		cur.REX|=1;
	      if (tempa1->reg1>=20) //RBPu8,RSPu8,RSIu8,RDIu8?
		cur.has_REX=TRUE;
	    }
	  } else {//SV_I_REG
	    if (tempa1->fstk!=REG_NONE)
	      cur.last_opcode_U8|=tempa1->fstk;
	    if (tempa2->fstk!=REG_NONE)
	      cur.last_opcode_U8|=tempa2->fstk;
	  }
	}
	if (a1==ARGT_RM64 || a2==ARGT_RM64 ||
	      a1==ARGT_M64 || a2==ARGT_M64 ||
	      a1==ARGT_R64 || a2==ARGT_R64)
	  cur.REX|=8;
	if (ARGT_RM8<=a1<=ARGT_RM64 || ARGT_RM8<=a2<=ARGT_RM64 ||
	      ARGT_M8 <=a1<=ARGT_M64  || ARGT_M8 <=a2<=ARGT_M64) {
	  if (aotc->seg_size==16)
	    cur.has_addr_prefix=TRUE;
	  cur.has_ModrM=TRUE;
	  if (ARGT_RM8<=a2<=ARGT_RM64 || ARGT_M8<=a2<=ARGT_M64) {
	    tempa1=&aotc->arg2;
	    tempa2=&aotc->arg1;
	  }
	  if (tempi->slash_val==SV_R_REG) {
	    if (tempa2->just_seg)
	      cur.ModrM|=tempa2->seg<<3;
	    else {
	      if (tempa2->reg1==REG_NONE) {
		cur.ModrM|=(tempa1->reg1&7)<<3;
		if (tempa1->reg1&15>7)
		  cur.REX|=4;
		if (tempa1->reg1>=20) //RBPu8,RSPu8,RSIu8,RDIu8?
		  cur.has_REX=TRUE;
	      } else {
		cur.ModrM|=(tempa2->reg1&7)<<3;
		if (tempa2->reg1&15>7)
		  cur.REX|=4;
		if (tempa2->reg1>=20) //RBPu8,RSPu8,RSIu8,RDIu8?
		  cur.has_REX=TRUE;
	      }
	    }
	  }
	  if (tempa1->reg2==REG_NONE && tempa1->scale==1) {
	    if (tempa1->reg1!=REG_NONE) {
	      cur.ModrM|=tempa1->reg1&7;
	      if (tempa1->reg1&15>7)
		cur.REX|=1;
	      if (tempa1->reg1>=20) //RBPu8,RSPu8,RSIu8,RDIu8?
		cur.has_REX=TRUE;
	    }
	  } else {
	    cur.ModrM|=4;
	    cur.has_SIB=TRUE;
	    if (tempa1->scale==1)
	      cur.SIB=0;
	    else if (tempa1->scale==2)
	      cur.SIB=0x40;
	    else if (tempa1->scale==4)
	      cur.SIB=0x80;
	    else if (tempa1->scale==8)
	      cur.SIB=0xC0;
	    if (tempa1->reg2==REG_NONE) {
	      ModrM_complete=TRUE;
	      cur.SIB|=(tempa1->reg1&7)<<3+5;
	      if (tempa1->reg1&15>7)
		cur.REX|=2;
	      if (tempa1->reg1>=20) //RBPu8,RSPu8,RSIu8,RDIu8?
		cur.has_REX=TRUE;
	      MemCpy(&cur.disp.num,&tempa1->num,sizeof(CAsmNum));
	      cur.disp.U8_cnt=4;
	    } else {
	      cur.SIB|=(tempa1->reg1&7)<<3+tempa1->reg2&7;
	      if (tempa1->reg1&15>7)
		cur.REX|=2;
	      if (tempa1->reg1>=20) //RBPu8,RSPu8,RSIu8,RDIu8?
		cur.has_REX=TRUE;
	      if (tempa1->reg2&15>7)
		cur.REX|=1;
	      if (tempa1->reg2>=20) //RBPu8,RSPu8,RSIu8,RDIu8?
		cur.has_REX=TRUE;
	      if (tempa1->reg2&7==REG_RBP &&
		    !tempa1->imm_or_off_present && tempa1->indirect) {
		cur.ModrM|=0x40;
		cur.disp.U8_cnt=1;
		ModrM_complete=TRUE;
	      }
	    }
	  }
	  if (!ModrM_complete) {
	    if (tempa1->imm_or_off_present &&
		  tempa1->indirect && tempa1->reg1==REG_NONE) {
	      cur.ModrM=cur.ModrM&0xF8+5;
	      MemCpy(&cur.disp.num,&tempa1->num,sizeof(CAsmNum));
	      cur.disp.U8_cnt=4;
	      if (aotc->seg_size==64)
		cur.disp.imm_flag=FALSE;
	    } else {
	      if (tempa1->imm_or_off_present) {
		MemCpy(&cur.disp.num,&tempa1->num,sizeof(CAsmNum));
		if (!cur.disp.num.machine_code &&
		      MIN_I8<=cur.disp.num.i<=MAX_I8) {
		  cur.ModrM|=0x40;
		  cur.disp.U8_cnt=1;
		} else if (aotc->seg_size==16) {
		  cur.ModrM|=0x80;
		  cur.disp.U8_cnt=2;
		} else {
		  cur.ModrM|=0x80;
		  cur.disp.U8_cnt=4;
		}
	      } else if (!tempa1->indirect) {
		cur.has_addr_prefix=FALSE;
		cur.ModrM|=0xC0;
	      } else {
		if (tempa1->reg1&7==REG_RBP) {
		  cur.ModrM|=0x40;
		  cur.disp.U8_cnt=1;
		}
	      }
	    }
	  }
	} else if (ARGT_MOFFS8<=a1<=ARGT_MOFFS64) {
	  MemCpy(&cur.disp.num,&tempa1->num,sizeof(CAsmNum));
	  if (aotc->seg_size==16)
	    cur.disp.U8_cnt=2;
	  else
	    cur.disp.U8_cnt=4;
	  cur.has_addr_prefix=FALSE;
	} else if (ARGT_MOFFS8<=a2<=ARGT_MOFFS64) {
	  MemCpy(&cur.disp.num,&tempa2->num,sizeof(CAsmNum));
	  if (aotc->seg_size==16)
	    cur.disp.U8_cnt=2;
	  else
	    cur.disp.U8_cnt=4;
	  cur.has_addr_prefix=FALSE;
	} else if (ARGT_IMM8<=a1<=ARGT_IMM64 ||
	      ARGT_UIMM8<=a1<=ARGT_UIMM64) {
	  MemCpy(&cur.imm.num,&tempa1->num,sizeof(CAsmNum));
	  if (a1==ARGT_IMM8 || a1==ARGT_UIMM8) cur.imm.U8_cnt=1;
	  else if (a1==ARGT_IMM16 || a1==ARGT_UIMM16) cur.imm.U8_cnt=2;
	  else if (a1==ARGT_IMM32 || a1==ARGT_UIMM32) cur.imm.U8_cnt=4;
	  else {
	    cur.imm.U8_cnt=8;
	    cur.REX|=8;
	  }
	}
	if (ARGT_IMM8<=a2<=ARGT_IMM64 ||
	      ARGT_UIMM8<=a2<=ARGT_UIMM64) {
	  MemCpy(&cur.imm.num,&tempa2->num,sizeof(CAsmNum));
	  if (a2==ARGT_IMM8 || a2==ARGT_UIMM8) cur.imm.U8_cnt=1;
	  else if (a2==ARGT_IMM16 || a2==ARGT_UIMM16) cur.imm.U8_cnt=2;
	  else if (a2==ARGT_IMM32 || a2==ARGT_UIMM32) {
	    cur.imm.U8_cnt=4;
	    if (tempi->flags&IEF_REX_ONLY_R8_R15 && a2==ARGT_UIMM32)
	      cur.REX&=~8;
	  } else {
	    cur.imm.U8_cnt=8;
	    cur.REX|=8;
	  }
	}
      }
      cur.U8_cnt=tempi->opcode_cnt+cur.disp.U8_cnt+cur.imm.U8_cnt;
      if (cur.has_ModrM) cur.U8_cnt++;
      if (cur.has_SIB) cur.U8_cnt++;
      if (aotc->seg_size==64 && cur.REX&0x40==0x40 &&
	    (cur.REX!=0x40 || cur.has_REX) &&
	    (cur.REX&7 || !(tempi->flags&IEF_REX_ONLY_R8_R15 ||
	    tempi->flags&IEF_REX_XOR_LIKE && tempa1->reg1==tempa2->reg1 &&
	    cur.ModrM&0xC0==0xC0)))
	cur.U8_cnt++;
      if (cur.U8_cnt<best.U8_cnt &&
	    !(tempi->flags & IEF_DONT_SWITCH_MODES &&
	    (cur.has_addr_prefix || cur.has_operand_prefix)))
	MemCpy(&best,&cur,sizeof(CAsmIns));
    }
  }
  if (best.U8_cnt<255) {
    tempi=best.tempi;
    seg=REG_NONE;
    if (argcnt>1 && aotc->arg2.seg!=REG_NONE && !aotc->arg2.just_seg)
      seg=aotc->arg2.seg;
    else if (argcnt>0 && aotc->arg1.seg!=REG_NONE && !aotc->arg1.just_seg)
      seg=aotc->arg1.seg;
    if (seg!=REG_NONE)
      AOTStoreCodeU8(cc,asm_seg_prefixes[seg]);
    if (best.has_operand_prefix)
      AOTStoreCodeU8(cc,OC_OP_SIZE_PREFIX); //Operand size override
    if (best.has_addr_prefix  || aotc->seg_size==16 && cur.has_SIB)
      AOTStoreCodeU8(cc,OC_ADDR_SIZE_PREFIX); //Operand size override
    if (aotc->seg_size==64 && best.REX&0x40==0x40 &&
	  (best.REX!=0x40 || best.has_REX) &&
	  (best.REX&7||!(tempi->flags&IEF_REX_ONLY_R8_R15 ||
	  tempi->flags&IEF_REX_XOR_LIKE && tempa1->reg1==tempa2->reg1 &&
	  best.ModrM&0xC0==0xC0)))
      AOTStoreCodeU8(cc,best.REX);
    for (j=0;j<tempi->opcode_cnt-1;j++)
      AOTStoreCodeU8(cc,tempi->opcode[j]);
    AOTStoreCodeU8(cc,best.last_opcode_U8);

    if (best.has_ModrM)
      AOTStoreCodeU8(cc,best.ModrM);
    if (best.has_SIB)
      AOTStoreCodeU8(cc,best.SIB);

    if (best.disp.U8_cnt) {
      best.disp.rel=aotc->ip+best.disp.U8_cnt+best.imm.U8_cnt;
      if (!AsmStoreNum(cc,&best.disp,1,U8_avail))
	return FALSE;
    }

    if (best.imm.U8_cnt) {
      best.imm.rel=aotc->ip+best.imm.U8_cnt;
      if (!AsmStoreNum(cc,&best.imm,1,U8_avail))
	return FALSE;
    }
    if (tempi->flags&IEF_ENDING_ZERO) //ENTER instruction
      AOTStoreCodeU8(cc,0);
    return TRUE;
  }
  LexExcept(cc,"invalid instruction at ");
}

U0 PrsAsmDefine(CCmpCtrl *cc,I64 U8_cnt)
{
  Bool is_dup;
  I64 i,dup_val;
  U8 *ptr;
  CAsmNum2 num2;
  num2.U8_cnt=U8_cnt;

  while (cc->token && cc->token!=';') {
    num2.num.local_asm_undef_hash=NULL;
    num2.num.glbl_asm_undef_hash=NULL;
    if (cc->token==TK_STR) {
      ptr=cc->cur_str;
      i=cc->cur_str_len-1;
      while (i--)
	AOTStoreCodeU8(cc,*ptr++);
      Lex(cc);	//Skip Str
    } else {
      is_dup=FALSE;
      cc->abs_cnts=0;
      cc->asm_undef_hash=NULL;
      cc->flags&=~(CCF_UNRESOLVED+CCF_LOCAL);
      if (!IsLexExpression2Bin(cc,&num2.num.machine_code))
	LexSkipEol(cc);
      else {
	if (cc->abs_cnts.externs)
	  LexExcept(cc,"Extern Not Allowed at ");
	if (cc->flags & CCF_UNRESOLVED) {
	  if (cc->flags & CCF_LOCAL) {
	    num2.num.local_asm_undef_hash=cc->asm_undef_hash;
	    cc->asm_undef_hash=NULL;
	  } else {
	    num2.num.glbl_asm_undef_hash=cc->asm_undef_hash;
	    cc->asm_undef_hash=NULL;
	  }
	} else {
	  i=Call(num2.num.machine_code);
	  Free(num2.num.machine_code);
	}
      }
      if (cc->token==TK_IDENT && cc->hash_entry) {
	if (cc->hash_entry->type & HTT_ASM_KEYWORD &&
	      cc->hash_entry->user_data0==AKW_DUP) {
	  is_dup=TRUE;
	  if (Lex(cc)!='(')
	    LexExcept(cc,"expecting '(' at ");
	  Lex(cc); //skip (
	  dup_val=AsmLexExpression(cc);
	  if (cc->token!=')')
	    LexExcept(cc,"expecting ')' at ");
	  Lex(cc); //SKIP )
	}
      }
      num2.rel=0;
      num2.imm_flag=TRUE;
      num2.num.abs_cnts=cc->abs_cnts;
      if (is_dup) {
	if (num2.num.local_asm_undef_hash || num2.num.glbl_asm_undef_hash)
	  LexExcept(cc,"undefined DUP cnt at ");
	num2.num.i=dup_val;
	AsmStoreNum(cc,&num2,i,FALSE);
      } else {
	num2.num.i=i;
	AsmStoreNum(cc,&num2,1,FALSE);
      }
    }
    if (cc->token==',')
      Lex(cc);
  }
  if (cc->token!=';')
    LexExcept(cc,"missing ';' at");
  Lex(cc);
}

U0 PrsBinFile(CCmpCtrl *cc)
{
  I64 i,size;
  U8 *buf,*st;
  if (cc->token!=TK_STR)
    LexExcept(cc,"expecting string at ");
  st=DftExt(cc->cur_str,"BIN");
  buf=FileRead(st,&size);
  Free(st);
  for (i=0;i<size;i++)
    AOTStoreCodeU8(cc,buf[i]);
  if (Lex(cc)!=';')
    LexExcept(cc,"missing ';' at");
  Lex(cc);
}

U0 PrsAsmBlk(CCmpCtrl *cc,I64 cmp_flags)
{
  CAOTCtrl *aotc=cc->aotc;
  I64 i,j,k,argcnt,
	old_flags=cc->flags & CCF_ASM_EXPRESSIONS;
  CHashOpcode *tempo;
  CHashExport *tempex;
  U8 *next_last_label;
  CCodeMisc *g_lb;

  aotc->seg_size=64;
  cc->flags|=CCF_ASM_EXPRESSIONS;
  if (!(cmp_flags&CMPF_ONE_ASM_INS)) {
    if (cc->token!='{')
      LexExcept(cc,"expecting '{' at ");
    Lex(cc);
  }
  while (cc->token && cc->token!='}') {
    AsmLineLst(cc);
    if (cc->token==TK_IDENT && cc->hash_entry) {
      if (cc->hash_entry->type&HTT_ASM_KEYWORD) {
	i=cc->hash_entry->user_data0;
	Lex(cc); //skip keyword
	switch (i) {
	  case AKW_IMPORT:
	    while (cc->token && cc->token!=';') {
	      if (cc->token!=TK_IDENT)
		LexExcept(cc,"expecting identifier at ");
	      else {
		tempex=NULL;
		tempex=CAlloc(sizeof(CHashExport));
		tempex->str=cc->cur_str;
		cc->cur_str=0;
		tempex->type=HTT_EXPORT_SYS_SYM|HTF_UNRESOLVED;
		HashAdd(tempex,cc->htc.glbl_hash_table);
		tempex->type|=HTF_IMPORT;
		if (Lex(cc)==',')
		  Lex(cc); //skip ','
	      }
	    }
	    if (cc->token!=';')
	      LexExcept(cc,"missing ';' at");
	    Lex(cc); //skip ';';
	    break;
	  case AKW_MODULE_ORG:
	    if (cc->htc.local_var_lst)
	      LexExcept(cc,"ORG not allowed in fun asm blk ");
	    if (aotc->module_org!=INVALID_PTR)
	      LexExcept(cc,"Just one org allowed ");
	    if (aotc->ip)
	      LexExcept(cc,"ORG must be at beginning ");
	    aotc->module_org=AsmLexExpression(cc);
	    break;
	  case AKW_ALIGN:
	    if (cc->htc.local_var_lst)
	      LexExcept(cc,"ALIGN not allowed in fun asm blk ");
	    i=AsmLexExpression(cc);
	    j=Bsf(i);
	    if (!i || j!=Bsr(i))
	      LexExcept(cc,"ALIGN must be power of two at ");
	    if (!(cc->flags&CCF_AOT_COMPILE) && i>8)
	      LexExcept(cc,"In JIT mode, max ALIGN is 8 ");
	    if (j>aotc->max_align_bits) aotc->max_align_bits=j;
	    i=CeilU64(aotc->ip,i);
	    if (cc->token!=',')
	      LexExcept(cc,"expecting ',' at ");
	    Lex(cc);
	    k=AsmLexExpression(cc);
	    for (j=aotc->ip;j<i;j++)
	      AOTStoreCodeU8(cc,k);
	    break;
	  case AKW_DU8:
	    PrsAsmDefine(cc,1);
	    break;
	  case AKW_DU16:
	    PrsAsmDefine(cc,2);
	    break;
	  case AKW_DU32:
	    PrsAsmDefine(cc,4);
	    break;
	  case AKW_DU64:
	    PrsAsmDefine(cc,8);
	    break;
	  case AKW_BINFILE:
	    PrsBinFile(cc);
	    break;
	  case AKW_LIST:
	    aotc->lst=TRUE;
	    break;
	  case AKW_NOLIST:
	    aotc->lst=FALSE;
	    break;
	  case AKW_USE16:
	    aotc->seg_size=16;
	    break;
	  case AKW_USE32:
	    aotc->seg_size=32;
	    break;
	  case AKW_USE64:
	    aotc->seg_size=64;
	    break;
	  default:
	    LexExcept(cc,"syntax error at ");
	}
      } else if (cc->hash_entry->type & HTT_OPCODE) {
	tempo=cc->hash_entry;
	Lex(cc); //skip opcode
	argcnt=0;
	if (tempo->ins[0].arg1) {
	  argcnt++;
	  if (ARGT_REL8<=tempo->ins[0].arg1<=ARGT_REL32)
	    PrsAsmArg(cc,&aotc->arg1,TRUE);
	  else
	    PrsAsmArg(cc,&aotc->arg1,FALSE);
	  if (tempo->ins[0].arg2) {
	    argcnt++;
	    if (cc->token!=',')
	      LexExcept(cc,"expecting ',' at ");
	    else {
	      Lex(cc); //skip ','
	      if (ARGT_REL8<=tempo->ins[0].arg2<=ARGT_REL32)
		PrsAsmArg(cc,&aotc->arg2,TRUE);
	      else
		PrsAsmArg(cc,&aotc->arg2,FALSE);
	    }
	  }
	}
	PrsAsmInstruction(cc,tempo,argcnt);
      } else if (cc->hash_entry->type & HTT_EXPORT_SYS_SYM) {
	if (Btr(&cc->hash_entry->type,HTf_UNRESOLVED)) {
	  if (cc->hash_entry->type & HTF_LOCAL) {
	    cc->hash_entry(CHashExport *)->val=aotc->ip;
	    if (Lex(cc)!=':')
	      LexExcept(cc,"expecting ':' at ");
	    Lex(cc);
	  } else {
	    if (cc->hash_entry->type & HTF_IMPORT)
	      LexExcept(cc,"attempt to define import at ");
	    cc->hash_entry(CHashExport *)->val=aotc->ip;
	    next_last_label=cc->hash_entry->str;
	    Lex(cc); //Skip cur_str
	    if (cc->token!=':' && cc->token!=TK_DBL_COLON)
	      LexExcept(cc,"expecting ':' at ");
	    if (cc->token==TK_DBL_COLON) {
	      cc->hash_entry->type|=HTF_EXPORT;
	      HashSrcFileSet(cc,cc->hash_entry);

	      AOTLocalsResolve(cc);
	      aotc->last_label=next_last_label;
	    }
	    Lex(cc);
	  }
	} else if (cc->hash_entry(CHashExport *)->val==aotc->ip) {
	  Lex(cc); //Skip cur_str
	  if (cc->token!=':' && cc->token!=TK_DBL_COLON)
	    LexExcept(cc,"expecting ':' at ");
	  Lex(cc);
	} else
	  LexExcept(cc,"redefinition at ");
      } else
	LexExcept(cc,"syntax error at ");
    } else if (cc->token==TK_IDENT) {
      tempex=CAlloc(sizeof(CHashExport));
      tempex->str=cc->cur_str;
      cc->cur_str=0;
      tempex->type=HTT_EXPORT_SYS_SYM;
      tempex->val=aotc->ip;
      Lex(cc);	//Skip cur_str
      if (cc->token!=':' && cc->token!=TK_DBL_COLON)
	LexExcept(cc,"expecting ':' at ");
      else {
	if (*tempex->str=='@' && tempex->str[1]=='@') {
	  if (cc->token==TK_DBL_COLON)
	    LexExcept(cc,"no local glbl exports at ");
	  HashAdd(tempex,cc->htc.local_hash_table);
	} else
	  HashAdd(tempex,cc->htc.glbl_hash_table);
	if (cc->htc.local_var_lst) {//AsmBlk in fun? Also add goto-like label.
	  if (!(g_lb=COCGoToLabelFind(cc,tempex->str))) {
	    g_lb=COCMiscNew(cc,CMT_ASM_LABEL);
	    g_lb->str=StrNew(tempex->str);
	  } else if (g_lb->flags&CMF_DEFINED)
	    LexExcept(cc,"Duplicate goto label at ");
	  g_lb->type=CMT_ASM_LABEL;
	  g_lb->flags|=CMF_DEFINED;
	  g_lb->ip=aotc->ip;
	  g_lb->use_cnt++; //Disable warning on unused labels.
	  ICAdd(cc,IC_LABEL,g_lb,0);
	}
	if (cc->token==TK_DBL_COLON) {
	  tempex->type|=HTF_EXPORT;
	  HashSrcFileSet(cc,tempex);

	  AOTLocalsResolve(cc);
	  aotc->last_label=tempex->str;
	}
	Lex(cc);
      }
    } else if (cc->token==';')
      Lex(cc);
    else
      LexExcept(cc,"syntax error at ");
    if (cmp_flags&CMPF_ONE_ASM_INS && (cc->token!=TK_IDENT ||
	  !(tempo=cc->hash_entry) || !(tempo->type&HTT_OPCODE)))
      break;
  }
  AOTLocalsResolve(cc);
  aotc->lst=FALSE;
  cc->flags&=cc->flags&~CCF_ASM_EXPRESSIONS|old_flags;
}
