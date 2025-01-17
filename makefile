CC = g++ -std=c++11 -c -O3

OBJECTS = array.o error.o machine.o main.o memtab.o opcode.o reader.o std.o string.o struct.o symtab.o value.o main.o compiler.o scanner.o token.o statement.o expression.o parser.o printer.o scope.o

all: $(OBJECTS)
	g++ -o D:\Code\usr\neem.vm -O3 $(OBJECTS)

main.o: main.cpp
	$(CC) main.cpp

compiler.o: compiler.cpp compiler.h
	$(CC) compiler.cpp 

scanner.o: scanner.cpp scanner.h
	$(CC) scanner.cpp

token.o: token.cpp token.h
	$(CC) token.cpp

statement.o: statement.cpp statement.h symbol.h expression.h
	$(CC) statement.cpp

expression.o: expression.cpp expression.h symbol.h
	$(CC) expression.cpp

parser.o: parser.cpp parser.h
	$(CC) parser.cpp

printer.o: printer.cpp printer.h
	$(CC) printer.cpp

scope.o: scope.cpp scope.h
	$(CC) scope.cpp

array.o: array.cpp array.h
	$(CC) array.cpp

error.o: error.cpp error.h
	$(CC) error.cpp

machine.o: machine.cpp machine.h value.h struct.h symtab.h error.h string.h opcode.h reader.h machine.h
	$(CC) machine.cpp

main.o: main.cpp machine.h
	$(CC) main.cpp

memtab.o: memtab.cpp memtab.h
	$(CC) memtab.cpp

opcode.o: opcode.cpp opcode.h
	$(CC) opcode.cpp

reader.o: reader.cpp reader.h string.h struct.h machine.h memtab.h
	$(CC) reader.cpp

std.o: std.cpp std.h
	$(CC) std.cpp

string.o: string.cpp string.h
	$(CC) string.cpp

struct.o: struct.cpp struct.h value.h memtab.h
	$(CC) struct.cpp

symtab.o: symtab.cpp symtab.h
	$(CC) symtab.cpp

value.o: value.cpp value.h
	$(CC) value.cpp

clean:
	del *.o
