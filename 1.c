#include <u.h>

enum {    // processor fault codes
    FKEYBD, // keyboard interrupt
    USER=16 // user mode exception
};

int in(port) {
    asm(LL,8);
    asm(BIN);
}
out() {
    asm(PSHA);
    asm(POPB);
    asm(LI, 1);
    asm(BOUT);
}

ivec(void *isr) {
    asm(LL,8);
    asm(IVEC);
}
stmr(int val) {
    asm(LL,8);
    asm(TIME);
}

halt(val) {
    asm(LL,8);
    asm(HALT);
}

trap(int c, int b, int a, int fc, int pc)
{
  switch (fc) {
  case FKEYBD:
    in(0);
    out();

    break;
  default: break;
  }
}
alltraps() {
  asm(PSHA);
  asm(PSHB);
  asm(PSHC);

  trap();

  asm(POPC);
  asm(POPB);
  asm(POPA);
  asm(RTI);
}

main()
{
  ivec(alltraps);
  asm(STI);
  while(1) {
    asm(NOP);
  }
  halt(0);
}

