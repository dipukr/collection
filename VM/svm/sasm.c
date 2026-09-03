/*
 * sasm - the Saral assembler.
 *
 * Translates a textual .sasm file into the binary .svm module format read by
 * the SVM class loader (see class.c for the format spec).  It builds the
 * constant pool automatically from symbolic references.
 *
 * Syntax (one directive/instruction per line, ';' begins a comment):
 *
 *   .class  <flags> <name>
 *   .super  <name>                 ; omit for saral/lang/Object
 *   .source <filename>
 *   .interface <name>              ; repeatable
 *   .field  <flags> <name> <descriptor> [= <int|long|num|str> <value>]
 *   .method <flags> <name> <descriptor>
 *       .stack  <n>
 *       .locals <n>
 *       .catch  <type|any> <fromLabel> <toLabel> <handlerLabel>
 *     label:
 *       <mnemonic> [operands...]
 *   .end method
 *   .end class
 *
 * flags: public private protected static final synchronized native abstract
 *        interface
 *
 * Instruction operands:
 *   iconst <int>              load/store <localIdx>      iinc <idx> <delta>
 *   ldc int|long|num <value> | ldc str "text"
 *   goto/if* <label>
 *   getstatic/putstatic/getfield/putfield <class> <name> <desc>
 *   invokevirtual/special/static/interface <class> <name> <desc>
 *   new/anewarray/checkcast/instanceof <class>
 *   newarray <byte|bool|ubyte|char|int|uint|long|ulong|num>
 *   multianewarray <class> <dims>
 *   tableswitch <low> ... default <label> .end switch
 *   lookupswitch <match>:<label> ... default <label> .end switch
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef uint8_t  u1;
typedef uint16_t u2;
typedef uint32_t u4;
typedef uint64_t u8;

#define die(...) do { fprintf(stderr, "sasm: " __VA_ARGS__); exit(1); } while(0)

/* ------- opcode table (must match svm.h) -------------------------------- */

enum {
    OP_NOP,OP_ACONST_NULL,OP_ICONST,OP_LDC,OP_LOAD,OP_STORE,OP_IINC,OP_POP,
    OP_DUP,OP_DUP_X1,OP_SWAP,OP_IADD,OP_ISUB,OP_IMUL,OP_IDIV,OP_IREM,OP_UDIV,
    OP_UREM,OP_INEG,OP_DADD,OP_DSUB,OP_DMUL,OP_DDIV,OP_DREM,OP_DNEG,OP_IAND,
    OP_IOR,OP_IXOR,OP_INOT,OP_ISHL,OP_ISHR,OP_IUSHR,OP_I2D,OP_D2I,OP_U2D,
    OP_I2B,OP_I2UB,OP_I2C,OP_I2S,OP_I2I,OP_I2U,OP_LCMP,OP_ULCMP,OP_DCMPL,
    OP_DCMPG,OP_GOTO,OP_IFEQ,OP_IFNE,OP_IFLT,OP_IFGE,OP_IFGT,OP_IFLE,
    OP_IF_ICMPEQ,OP_IF_ICMPNE,OP_IF_ICMPLT,OP_IF_ICMPGE,OP_IF_ICMPGT,
    OP_IF_ICMPLE,OP_IF_ACMPEQ,OP_IF_ACMPNE,OP_IFNULL,OP_IFNONNULL,
    OP_TABLESWITCH,OP_LOOKUPSWITCH,OP_RETURN,OP_IRETURN,OP_GETSTATIC,
    OP_PUTSTATIC,OP_GETFIELD,OP_PUTFIELD,OP_INVOKEVIRTUAL,OP_INVOKESPECIAL,
    OP_INVOKESTATIC,OP_INVOKEINTERFACE,OP_NEW,OP_NEWARRAY,OP_ANEWARRAY,
    OP_ARRAYLENGTH,OP_MULTIANEWARRAY,OP_IALOAD,OP_IASTORE,OP_BALOAD,
    OP_BASTORE,OP_UBALOAD,OP_UBASTORE,OP_CALOAD,OP_CASTORE,OP_SALOAD,
    OP_SASTORE,OP_WALOAD,OP_WASTORE,OP_CHECKCAST,OP_INSTANCEOF,OP_ATHROW,
    OP_MONITORENTER,OP_MONITOREXIT
};

/* operand kinds */
enum { K_NONE, K_IMM4, K_LOCAL, K_BRANCH, K_LDC, K_FIELD, K_METHOD, K_CLASS,
       K_NEWARRAY, K_MULTI, K_IINC, K_TABLESWITCH, K_LOOKUPSWITCH };

typedef struct { const char *name; int opcode; int kind; } OpDef;

static OpDef ops[] = {
    {"nop",OP_NOP,K_NONE},{"aconst_null",OP_ACONST_NULL,K_NONE},
    {"iconst",OP_ICONST,K_IMM4},{"ldc",OP_LDC,K_LDC},
    {"load",OP_LOAD,K_LOCAL},{"store",OP_STORE,K_LOCAL},{"iinc",OP_IINC,K_IINC},
    {"pop",OP_POP,K_NONE},{"dup",OP_DUP,K_NONE},{"dup_x1",OP_DUP_X1,K_NONE},
    {"swap",OP_SWAP,K_NONE},
    {"iadd",OP_IADD,K_NONE},{"isub",OP_ISUB,K_NONE},{"imul",OP_IMUL,K_NONE},
    {"idiv",OP_IDIV,K_NONE},{"irem",OP_IREM,K_NONE},{"udiv",OP_UDIV,K_NONE},
    {"urem",OP_UREM,K_NONE},{"ineg",OP_INEG,K_NONE},
    {"dadd",OP_DADD,K_NONE},{"dsub",OP_DSUB,K_NONE},{"dmul",OP_DMUL,K_NONE},
    {"ddiv",OP_DDIV,K_NONE},{"drem",OP_DREM,K_NONE},{"dneg",OP_DNEG,K_NONE},
    {"iand",OP_IAND,K_NONE},{"ior",OP_IOR,K_NONE},{"ixor",OP_IXOR,K_NONE},
    {"inot",OP_INOT,K_NONE},{"ishl",OP_ISHL,K_NONE},{"ishr",OP_ISHR,K_NONE},
    {"iushr",OP_IUSHR,K_NONE},
    {"i2d",OP_I2D,K_NONE},{"d2i",OP_D2I,K_NONE},{"u2d",OP_U2D,K_NONE},
    {"i2b",OP_I2B,K_NONE},{"i2ub",OP_I2UB,K_NONE},{"i2c",OP_I2C,K_NONE},
    {"i2s",OP_I2S,K_NONE},{"i2i",OP_I2I,K_NONE},{"i2u",OP_I2U,K_NONE},
    {"lcmp",OP_LCMP,K_NONE},{"ulcmp",OP_ULCMP,K_NONE},
    {"dcmpl",OP_DCMPL,K_NONE},{"dcmpg",OP_DCMPG,K_NONE},
    {"goto",OP_GOTO,K_BRANCH},
    {"ifeq",OP_IFEQ,K_BRANCH},{"ifne",OP_IFNE,K_BRANCH},{"iflt",OP_IFLT,K_BRANCH},
    {"ifge",OP_IFGE,K_BRANCH},{"ifgt",OP_IFGT,K_BRANCH},{"ifle",OP_IFLE,K_BRANCH},
    {"if_icmpeq",OP_IF_ICMPEQ,K_BRANCH},{"if_icmpne",OP_IF_ICMPNE,K_BRANCH},
    {"if_icmplt",OP_IF_ICMPLT,K_BRANCH},{"if_icmpge",OP_IF_ICMPGE,K_BRANCH},
    {"if_icmpgt",OP_IF_ICMPGT,K_BRANCH},{"if_icmple",OP_IF_ICMPLE,K_BRANCH},
    {"if_acmpeq",OP_IF_ACMPEQ,K_BRANCH},{"if_acmpne",OP_IF_ACMPNE,K_BRANCH},
    {"ifnull",OP_IFNULL,K_BRANCH},{"ifnonnull",OP_IFNONNULL,K_BRANCH},
    {"tableswitch",OP_TABLESWITCH,K_TABLESWITCH},
    {"lookupswitch",OP_LOOKUPSWITCH,K_LOOKUPSWITCH},
    {"return",OP_RETURN,K_NONE},{"ireturn",OP_IRETURN,K_NONE},
    {"getstatic",OP_GETSTATIC,K_FIELD},{"putstatic",OP_PUTSTATIC,K_FIELD},
    {"getfield",OP_GETFIELD,K_FIELD},{"putfield",OP_PUTFIELD,K_FIELD},
    {"invokevirtual",OP_INVOKEVIRTUAL,K_METHOD},
    {"invokespecial",OP_INVOKESPECIAL,K_METHOD},
    {"invokestatic",OP_INVOKESTATIC,K_METHOD},
    {"invokeinterface",OP_INVOKEINTERFACE,K_METHOD},
    {"new",OP_NEW,K_CLASS},{"newarray",OP_NEWARRAY,K_NEWARRAY},
    {"anewarray",OP_ANEWARRAY,K_CLASS},{"arraylength",OP_ARRAYLENGTH,K_NONE},
    {"multianewarray",OP_MULTIANEWARRAY,K_MULTI},
    {"iaload",OP_IALOAD,K_NONE},{"iastore",OP_IASTORE,K_NONE},
    {"baload",OP_BALOAD,K_NONE},{"bastore",OP_BASTORE,K_NONE},
    {"ubaload",OP_UBALOAD,K_NONE},{"ubastore",OP_UBASTORE,K_NONE},
    {"caload",OP_CALOAD,K_NONE},{"castore",OP_CASTORE,K_NONE},
    {"saload",OP_SALOAD,K_NONE},{"sastore",OP_SASTORE,K_NONE},
    {"waload",OP_WALOAD,K_NONE},{"wastore",OP_WASTORE,K_NONE},
    {"checkcast",OP_CHECKCAST,K_CLASS},{"instanceof",OP_INSTANCEOF,K_CLASS},
    {"athrow",OP_ATHROW,K_NONE},
    {"monitorenter",OP_MONITORENTER,K_NONE},{"monitorexit",OP_MONITOREXIT,K_NONE},
    {NULL,0,0}
};

static OpDef *findOp(const char *name) {
    int i;
    for(i = 0; ops[i].name; i++)
        if(strcmp(ops[i].name, name) == 0) return &ops[i];
    return NULL;
}

/* ------- constant pool builder ------------------------------------------ */

enum { CP_Utf8=1, CP_Integer=3, CP_Long=5, CP_Double=6, CP_String=8,
       CP_Class=7, CP_Fieldref=9, CP_Methodref=10, CP_InterfaceMethodref=11,
       CP_NameAndType=12 };

typedef struct {
    u1  tag;
    char *s;          /* Utf8 */
    u2  a, b;         /* class/nt indices, or utf8 idx for string/class */
    int64_t i;        /* Integer/Long */
    double  d;        /* Double */
} CPEnt;

static CPEnt cp[8192];
static int cp_count = 1;   /* index 0 unused */

static int cpUtf8(const char *s) {
    int i;
    for(i = 1; i < cp_count; i++)
        if(cp[i].tag == CP_Utf8 && strcmp(cp[i].s, s) == 0) return i;
    cp[cp_count].tag = CP_Utf8;
    cp[cp_count].s = strdup(s);
    return cp_count++;
}
static int cpClass(const char *name) {
    int u = cpUtf8(name), i;
    for(i = 1; i < cp_count; i++)
        if(cp[i].tag == CP_Class && cp[i].a == u) return i;
    cp[cp_count].tag = CP_Class; cp[cp_count].a = u;
    return cp_count++;
}
static int cpNT(const char *name, const char *desc) {
    int n = cpUtf8(name), t = cpUtf8(desc), i;
    for(i = 1; i < cp_count; i++)
        if(cp[i].tag == CP_NameAndType && cp[i].a == n && cp[i].b == t) return i;
    cp[cp_count].tag = CP_NameAndType; cp[cp_count].a = n; cp[cp_count].b = t;
    return cp_count++;
}
static int cpRef(int tag, const char *cls, const char *nm, const char *desc) {
    int c = cpClass(cls), nt = cpNT(nm, desc), i;
    for(i = 1; i < cp_count; i++)
        if(cp[i].tag == tag && cp[i].a == c && cp[i].b == nt) return i;
    cp[cp_count].tag = tag; cp[cp_count].a = c; cp[cp_count].b = nt;
    return cp_count++;
}
static int cpString(const char *s) {
    int u = cpUtf8(s), i;
    for(i = 1; i < cp_count; i++)
        if(cp[i].tag == CP_String && cp[i].a == u) return i;
    cp[cp_count].tag = CP_String; cp[cp_count].a = u;
    return cp_count++;
}
static int cpInteger(int64_t v) {
    cp[cp_count].tag = CP_Integer; cp[cp_count].i = v;
    return cp_count++;
}
static int cpLong(int64_t v) {
    cp[cp_count].tag = CP_Long; cp[cp_count].i = v;
    return cp_count++;
}
static int cpDouble(double d) {
    cp[cp_count].tag = CP_Double; cp[cp_count].d = d;
    return cp_count++;
}

/* ------- output buffer -------------------------------------------------- */

static u1  out[1 << 20];
static int out_len = 0;
static void o1(u1 v)  { out[out_len++] = v; }
static void o2(u2 v)  { o1(v >> 8); o1(v & 0xff); }
static void o4(u4 v)  { o1(v>>24); o1(v>>16); o1(v>>8); o1(v); }
static void o8(u8 v)  { o4(v >> 32); o4(v & 0xffffffffu); }

/* ------- flags ---------------------------------------------------------- */

static int isFlagTok(const char *f) {
    return !strcmp(f,"public") || !strcmp(f,"private") || !strcmp(f,"protected")
        || !strcmp(f,"static") || !strcmp(f,"final") || !strcmp(f,"synchronized")
        || !strcmp(f,"native") || !strcmp(f,"interface") || !strcmp(f,"abstract");
}

static u2 parseFlag(const char *f) {
    if(!strcmp(f,"public"))       return 0x0001;
    if(!strcmp(f,"private"))      return 0x0002;
    if(!strcmp(f,"protected"))    return 0x0004;
    if(!strcmp(f,"static"))       return 0x0008;
    if(!strcmp(f,"final"))        return 0x0010;
    if(!strcmp(f,"synchronized")) return 0x0020;
    if(!strcmp(f,"native"))       return 0x0100;
    if(!strcmp(f,"interface"))    return 0x0200;
    if(!strcmp(f,"abstract"))     return 0x0400;
    die("unknown flag '%s'\n", f);
    return 0;
}

/* ------- tokenizer (quote-aware) ---------------------------------------- */

#define MAXTOK 64
static char tokbuf[MAXTOK][256];
static int  ntok;

static void tokenize(char *line) {
    char *p = line;
    ntok = 0;
    while(*p) {
        while(*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
        if(*p == '\0' || *p == ';') break;
        if(*p == '"') {
            char *d = tokbuf[ntok]; p++;
            while(*p && *p != '"') {
                if(*p == '\\' && p[1]) { p++;
                    *d++ = (*p=='n')?'\n':(*p=='t')?'\t':*p;
                } else *d++ = *p;
                p++;
            }
            if(*p == '"') p++;
            *d = '\0'; ntok++;
        } else {
            char *d = tokbuf[ntok];
            while(*p && *p!=' ' && *p!='\t' && *p!='\r' && *p!='\n' && *p!=';')
                *d++ = *p++;
            *d = '\0'; ntok++;
        }
        if(ntok >= MAXTOK) break;
    }
}

/* ------- parsed method representation ----------------------------------- */

typedef struct {
    int opcode, kind;
    int offset;             /* byte offset in code */
    /* operands (interpretation depends on kind) */
    int64_t imm;            /* iconst / iinc idx / local / ldc long / int */
    int64_t imm2;           /* iinc delta */
    double  dval;           /* ldc num */
    char    s1[256], s2[128], s3[64];  /* class / name / desc / label / ldc str */
    int     ldc_kind;       /* 0 int,1 long,2 num,3 str */
    /* switch */
    int     sw_low;
    int     sw_n;
    int64_t sw_match[256];
    char    sw_label[256][64];
    char    sw_default[64];
} Instr;

typedef struct { char name[64]; int offset; } Label;

typedef struct {
    u2 access; char name[128]; char desc[256];
    u2 max_stack, max_locals;
    Instr code[8192]; int ncode;
    Label labels[2048]; int nlabels;
    struct { char type[128]; char from[64], to[64], handler[64]; } etab[256];
    int netab;
} Method;

typedef struct { u2 access; char name[128]; char desc[64];
                 int cval_kind; int64_t cval_i; double cval_d; char cval_s[256]; } Field;

/* class-level state */
static u2   cls_access;
static char cls_name[256];
static char cls_super[256];
static char cls_source[256];
static char interfaces[64][256]; static int ninterfaces;
static Field  fields[512];  static int nfields;
static Method methods[512]; static int nmethods;

/* ------- helpers -------------------------------------------------------- */

static int labelOffset(Method *m, const char *name) {
    int i;
    for(i = 0; i < m->nlabels; i++)
        if(strcmp(m->labels[i].name, name) == 0) return m->labels[i].offset;
    die("undefined label '%s' in %s\n", name, m->name);
    return 0;
}

static int newarrayTag(const char *t) {
    if(!strcmp(t,"bool"))  return 1;
    if(!strcmp(t,"byte"))  return 2;
    if(!strcmp(t,"ubyte")) return 3;
    if(!strcmp(t,"char"))  return 4;
    if(!strcmp(t,"int"))   return 5;
    if(!strcmp(t,"uint"))  return 6;
    if(!strcmp(t,"long"))  return 7;
    if(!strcmp(t,"ulong")) return 8;
    if(!strcmp(t,"num"))   return 9;
    die("unknown array type '%s'\n", t);
    return 0;
}

/* size of one instruction given its start offset (for switch alignment) */
static int instrSize(Instr *in, int off) {
    switch(in->kind) {
        case K_NONE:   return 1;
        case K_IMM4:   return 5;
        case K_LOCAL:  return 3;
        case K_BRANCH: return 3;
        case K_LDC:    return 3;
        case K_FIELD:  return 3;
        case K_METHOD: return 3;
        case K_CLASS:  return 3;
        case K_NEWARRAY: return 2;
        case K_MULTI:  return 4;
        case K_IINC:   return 5;
        case K_TABLESWITCH: {
            int pad = ((off + 1 + 3) & ~3) - (off + 1);
            return 1 + pad + 12 + in->sw_n * 4;
        }
        case K_LOOKUPSWITCH: {
            int pad = ((off + 1 + 3) & ~3) - (off + 1);
            return 1 + pad + 8 + in->sw_n * 8;
        }
    }
    return 1;
}

/* ------- emit one method's code ----------------------------------------- */

static void emitMethod(Method *m) {
    int i, off = 0;

    /* pass 1: assign offsets, resolve labels already collected during parse
       (labels store the *instruction index*; convert to byte offset here) */
    for(i = 0; i < m->ncode; i++) { m->code[i].offset = off; off += instrSize(&m->code[i], off); }
    /* labels were stored with offset = index of following instruction */
    for(i = 0; i < m->nlabels; i++) {
        int idx = m->labels[i].offset;
        m->labels[i].offset = (idx < m->ncode) ? m->code[idx].offset : off;
    }

    /* method header */
    o2(m->access);
    o2(cpUtf8(m->name));
    o2(cpUtf8(m->desc));
    o2(m->max_stack);
    o2(m->max_locals);

    /* code length placeholder */
    {
        int lenpos = out_len; o4(0);
        int codestart = out_len;
        for(i = 0; i < m->ncode; i++) {
            Instr *in = &m->code[i];
            int here = out_len - codestart;
            switch(in->kind) {
                case K_NONE: o1(in->opcode); break;
                case K_IMM4: o1(in->opcode); o4((u4)(int32_t)in->imm); break;
                case K_LOCAL: o1(in->opcode); o2((u2)in->imm); break;
                case K_IINC: o1(in->opcode); o2((u2)in->imm); o2((u2)(int16_t)in->imm2); break;
                case K_BRANCH: {
                    o1(in->opcode);
                    o2((u2)(int16_t)(labelOffset(m, in->s1) - here));
                    break;
                }
                case K_LDC: {
                    int idx;
                    if(in->ldc_kind == 0) idx = cpInteger(in->imm);
                    else if(in->ldc_kind == 1) idx = cpLong(in->imm);
                    else if(in->ldc_kind == 2) idx = cpDouble(in->dval);
                    else idx = cpString(in->s1);
                    o1(in->opcode); o2(idx);
                    break;
                }
                case K_FIELD: o1(in->opcode);
                    o2(cpRef(CP_Fieldref, in->s1, in->s2, in->s3)); break;
                case K_METHOD: o1(in->opcode);
                    o2(cpRef(in->opcode==OP_INVOKEINTERFACE?CP_InterfaceMethodref:CP_Methodref,
                             in->s1, in->s2, in->s3)); break;
                case K_CLASS: o1(in->opcode); o2(cpClass(in->s1)); break;
                case K_NEWARRAY: o1(in->opcode); o1((u1)in->imm); break;
                case K_MULTI: o1(in->opcode); o2(cpClass(in->s1)); o1((u1)in->imm2); break;
                case K_TABLESWITCH: {
                    int pad = ((here + 1 + 3) & ~3) - (here + 1), j;
                    o1(in->opcode);
                    while(pad--) o1(0);
                    o4((u4)(int32_t)(labelOffset(m, in->sw_default) - here));
                    o4((u4)(int32_t)in->sw_low);
                    o4((u4)(int32_t)(in->sw_low + in->sw_n - 1));
                    for(j = 0; j < in->sw_n; j++)
                        o4((u4)(int32_t)(labelOffset(m, in->sw_label[j]) - here));
                    break;
                }
                case K_LOOKUPSWITCH: {
                    int pad = ((here + 1 + 3) & ~3) - (here + 1), j;
                    o1(in->opcode);
                    while(pad--) o1(0);
                    o4((u4)(int32_t)(labelOffset(m, in->sw_default) - here));
                    o4((u4)in->sw_n);
                    for(j = 0; j < in->sw_n; j++) {
                        o4((u4)(int32_t)in->sw_match[j]);
                        o4((u4)(int32_t)(labelOffset(m, in->sw_label[j]) - here));
                    }
                    break;
                }
            }
        }
        {
            int codelen = out_len - codestart;
            out[lenpos]   = codelen >> 24; out[lenpos+1] = codelen >> 16;
            out[lenpos+2] = codelen >> 8;  out[lenpos+3] = codelen;
        }
    }

    /* exception table */
    o2(m->netab);
    for(i = 0; i < m->netab; i++) {
        o2(labelOffset(m, m->etab[i].from));
        o2(labelOffset(m, m->etab[i].to));
        o2(labelOffset(m, m->etab[i].handler));
        o2(strcmp(m->etab[i].type,"any")==0 ? 0 : cpClass(m->etab[i].type));
    }
    /* line number table (none emitted by this simple assembler) */
    o2(0);
}

/* ------- parsing -------------------------------------------------------- */

static Method *cur_method = NULL;
static int in_switch = 0;
static Instr *sw_instr = NULL;

static void addLabel(Method *m, const char *name) {
    strcpy(m->labels[m->nlabels].name, name);
    m->labels[m->nlabels].offset = m->ncode;   /* index of next instr */
    m->nlabels++;
}

static void parseInstr(OpDef *op) {
    Instr *in = &cur_method->code[cur_method->ncode++];
    memset(in, 0, sizeof(*in));
    in->opcode = op->opcode;
    in->kind = op->kind;

    switch(op->kind) {
        case K_NONE: break;
        case K_IMM4:  in->imm = strtoll(tokbuf[1], NULL, 0); break;
        case K_LOCAL: in->imm = strtoll(tokbuf[1], NULL, 0); break;
        case K_IINC:  in->imm = strtoll(tokbuf[1], NULL, 0);
                      in->imm2 = strtoll(tokbuf[2], NULL, 0); break;
        case K_BRANCH: strcpy(in->s1, tokbuf[1]); break;
        case K_CLASS:  strcpy(in->s1, tokbuf[1]); break;
        case K_NEWARRAY: in->imm = newarrayTag(tokbuf[1]); break;
        case K_MULTI:  strcpy(in->s1, tokbuf[1]); in->imm2 = strtoll(tokbuf[2],NULL,0); break;
        case K_FIELD: case K_METHOD:
            strcpy(in->s1, tokbuf[1]); strcpy(in->s2, tokbuf[2]); strcpy(in->s3, tokbuf[3]); break;
        case K_LDC:
            if(!strcmp(tokbuf[1],"int"))       { in->ldc_kind=0; in->imm=strtoll(tokbuf[2],NULL,0); }
            else if(!strcmp(tokbuf[1],"long")) { in->ldc_kind=1; in->imm=strtoll(tokbuf[2],NULL,0); }
            else if(!strcmp(tokbuf[1],"num"))  { in->ldc_kind=2; in->dval=strtod(tokbuf[2],NULL); }
            else if(!strcmp(tokbuf[1],"str"))  { in->ldc_kind=3; strcpy(in->s1,tokbuf[2]); }
            else die("bad ldc operand '%s'\n", tokbuf[1]);
            break;
        case K_TABLESWITCH:
            in->sw_low = (int)strtoll(tokbuf[1], NULL, 0);
            in_switch = 1; sw_instr = in; in->sw_n = 0; break;
        case K_LOOKUPSWITCH:
            in_switch = 2; sw_instr = in; in->sw_n = 0; break;
    }
}

static void parseLine(char *line) {
    char *colon;
    tokenize(line);
    if(ntok == 0) return;

    /* inside a switch body */
    if(in_switch) {
        if(!strcmp(tokbuf[0], ".end") && ntok >= 2 && !strcmp(tokbuf[1],"switch")) {
            in_switch = 0; sw_instr = NULL; return;
        }
        if(!strcmp(tokbuf[0], "default")) { strcpy(sw_instr->sw_default, tokbuf[1]); return; }
        if(in_switch == 1) {                 /* tableswitch: label per line */
            strcpy(sw_instr->sw_label[sw_instr->sw_n++], tokbuf[0]);
        } else {                             /* lookupswitch: match:label */
            char *c = strchr(tokbuf[0], ':');
            if(c) { *c = '\0';
                sw_instr->sw_match[sw_instr->sw_n] = strtoll(tokbuf[0], NULL, 0);
                strcpy(sw_instr->sw_label[sw_instr->sw_n], c + 1);
                sw_instr->sw_n++;
            } else if(ntok >= 2) {
                sw_instr->sw_match[sw_instr->sw_n] = strtoll(tokbuf[0], NULL, 0);
                strcpy(sw_instr->sw_label[sw_instr->sw_n], tokbuf[1]);
                sw_instr->sw_n++;
            }
        }
        return;
    }

    /* directives */
    if(tokbuf[0][0] == '.') {
        if(!strcmp(tokbuf[0], ".class")) {
            int i; cls_access = 0;
            for(i = 1; i < ntok - 1; i++) cls_access |= parseFlag(tokbuf[i]);
            strcpy(cls_name, tokbuf[ntok - 1]);
        } else if(!strcmp(tokbuf[0], ".super")) {
            strcpy(cls_super, tokbuf[1]);
        } else if(!strcmp(tokbuf[0], ".source")) {
            strcpy(cls_source, tokbuf[1]);
        } else if(!strcmp(tokbuf[0], ".interface")) {
            strcpy(interfaces[ninterfaces++], tokbuf[1]);
        } else if(!strcmp(tokbuf[0], ".field")) {
            Field *f = &fields[nfields++];
            int i; memset(f, 0, sizeof(*f));
            /* accumulate leading flag tokens; stop at the first non-flag,
               which is the field name (this handles the optional "= k v"). */
            for(i = 1; i < ntok && isFlagTok(tokbuf[i]); i++)
                f->access |= parseFlag(tokbuf[i]);
            strcpy(f->name, tokbuf[i]); strcpy(f->desc, tokbuf[i+1]);
            if(i + 2 < ntok && !strcmp(tokbuf[i+2], "=")) {
                const char *k = tokbuf[i+3], *v = tokbuf[i+4];
                if(!strcmp(k,"int"))  { f->cval_kind=CP_Integer; f->cval_i=strtoll(v,NULL,0); }
                else if(!strcmp(k,"long")){ f->cval_kind=CP_Long; f->cval_i=strtoll(v,NULL,0); }
                else if(!strcmp(k,"num")) { f->cval_kind=CP_Double; f->cval_d=strtod(v,NULL); }
                else if(!strcmp(k,"str")) { f->cval_kind=CP_String; strcpy(f->cval_s,v); }
            }
        } else if(!strcmp(tokbuf[0], ".method")) {
            Method *m = &methods[nmethods++];
            int i; memset(m, 0, sizeof(*m));
            for(i = 1; i < ntok - 2; i++) m->access |= parseFlag(tokbuf[i]);
            strcpy(m->name, tokbuf[ntok - 2]);
            strcpy(m->desc, tokbuf[ntok - 1]);
            cur_method = m;
        } else if(!strcmp(tokbuf[0], ".stack")) {
            cur_method->max_stack = atoi(tokbuf[1]);
        } else if(!strcmp(tokbuf[0], ".locals")) {
            cur_method->max_locals = atoi(tokbuf[1]);
        } else if(!strcmp(tokbuf[0], ".catch")) {
            int n = cur_method->netab++;
            strcpy(cur_method->etab[n].type, tokbuf[1]);
            strcpy(cur_method->etab[n].from, tokbuf[2]);
            strcpy(cur_method->etab[n].to, tokbuf[3]);
            strcpy(cur_method->etab[n].handler, tokbuf[4]);
        } else if(!strcmp(tokbuf[0], ".end")) {
            if(!strcmp(tokbuf[1], "method")) cur_method = NULL;
            /* ".end class" -> nothing */
        } else {
            die("unknown directive '%s'\n", tokbuf[0]);
        }
        return;
    }

    /* label? token ending with ':' (may be alone or followed by an instr) */
    colon = strchr(tokbuf[0], ':');
    if(colon && colon[1] == '\0') {
        *colon = '\0';
        addLabel(cur_method, tokbuf[0]);
        if(ntok == 1) return;
        /* shift tokens left by one so the instruction parses normally */
        {
            int i;
            for(i = 0; i < ntok - 1; i++) strcpy(tokbuf[i], tokbuf[i+1]);
            ntok--;
        }
    }

    /* instruction */
    {
        OpDef *op = findOp(tokbuf[0]);
        if(op == NULL) die("unknown mnemonic '%s'\n", tokbuf[0]);
        parseInstr(op);
    }
}

/* ------- final assembly ------------------------------------------------- */

static void writeClass(const char *outpath) {
    int i;
    FILE *f;

    /* header */
    o4(0x5341524C);   /* "SARL" */
    o2(1);            /* version */

    /* We must lay the constant pool out first, but instructions add to it as
       they are emitted.  So: emit everything into a temporary body buffer
       AFTER the cp is fully built.  Strategy: emit method bodies first (which
       populates cp), buffering them, then write header+cp+bodies. To keep the
       code simple we instead pre-touch every symbolic reference by emitting
       the body into `out` after reserving the header, building cp as we go,
       then splicing.  Here we take the simplest correct approach: build the
       whole class body into a secondary buffer. */
    (void)i; (void)f;
}

/* We build the class body (after the cp) into `body`, populating cp; then we
 * serialise header + cp + body. */
static u1  body[1 << 20];
static int body_len = 0;
static void b1(u1 v){ body[body_len++]=v; }
static void b2(u2 v){ b1(v>>8); b1(v); }

int main(int argc, char *argv[]) {
    char inpath[512], outpath[512];
    FILE *in, *out_f;
    char line[4096];
    int i;

    if(argc < 2) die("usage: sasm <file.sasm> [-o out.svm]\n");
    strcpy(inpath, argv[1]);
    strcpy(outpath, "");
    for(i = 2; i < argc; i++)
        if(!strcmp(argv[i], "-o") && i+1 < argc) strcpy(outpath, argv[++i]);

    cls_super[0] = '\0';
    cls_source[0] = '\0';

    in = fopen(inpath, "r");
    if(!in) die("cannot open %s\n", inpath);
    while(fgets(line, sizeof(line), in)) parseLine(line);
    fclose(in);

    /* Phase A: emit all method bodies into `out` (this populates cp with all
     * bytecode-level references).  Capture each body's byte range. */
    {
        static int method_body_start[512], method_body_end[512];
        static int field_cval[512];
        int this_idx, super_idx, iface_idx[64], src_idx;
        static u1 image[1 << 21];
        int n = 0;

        out_len = 0;
        for(i = 0; i < nmethods; i++) {
            method_body_start[i] = out_len;
            emitMethod(&methods[i]);
            method_body_end[i] = out_len;
        }

        /* Phase A2: register EVERY remaining constant-pool reference now, so
         * the pool is final before we serialise it. */
        this_idx  = cpClass(cls_name);
        super_idx = cls_super[0] ? cpClass(cls_super) : 0;
        for(i = 0; i < ninterfaces; i++) iface_idx[i] = cpClass(interfaces[i]);
        for(i = 0; i < nfields; i++) {
            Field *fl = &fields[i];
            cpUtf8(fl->name); cpUtf8(fl->desc);
            if(fl->cval_kind == CP_Integer)      field_cval[i] = cpInteger(fl->cval_i);
            else if(fl->cval_kind == CP_Long)    field_cval[i] = cpLong(fl->cval_i);
            else if(fl->cval_kind == CP_Double)  field_cval[i] = cpDouble(fl->cval_d);
            else if(fl->cval_kind == CP_String)  field_cval[i] = cpString(fl->cval_s);
            else                                 field_cval[i] = 0;
        }
        src_idx = cls_source[0] ? cpUtf8(cls_source) : 0;

        /* Phase B: serialise the final image. */
        #define I1(v) image[n++] = (u1)(v)
        #define I2(v) do { u2 _t=(u2)(v); I1(_t>>8); I1(_t); } while(0)
        #define I4(v) do { u4 _t=(u4)(v); I1(_t>>24); I1(_t>>16); I1(_t>>8); I1(_t); } while(0)
        #define I8(v) do { u8 _t=(u8)(v); I4((u4)(_t>>32)); I4((u4)(_t&0xffffffffu)); } while(0)

        I4(0x5341524C); I2(1);

        I2(cp_count);
        for(i = 1; i < cp_count; i++) {
            CPEnt *e = &cp[i];
            I1(e->tag);
            switch(e->tag) {
                case CP_Utf8: { int l = strlen(e->s), k; I2(l);
                                for(k=0;k<l;k++) I1(e->s[k]); break; }
                case CP_Integer: I4((u4)(int32_t)e->i); break;
                case CP_Long:    I8((u8)e->i); break;
                case CP_Double:  { u8 bits; memcpy(&bits,&e->d,8); I8(bits); break; }
                case CP_String:  I2(e->a); break;
                case CP_Class:   I2(e->a); break;
                case CP_NameAndType: I2(e->a); I2(e->b); break;
                case CP_Fieldref: case CP_Methodref: case CP_InterfaceMethodref:
                    I2(e->a); I2(e->b); break;
            }
        }

        I2(cls_access);
        I2(this_idx);
        I2(super_idx);

        I2(ninterfaces);
        for(i = 0; i < ninterfaces; i++) I2(iface_idx[i]);

        I2(nfields);
        for(i = 0; i < nfields; i++)
            { I2(fields[i].access); I2(cpUtf8(fields[i].name));
              I2(cpUtf8(fields[i].desc)); I2(field_cval[i]); }

        I2(nmethods);
        for(i = 0; i < nmethods; i++) {
            int s = method_body_start[i], e = method_body_end[i], k;
            for(k = s; k < e; k++) I1(out[k]);
        }

        I2(src_idx);

        if(outpath[0] == '\0') {
            const char *base = cls_name;
            const char *slash = strrchr(cls_name, '/');
            if(slash) base = slash + 1;
            snprintf(outpath, sizeof(outpath), "%s.svm", base);
        }
        out_f = fopen(outpath, "wb");
        if(!out_f) die("cannot write %s\n", outpath);
        fwrite(image, 1, n, out_f);
        fclose(out_f);
        printf("wrote %s (%d bytes, cp=%d)\n", outpath, n, cp_count);
    }
    (void)writeClass; (void)body; (void)body_len; (void)b1; (void)b2;
    return 0;
}
