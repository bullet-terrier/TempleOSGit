#help_index "God"

#define GOD_BAD_BITS	4
#define GOD_GOOD_BITS	24

public class CGodGlbls
{
  U8	**words;
  I64	num_words;
  CFifoU8 *fifo;
  CDC	*doodle_dc;
  I64	doodle_ch;
  CFifoU8 *doodle_fifo;
  Bool	doodle_done;
} god;
MemSet(&god,0,sizeof(CGodGlbls));
god.doodle_fifo	=FifoU8New(32);
god.fifo	=FifoU8New(32);

extern U0 GodBitsIns(I64 num_bits,I64 n);
extern U0 GodWord();
extern U0 GodPassage(I64 num_lines=20);
extern U0 GodSong();
extern U0 GodDoodle();
