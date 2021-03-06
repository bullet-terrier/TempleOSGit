F64 Clamp(F64 num,F64 lo,F64 hi)
{//Clamp to F64 [] range.
  if (num<lo)
    return lo;
  if (num>hi)
    return hi;
  return num;
}

F64 Min(F64 n1,F64 n2)
{//Min of two F64s.
  if (n1<=n2)
    return n1;
  else
    return n2;
}

F64 Max(F64 n1,F64 n2)
{//Max of two F64s.
  if (n1>=n2)
    return n1;
  else
    return n2;
}

F64 Pow10I64(I64 i)
{//F64 integer powers of ten.
  if (i>308)
    return inf;
  else if (i<-308)
    return 0.0;
  else
    return pow10_I64[i+309];
}

U64 FloorU64(U64 num,U64 to)
{//Integer multiples of num.
  return num-num%to;
}

U64 CeilU64(U64 num,U64 to)
{//Integer multiples of num.
  num+=to-1;
  return num-num%to;
}

I64 RoundI64(I64 num,I64 to)
{//Integer multiples of num.
  return num-num%to;
}

I64 FloorI64(I64 num,I64 to)
{//Integer multiples of num.
  if (num>=0)
    return num-num%to;
  else {
    num++;
    return num-num%to-to;
  }
}

I64 CeilI64(I64 num,I64 to)
{//Integer multiples of num.
  if (num>=0) {
    num+=to-1;
    return num-num%to;
  } else {
    num+=to-1;
    return num-num%to-to;
  }
}

//See [C:/Doc/Credits.TXT.Z,1].
I16 RandI16()
{//Random I16.
  I64 result=1664525*Fs->rand_seed+1013904223;
  if (!Bt(&Fs->task_flags,TASKf_NONTIMER_RAND))
    result^=GetTSC>>4;
  Fs->rand_seed=result;
  return result.i16[0];
}

U16 RandU16()
{//Random U16.
  I64 result=1664525*Fs->rand_seed+1013904223;
  if (!Bt(&Fs->task_flags,TASKf_NONTIMER_RAND))
    result^=GetTSC>>4;
  Fs->rand_seed=result;
  return result.u16[0];
}

I32 RandI32()
{//Random I32.
  I64 result=1664525*Fs->rand_seed+1013904223;
  if (!Bt(&Fs->task_flags,TASKf_NONTIMER_RAND))
    result^=GetTSC>>4;
  Fs->rand_seed=result;
  return result.i32[0];
}

U32 RandU32()
{//Random U32.
  I64 result=1664525*Fs->rand_seed+1013904223;
  if (!Bt(&Fs->task_flags,TASKf_NONTIMER_RAND))
    result^=GetTSC>>4;
  Fs->rand_seed=result;
  return result.u32[0];
}

I64 RandI64()
{//Random I64.
  I64 result=1664525*Fs->rand_seed+1013904223;
  if (!Bt(&Fs->task_flags,TASKf_NONTIMER_RAND))
    result^=GetTSC>>4;
  Fs->rand_seed=1664525*result+1013904223;
  return result<<32+Fs->rand_seed&0xFFFFFFFF;
}

U64 RandU64()
{//Random U64.
  I64 result=1664525*Fs->rand_seed+1013904223;
  if (!Bt(&Fs->task_flags,TASKf_NONTIMER_RAND))
    result^=GetTSC>>4;
  Fs->rand_seed=1664525*result+1013904223;
  return result<<32+Fs->rand_seed&0xFFFFFFFF;
}

F64 Rand()
{//Random F64.
  I64 result=RandI64&0x3FFFFFFFFFFFFFFF;
  return result/ToF64(0x4000000000000000);
}

I64 Seed(I64 seed=0,CTask *task=NULL)
{//Set [C:/Kernel/KMathB.CPP.Z,128] Rand() seed. Zero for timer-based.
  if (!task) task=Fs;
  if (seed) {
    LBts(&task->task_flags,TASKf_NONTIMER_RAND);
    return task->rand_seed=seed;
  } else {
    LBtr(&task->task_flags,TASKf_NONTIMER_RAND);
    return task->rand_seed=GetTSC>>4;
  }
}
