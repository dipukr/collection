# SVM — the Saral Virtual Machine

A compact, stack-based virtual machine in C for the **Saral** object-oriented
language. SVM is written as a learning-oriented compiler target: your Saral
compiler emits `.svm` binary modules (or you hand-write `.sasm` assembly), and
SVM loads, links, and interprets them.

The design follows **JamVM 1.0.0** (Robert Lougher, 2003) closely — mark/sweep
GC, thin locks that inflate to heavyweight monitors, and POSIX-thread threading
with **signal-based stop-the-world suspension** — with a few deliberate,
documented changes for clarity and 64-bit correctness.

> Platform: **POSIX (Linux / WSL)**, gcc. Uses pthreads, POSIX signals, `mmap`.

---

## 1. Building & running

```bash
make            # builds ./svm, ./sasm, assembles lib/ and examples/
make run EX=Hello
./svm -cp lib:examples Poly
```

VM options: `-cp <path>`, `-verbose` (class linking), `-verbosegc`,
`-ms<size>`/`-mx<size>` (heap), `-ss<size>` (thread stack). Sizes accept
`k`/`m` suffixes.

The assembler:

```bash
./sasm lib/saral/lang/Object.sasm -o lib/saral/lang/Object.svm
```

---

## 2. How the differences from JamVM matter

| Area | JamVM | SVM |
|------|-------|-----|
| Slot width | 32-bit; `long`/`double` use **two** slots | **64-bit `Slot` union**; every value is **one** slot |
| Atomic CAS | per-arch asm (`cmpxchgl`, `lwarx`) | portable `__sync_bool_compare_and_swap` (`atomic.h`) |
| Macros | function-like macros everywhere | `static inline` functions; the only remaining macro is interpreter `DISPATCH` (unavoidable for computed goto) |
| Class library | GNU Classpath | tiny built-in `lib/saral/lang/*` |
| Native ABI | JNI + lightweight internal | one lightweight convention (`natives.c`) |
| Finalizers, async-GC thread | yes | omitted (core build) |

The 64-bit-slot decision is the big one: it makes `long`, `ulong`, and `num`
single-slot, so there is one load/store family and one arithmetic width, which
greatly simplifies the interpreter and is correct on x86-64 (JamVM's `u4`
slot assumption breaks on 64-bit pointers).

---

## 3. Source map

| File | Responsibility |
|------|----------------|
| `svm.h` | core types, `Slot`, structs, opcode enum, inline accessors, prototypes |
| `thread.h` / `atomic.h` / `heap.h` | thread & lock primitives, CAS, heap header bits |
| `hash.[ch]` | generic open-addressing hash table (callback-based) |
| `utf8.c` / `string.c` | modified-UTF-8, interning; `String` objects |
| `heap.c` | `mmap` heap, mark/sweep GC, allocation |
| `thread.c` | pthreads, thread list, `SIGUSR1` suspension, GC stack scan |
| `lock.c` | thin locks + monitor inflation, `wait`/`notify` |
| `class.c` | `.svm` loader, linking, init, constant-pool resolution, `isInstanceOf` |
| `excep.c` | exception raising, handler search, stack traces |
| `execute.c` | VM→Saral call boundary (frame setup, arg marshalling) |
| `interp.c` | the bytecode interpreter (threaded + switch fallback) |
| `natives.c` | built-in native methods |
| `main.c` | CLI, VM bootstrap |
| `sasm.c` | the textual assembler (`.sasm` → `.svm`) |

---

## 4. Object & value model

- **Slot** (`svm.h`): 64-bit union — `i` (signed), `u` (unsigned), `d`
  (double/`num`), `o` (object ref).
- **Object header**: `{ uintptr_t lock; Class *class; }`. Instance fields
  follow inline, one `Slot` each (`inst_data(ob)`).
- **Arrays**: `[ header | int length | packed elements ]`. Element storage is
  packed by natural size (1/2/4/8 bytes); the length is the first instance
  slot (`array_length`, `array_body`).
- **Class**: a `Class` object is layout-compatible with `Object`; its
  `ClassBlock` follows it (`class_cb`). Class objects are allocated by
  `allocClass` and their real state lives in the `ClassBlock`.

### Saral types → descriptors

| Type | Desc | Bytes (array) | Notes |
|------|------|---------------|-------|
| void | `V` | – | |
| bool | `Z` | 1 | |
| byte | `B` | 1 | signed 8 |
| ubyte | `H` | 1 | unsigned 8 |
| char | `C` | 2 | unsigned 16 |
| int | `I` | 4 | signed 32 |
| uint | `U` | 4 | unsigned 32 |
| long | `J` | 8 | signed 64 |
| ulong | `K` | 8 | unsigned 64 |
| num | `D` | 8 | IEEE double |
| ref | `L…;` / `[…` | 8 | object / array |

Signedness that the descriptor can't carry at runtime (e.g. `uint`) is handled
by the compiler inserting conversion opcodes (`i2u`, `u2d`, `udiv`, `iushr`, …).

---

## 5. Feature → mechanism map (Saral language)

| Saral feature | How SVM realises it |
|---------------|---------------------|
| `+ - * / %` (signed) | `iadd isub imul idiv irem` / `dadd…` for `num` |
| unsigned `/ %` `>>` | `udiv urem iushr` (compiler picks by static type) |
| `<< >> & \| ^ ~` | `ishl ishr iand ior ixor inot` |
| `= += -=` … | compiler expands to load/op/`store` (+`iinc` for int increments) |
| `== != < <= > >= ` | `if_icmp*`, `if*`, `lcmp/ulcmp/dcmpl/dcmpg` + branch |
| `&& \|\| !` `?:` | short-circuit / select compiled to branches |
| `if/else/while/for/foreach` | conditional branches + `goto` |
| `switch-case` | `tableswitch` (dense) / `lookupswitch` (sparse) |
| `break/return` | `goto` / `return`,`ireturn` |
| function overloading | resolved at compile time via signature; distinct `Methodref`s |
| encapsulation | `ACC_PRIVATE/PROTECTED/PUBLIC`; field access via `getfield/putfield` |
| polymorphism (virtual) | vtable (`method_table`) + `invokevirtual` |
| interfaces | `invokeinterface` + `isInstanceOf`/`implements` |
| generics (C++-style) | **monomorphised by the compiler**: one concrete class per instantiation, so the VM needs no generic metadata |
| `try/catch/finally` (single catch) | per-method exception table; `catch_type = 0` is the `finally`/catch-all handler; `athrow` + `findCatchBlock` unwinds |
| objects / `new` | `new` + `invokespecial <init>` |
| arrays | `newarray/anewarray/multianewarray`, `*aload`/`*astore` |
| threads | `saral/lang/Thread` (native `start` → `createSaralThread`) |
| synchronization | `monitorenter/exit`, `ACC_SYNCHRONIZED`, thin locks |
| garbage collection | stop-the-world mark & sweep (`heap.c`) |

---

## 6. Bytecode reference

All indices/branches are big-endian; branch offsets are signed 16-bit relative
to the **start** of the branch instruction. See the `enum Opcode` in `svm.h`
for the authoritative numeric values. Categories:

- **const/local**: `nop aconst_null iconst<i32> ldc<cp> load<idx> store<idx> iinc<idx,d>`
- **stack**: `pop dup dup_x1 swap`
- **int arith**: `iadd isub imul idiv irem udiv urem ineg`
- **num arith**: `dadd dsub dmul ddiv drem dneg`
- **bitwise**: `iand ior ixor inot ishl ishr iushr`
- **convert**: `i2d d2i u2d i2b i2ub i2c i2s i2i i2u`
- **compare**: `lcmp ulcmp dcmpl dcmpg`
- **branch**: `goto if{eq,ne,lt,ge,gt,le} if_icmp{…} if_acmp{eq,ne} ifnull ifnonnull`
- **switch**: `tableswitch lookupswitch`
- **return**: `return ireturn` (`ireturn` returns any one-slot value)
- **fields**: `getstatic putstatic getfield putfield`
- **invoke**: `invokevirtual invokespecial invokestatic invokeinterface`
- **alloc**: `new newarray anewarray multianewarray arraylength`
- **array**: `iaload/iastore` (8-byte: long/ulong/num/ref), `waload/wastore`
  (4-byte int/uint), `baload/bastore` (signed byte), `ubaload/ubastore`
  (unsigned byte), `caload/castore` (char), `saload/sastore` (signed 16)
- **checks/monitors/throw**: `checkcast instanceof athrow monitorenter monitorexit`

---

## 7. Assembler (`.sasm`) syntax

```
.class  <flags> <name>
.super  <name>                 ; omit for saral/lang/Object
.source <file>
.interface <name>              ; repeatable
.field  <flags> <name> <descriptor> [= int|long|num|str <value>]
.method <flags> <name> <descriptor>
    .stack  <n>
    .locals <n>
    .catch  <type|any> <fromLabel> <toLabel> <handlerLabel>
  label:
    <mnemonic> [operands]
.end method
.end class
```

Operand forms: `iconst 5`, `load 0`, `iinc 1 1`, `ldc int|long|num <v>`,
`ldc str "text"`, `goto L`, `getfield <class> <name> <desc>`,
`invokevirtual <class> <name> <desc>`, `new <class>`, `newarray int`,
`anewarray <class>`, `multianewarray <class> <dims>`, `checkcast <class>`.
Switches:

```
tableswitch 0
  L0
  L1
  default Ldef
.end switch

lookupswitch
  1 : Lone
  100 : Lhundred
  default Ldef
.end switch
```

See `examples/` for complete programs (Hello, Arith, Loop, TryCatch, Poly,
Threads) and `lib/saral/lang/` for the core class library.

---

## 8. `.svm` binary format

See the header comment in `class.c` for the byte-exact layout: magic
`0x5341524C` ("SARL"), version, constant pool, access/this/super, interfaces,
fields, methods (each with code, exception table, line-number table), and the
source-file index. The assembler (`sasm.c`) is the reference writer.

---

## 9. Deliberate limitations (extension points)

- No finalizers, no async/concurrent GC thread (collection is synchronous
  stop-the-world). Adding a finalizer list + finalizer thread mirrors JamVM's
  `alloc.c` closely.
- Monitors are never *deflated* (once inflated, stay inflated) — simplifies the
  lock word transitions; a real VM would deflate idle monitors.
- No bytecode verifier and no `<init>` chaining checks — trusted input assumed.
- No JIT (interpreter only), matching JamVM's original release.
- User-defined class loaders are not supported (bootstrap loader only).
- The assembler does not emit line-number tables (stack traces show
  `(unknown)` for line numbers).

These are intentional to keep the core small and readable; each is a good
exercise to extend.
