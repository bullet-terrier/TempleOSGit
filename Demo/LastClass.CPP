U0 StructName(U8 *d,U8 *class_name=lastclass)
{//lastclass is the previous fun arg's class as a string.
  "%X is a \"%s\".\n",d,class_name;
}

class Student
{
  Student *next;
  I64 age;
  U8 name[32];
} a;

class School
{
  U8 name[32];
  Student *students;
} s;

I64 i;

StructName(Fs);
StructName(&a);
StructName(&s);
StructName(&i);
PressAKey;

//lastclass is used in [C:/Adam/ADbg.CPP.Z,154] ClassRep() and [C:/Adam/ADbg.CPP.Z,179] ClassRepD().
ClassRepD(Gs);
PressAKey;

ClassRep(Fs);

//See also [C:/Adam/DolDoc/DocPopUp.CPP.Z,316] DocForm(), [C:/Demo/Dsk/BlkDevRep.CPP.Z,1] and [C:/Demo/ClassMeta.CPP.Z,1].
