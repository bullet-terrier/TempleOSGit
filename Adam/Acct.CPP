#help_index "Misc/Accts"
#define ACCT_REGISTRY_FILENAME "::/Home/Registry.CPP.Z"
CDoc *sys_registry_doc=NULL;
I64 sys_msg_flags[1]={0};
F64 registry_version;

Bool AcctCacheRegistry()
{
  Bool old_silent;
  if (!sys_registry_doc) {
    old_silent=Silent;
    sys_registry_doc=DocRead(ACCT_REGISTRY_FILENAME);
    Silent(old_silent);
    return FALSE;
  } else
    return TRUE;
}

public Bool AcctRegSetDftEntry(U8 *path,U8 *val,Bool is_adam_entry=FALSE)
{//Add code doc tree branch to registry.
  Bool result,unlock_doc;
  AcctCacheRegistry;
  unlock_doc=DocLock(sys_registry_doc);
  if (!DocTreeFind(sys_registry_doc,path)) {
    DocTreeMake(sys_registry_doc,path);
    DocPrint(sys_registry_doc,"%s",val);
    if (is_adam_entry) {
      if (Fs==adam_task)
	ExePrint("%s",val);
      else
	Adam("%s",val);
    }
    if (IsDrvWritable(':'))
      DocWrite(sys_registry_doc);
    result=FALSE;
  } else
    result=TRUE;
  if (unlock_doc)
    DocUnlock(sys_registry_doc);
  return result;
}

public I64 AcctRegExeBranch(U8 *path)
{//Execute doc tree branch in registry.
  AcctCacheRegistry;
  return DocTreeBranchExe(sys_registry_doc,path);
}

public Bool AcctRegWriteBranch(U8 *path,U8 *fmt,...)
{//Rewrite doc tree branch in registry.
  Bool result,unlock_doc;
  CDocEntry *tree_branch,*start_indent,*end_indent;
  U8 *buf=StrPrintJoin(NULL,fmt,argc,argv);
  AcctCacheRegistry;
  unlock_doc=DocLock(sys_registry_doc);
  if (result=DocTreeFind(sys_registry_doc,path,&tree_branch,&start_indent,&end_indent))
    DocCut(sys_registry_doc,tree_branch,end_indent);
  DocTreeMake(sys_registry_doc,path);
  DocPrint(sys_registry_doc,"%s",buf);
  if (IsDrvWritable(':'))
    DocWrite(sys_registry_doc);
  if (unlock_doc)
    DocUnlock(sys_registry_doc);
  Free(buf);
  return result;
}
 
public Bool OneTimePopUp(U8 *_flags,I64 flag_num,U8 *msg)
{//See [C:/Apps/X-Caliber/X-Caliber.CPP.Z,1].
  Bool result=FALSE;
  CDoc *doc=DocNew;
  CDocEntry *doc_e;
  if (!Bt(_flags,flag_num)) {
    if (msg) DocPrint(doc,"%s",msg);
    doc_e=DocPrint(doc,"\n$CB,\"Do not show this msg again.\",LE=1$");
    DocPrint(doc,"$CM+CX,0,4$$BT,\"OKAY\",LE=1$\n");
    if (PopUpMenu(doc)==1 && doc_e->de_flags&DOCEF_CHECKED_COLLAPSED) {
      LBts(_flags,flag_num);
      result=TRUE;
    }
    DocDel(doc);
  }
  return result;
}

U0 AcctOneTimePopUp(I64 flag_num,U8 *msg)
{//You're not supposed to make system pop-up flags, only me.
  if (OneTimePopUp(sys_msg_flags,flag_num,msg))
    AcctRegWriteBranch("Adam/SysMsgFlags","sys_msg_flags[0]=0x%X;\n",
	sys_msg_flags[0]);
}

U0 RegistryInit()
{
  U8 buf[136];
  Bool version_present;
  AcctRegSetDftEntry("Adam/SysMsgFlags","sys_msg_flags[0]=0;\n",TRUE);
  StrPrint(buf,"registry_version=%4.3f;\n",os_version);
  version_present=AcctRegSetDftEntry("Adam/SysRegVer",buf,TRUE);
  AcctRegExeBranch("Adam");
  if (registry_version!=os_version) {
    AcctRegWriteBranch("Adam/SysRegVer",buf);
    AcctRegExeBranch("Adam");
  }
}
