import U8 *Doc2PlainText(CDoc *doc,CDocEntry *doc_e);
import U0 DocBinsValidate(CDoc *doc);
import U0 DocDel(CDoc *doc);
import U0 DocLoad(CDoc *doc,U8 *src2,I64 size);
import CDoc *DocNew(U8 *filename=NULL,CTask *task=NULL);
import CDocEntry *DocPrint(CDoc *doc,U8 *fmt,...);
import CDocEntry *DocPutLine(CDoc *doc,CDocEntry *doc_e);
import I64 DocSize(CDoc *doc);
import Bool DocWrite(CDoc *doc,Bool prompt=FALSE);
extern U8 *CmdLinePmt();
extern U0 CmpFixUpAOTAsm(CCmpCtrl *cc,CAOT *tempaot);
extern U0 CmpFixUpJITAsm(CCmpCtrl *cc,CAOT *tempaot);
extern CAOT *CmpJoin(
	CCmpCtrl *cc,I64 cmp_flags,U8 *map_name=NULL,U8 mapfile_drv_let=0);
extern I64 HashEntrySize2(CHashSrcSym *temph);
extern Bool IsLexExpression2Bin(CCmpCtrl *cc,U8 **_machine_code);
extern I64 Lex(CCmpCtrl *cc);
extern I64 LexExpression(CCmpCtrl *cc);
extern I64 LexGetChar(CCmpCtrl *cc);
extern CCodeMisc *OptLabelFwd(CCodeMisc *lb);
extern CIntermediateCode *OptPass012(CCmpCtrl *cc);
extern U0 OptPass3(CCmpCtrl *cc,COptReg *reg_offsets);
extern U0 OptPass4(CCmpCtrl *cc,COptReg *reg_offsets,I64 *_type);
extern U0 OptPass5(CCmpCtrl *cc);
extern U0 OptPass6(CCmpCtrl *cc);
extern I64 OptPass789A(
	CCmpCtrl *cc,COptReg *reg_offsets,U8 *buf,CDbgInfo **_dbg);
extern CHashClass *PrsClass(
	CCmpCtrl *cc,I64 keyword,I64 fsp_flags,Bool is_extern);
extern CHashFun *PrsFunJoin(
	CCmpCtrl *cc,CHashClass *temp_return,U8 *name,I64 fsp_flags);
extern I64 PrsKeyWord(CCmpCtrl *cc);
extern Bool PrsStatement(CCmpCtrl *cc,I64 try_cnt=0,
	CCodeMisc *lb_break=NULL,I64 cmp_flags=CMPF_PRS_SEMICOLON);
extern U0 PrsStreamBlk(CCmpCtrl *cc);
extern CHashClass *PrsType(CCmpCtrl *cc,CHashClass **tempc1,
	I64 *_mode,CMemberLst *tempm,U8 **_ident,CHashFun **_fun_ptr,
	CHashExport **_tempex,CArrayDim *tempad,I64 fsp_flags);
extern I64 PrsUnaryModifier(CCmpCtrl *cc,CPrsStk *ps,
	CMemberLst **_local_var,CArrayDim **_tempad,I64 *unary_post_prec);
extern I64 PrsUnaryTerm(
	CCmpCtrl *cc,CPrsStk *ps,CMemberLst **_local_var,CArrayDim **_tempad,
	I64 *max_prec,I64 *unary_pre_prec,I64 *paren_prec);
extern U0 PrsVarInit2(CCmpCtrl *cc,U8 **_dst,CHashClass *tempc,
	CArrayDim *tempad,U8 *data_addr_ip,U8 **_base,Bool data_heap,I64 pass);
_extern CMP_TEMPLATES			U32 cmp_templates[1];
_extern CMP_TEMPLATES_DONT_POP		U32 cmp_templates_dont_pop[1];
_extern CMP_TEMPLATES_DONT_PUSH2	U32 cmp_templates_dont_push2[1];
_extern CMP_TEMPLATES_DONT_PUSH		U32 cmp_templates_dont_push[1];
_extern CMP_TEMPLATES_DONT_PUSH_POP	U32 cmp_templates_dont_push_pop[1];
