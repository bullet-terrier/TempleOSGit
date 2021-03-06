#help_index "Compiler"
extern U0 CInit(Bool first);
public extern I64 ExeFile(U8 *name,Bool just_load=FALSE);
public extern I64 ExeFile2(U8 *name,Bool just_load=FALSE);
public extern I64 ExePrint(U8 *fmt,...);
public extern I64 ExePrint2(U8 *fmt,...);
public extern I64 ExePutS(U8 *buf,U8 *filename=NULL,Bool just_load=FALSE,
	CLexHashTableContext *htc=NULL);
public extern I64 ExePutS2(U8 *buf,U8 *filename=NULL,Bool just_load=FALSE);
public extern CCmpGlbls cmp;

#help_index "Compiler/Directive"
#help_file "::/Doc/Directives"
public extern Bool CTrace(Bool val);
public extern Bool Echo(Bool val);
public extern Bool GetOpt(I64 num);
public extern Bool OTrace(Bool val);
public extern I64 OTraceMask(I64 i=0b1001111101);
public extern Bool OptOff(I64 num);
public extern Bool OptOn(I64 num);
extern U0 StreamDir();
public extern I64 StreamExePrint(U8 *fmt,...);
public extern U0 StreamPrint(U8 *fmt,...);

#help_index "Compiler/Lex"
#help_file "::/Doc/Lex"
public extern U0 CmpCtrlDel(CCmpCtrl *cc);
public extern CCmpCtrl *CmpCtrlNew(U8 *buf=NULL,I64 flags=0,U8 *filename=NULL);
public extern I64 CmpCtrlSize(CCmpCtrl *cc);
public extern I64 IsLexExpression2Bin(
	CCmpCtrl *cc,U8 **machine_code); //FALSE=no err
public extern I64 Lex(CCmpCtrl *cc);
public extern U0 LexAttachDoc(CCmpCtrl *cc,CLexFile *tempf=NULL,
	CDoc *doc=NULL,U8 *abs_filename=NULL,CDocEntry *doc_e=NULL,I64 col=0);
public extern CD2I32 *LexD2I32(CCmpCtrl *cc,CD2I32 *p);
public extern CD3I32 *LexD3I32(CCmpCtrl *cc,CD3I32 *p);
public extern U0 LexExcept(CCmpCtrl *cc,U8 *str=NULL);
public extern I64 LexExpression(CCmpCtrl *cc);
public extern U8 *LexExpression2Bin(CCmpCtrl *cc,I64 *_type=NULL);
public extern F64 LexExpressionF64(CCmpCtrl *cc);
public extern I64 LexExpressionI64(CCmpCtrl *cc);
public extern U8 *LexExtStr(CCmpCtrl *cc,I64 *_size=NULL,Bool lex_next=TRUE);
public extern U8 *LexFirstRem(CCmpCtrl *cc,U8 *marker,I64 _len=NULL);
public extern I64 LexGetChar(CCmpCtrl *cc);
public extern U0 LexPopNoRestore(CCmpCtrl *cc);
public extern U0 LexPopRestore(CCmpCtrl *cc);
public extern U0 LexPush(CCmpCtrl *cc);
public extern U0 LexPutLine(CCmpCtrl *cc,U8 *start);
public extern U0 LexPutPos(CCmpCtrl *cc);
public extern U0 LexPutToken(CCmpCtrl *cc);
public extern U0 LexSkipEol(CCmpCtrl *cc);
public extern U8 *LexStatement2Bin(
	CCmpCtrl *cc,I64 *_type,I64 cmp_flags=CMPF_PRS_SEMICOLON);
public extern U0 LexWarn(CCmpCtrl *cc,U8 *str=NULL);
public extern CMemberLst *MemberFind(U8 *needle_str,CHashClass *haystack_class);
public extern CMemberLst *MemberFind2(
	CHashClass *needle_class,CHashClass *haystack_class);
extern U0 MemberLstDel(CHashClass *c);
public extern I64 MemberMetaData(U8 *st,CMemberLst *ml);
public extern CMemberLstMeta *MemberMetaFind(U8 *st,CMemberLst *ml);
public extern CHashClass *OptClassFwd(CHashClass *tempc);
public extern I64 PrsKeyWord(CCmpCtrl *cc);

#help_index "Compiler;Cmd Line (Typically)"
extern I64 Cmp(U8 *filename,
	U8 *map_name=NULL,U8 *out_name=NULL,U8 mapfile_drv_let=0);
#help_index "Debugging/Unassemble"
public extern U8 *U(U8 *ip,I64 cnt=20,I64 seg_size=64);
public extern U0 Ui(U8 *buf,U8 **_ip,I64 seg_size=64,
	I64 *_jmp_dst=NULL,Bool just_ins=FALSE);
public extern I64 Un(U8 *ip,I64 cnt=0x80,I64 seg_size=64);
extern CUAsmGlbls uasm;

#help_index "Hash"
public extern I64 HashEntrySize(CHashSrcSym *temph);
public extern I64 HashEntrySize2(CHashSrcSym *temph);
public extern I64 HashTableSize2(CHashTable *table);
