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

void *memcpy() { asm(LL,8); asm(LBL, 16); asm(LCL,24); asm(MCPY); asm(LL,8); }
void *memset() { asm(LL,8); asm(LBLB,16); asm(LCL,24); asm(MSET); asm(LL,8); }
void *memchr() { asm(LL,8); asm(LBLB,16); asm(LCL,24); asm(MCHR); }

write(fd, char *p, n) { while (n--) out(fd, *p++); }

int strlen(char *s) { return memchr(s, 0, -1) - (void *)s; }

enum { BUFSIZ = 32 };
int vsprintf(char *s, char *f, va_list v)
{
  char *e = s, *p, c, fill, b[BUFSIZ];
  int i, left, fmax, fmin, sign;

  while (c = *f++) {
    if (c != '%') { *e++ = c; continue; }
    if (*f == '%') { *e++ = *f++; continue; }
    if (left = (*f == '-')) f++;
    fill = (*f == '0') ? *f++ : ' ';
    fmin = sign = 0; fmax = BUFSIZ;
    if (*f == '*') { fmin = va_arg(v,int); f++; } else while ('0' <= *f && *f <= '9') fmin = fmin * 10 + *f++ - '0';
    if (*f == '.') { if (*++f == '*') { fmax = va_arg(v,int); f++; } else for (fmax = 0; '0' <= *f && *f <= '9'; fmax = fmax * 10 + *f++ - '0'); }
    if (*f == 'l') f++;
    switch (c = *f++) {
    case 0: *e++ = '%'; *e = 0; return e - s;
    case 'c': fill = ' '; i = (*(p = b) = va_arg(v,int)) ? 1 : 0; break;
    case 's': fill = ' '; if (!(p = va_arg(v,char *))) p = "(null)"; if ((i = strlen(p)) > fmax) i = fmax; break;
    case 'u': i = va_arg(v,int); goto c1;
    case 'd': if ((i = va_arg(v,int)) < 0) { sign = 1; i = -i; } c1: p = b + BUFSIZ-1; do { *--p = ((uint)i % 10) + '0'; } while (i = (uint)i / 10); i = (b + BUFSIZ-1) - p; break;
    case 'o': i = va_arg(v,int); p = b + BUFSIZ-1; do { *--p = (i & 7) + '0'; } while (i = (uint)i >> 3); i = (b + BUFSIZ-1) - p; break;
    case 'p': fill = '0'; fmin = 8; c = 'x';
    case 'x': case 'X': c -= 33; i = va_arg(v,int); p = b + BUFSIZ-1; do { *--p = (i & 15) + ((i & 15) > 9 ? c : '0'); } while (i = (uint)i >> 4); i = (b + BUFSIZ-1) - p; break;
    default: *e++ = c; continue;
    }
    fmin -= i + sign;
    if (sign && fill == '0') *e++ = '-';
    if (!left && fmin > 0) { memset(e, fill, fmin); e += fmin; }
    if (sign && fill == ' ') *e++ = '-';
    memcpy(e, p, i); e += i;
    if (left && fmin > 0) { memset(e, fill, fmin); e += fmin; }
  }
  *e = 0;
  return e - s;
}

int printf(char *f) { static char buf[4096]; return write(1, buf, vsprintf(buf, f, &f)); } // XXX remove static from buf

int expFlag;
int count;

alltraps()
{
  asm(PSHA);
  asm(PSHB);
  asm(PSHC);
  expFlag = 0;
  asm(POPC);
  asm(POPB);
  asm(POPA);
  asm(RTI);
}

main()
{
  ivec(alltraps);
  asm(STI);

  count = 0;
  expFlag = 1;

  while (expFlag) {
    count = count + 1;
    int val  = *(int*)(count);
  }
  printf("%d\n", count/1024/1024);
  halt(0);
}
