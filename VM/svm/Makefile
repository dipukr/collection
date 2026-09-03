# SVM - the Saral Virtual Machine
# Build under Linux / WSL with gcc.

CC      = gcc
CFLAGS  = -O2 -g -Wall -Wno-unused-label -DTHREADED
LDFLAGS = -lpthread -lm

VM_OBJS = main.o heap.o thread.o lock.o class.o excep.o \
          interp.o execute.o natives.o string.o utf8.o hash.o

all: svm sasm corelib examples

svm: $(VM_OBJS)
	$(CC) $(CFLAGS) -o $@ $(VM_OBJS) $(LDFLAGS)

sasm: sasm.o
	$(CC) $(CFLAGS) -o $@ sasm.o

%.o: %.c svm.h
	$(CC) $(CFLAGS) -c -o $@ $<

# assemble the core class library and examples (recursively)
CORE     = $(patsubst %.sasm,%.svm,$(shell find lib -name '*.sasm'))
EXAMPLES = $(patsubst %.sasm,%.svm,$(shell find examples -name '*.sasm'))

corelib: sasm $(CORE)

examples: sasm $(EXAMPLES)

lib/%.svm: lib/%.sasm sasm
	./sasm $< -o $@

examples/%.svm: examples/%.sasm sasm
	./sasm $< -o $@

# convenience: run an example  (make run EX=Hello)
run: all examples
	./svm -cp lib:examples $(EX)

clean:
	rm -f *.o svm sasm $(CORE) $(EXAMPLES)

.PHONY: all corelib examples run clean
