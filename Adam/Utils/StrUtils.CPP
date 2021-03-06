#help_index "Cmd Line (Typically)"

I64 DECompare(CDocEntry *e1,CDocEntry *e2)
{
  return StrCmp(e1->tag,e2->tag);
}

public I64 Sort(U8 *_in_name,U8 *_out_name=NULL,Bool unique=FALSE)
{//Sort lines of a text file. Removes blank lines.
  U8 *in_name,*out_name,*st;
  CDoc *doc;
  CDocEntry *doc_e,*doc_e1,**a;
  I64 i,cnt=0,result;

  if (!_in_name) return 0;
  in_name=DftExt(_in_name,"TXT.Z"),
	if (_out_name)
  out_name=DftExt(_out_name,"TXT.Z");
  else
  out_name=StrNew(in_name);

  doc=DocRead(in_name,DOCF_PLAIN_TEXT_WITH_TABS|DOCF_NO_CURSOR);
  doc_e=doc->root.next;
  while (doc_e!=doc) {
    if (doc_e->type_u8==DOCT_TEXT)
      cnt++;
    doc_e=doc_e->next;
  }
  a=MAlloc(cnt*sizeof(CDocEntry *));
  doc_e=doc->root.next;
  i=0;
  while (doc_e!=doc) {
    doc_e1=doc_e->next;
    if (doc_e->type_u8==DOCT_TEXT) {
      QueRem(doc_e);
      a[i++]=doc_e;
    } else
      DocEntryDel(doc,doc_e);
    doc_e=doc_e1;
  }
  QSortI64(a,cnt,&DECompare);

  result=0;
  st=NULL;
  for (i=0;i<cnt;i++) {
    QueIns(a[i],doc->root.last);
    if (!unique || !st || StrCmp(a[i]->tag,st)) {
      st=a[i]->tag;
      doc->cur_entry=&doc->root;
      doc->cur_col=0;
      DocPrint(doc,"\n");
      result++;
    } else
      DocEntryDel(doc,a[i]);
  }
  StrCpy(doc->filename.name,out_name);
  DocWrite(doc);

  Free(a);
  DocDel(doc);
  Free(in_name);
  Free(out_name);
  return result; //Num Lines
}

U0 SUFile(U8 *filename,I64 suf_flags,F64 indent_scale_factor)
{//String utility on a single file
//See [C:/Kernel/KernelA.HPP.Z,3695] SU Flags
  U8 *dst;
  Bool chged=FALSE;
  I64 reduced=0;
  CDoc *doc=DocRead(filename,DOCF_PLAIN_TEXT_WITH_TABS | DOCF_NO_CURSOR);
  CDocEntry *doc_e=doc->root.next;
  while (doc_e!=doc) {
    if (doc_e->type_u8==DOCT_TEXT) {
      dst=MStrUtil(doc_e->tag,suf_flags,indent_scale_factor);
      if (StrCmp(dst,doc_e->tag)) {
	reduced+=StrLen(doc_e->tag)-StrLen(dst);
	chged=TRUE;
	Free(doc_e->tag);
	doc_e->tag=dst;
      } else
	Free(dst);
    }
    doc_e=doc_e->next;
  }
  if (chged) {
    "Reduced %s by %d chars\n",filename,reduced;
    DocWrite(doc);
  }
  DocDel(doc);
}
public U0 SU(U8 *files_find_mask,I64 suf_flags,U8 *fu_flags=NULL,
	F64 indent_scale_factor=0)
{//Apply [C:/Kernel/StrA.CPP.Z,519] StrUtil() on files
//You can convert spaces to tabs, for example,
  //or removing trailing spaces on lines.
  //See [C:/Kernel/KernelA.HPP.Z,3695] SUF Flags.
  I64 fuf_flags=0;
  CDirEntry *tempde,*tempde1;
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+f+F+T");
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  tempde=tempde1=FilesFind(files_find_mask,fuf_flags);
  while (tempde) {
    SUFile(tempde->full_name,suf_flags,indent_scale_factor);
    tempde=tempde->next;
  }
  DirTreeDel(tempde1);
}

public U0 S2T(U8 *files_find_mask,U8 *fu_flags=NULL)
{//Spaces to tabs.
//Use "Hard Space" (SHIFT-SPACE) for spaces
  //in string consts in your code.
  SU(files_find_mask,SUF_S2T|SUF_REM_TRAILING,fu_flags);
}
