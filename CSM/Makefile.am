bin_PROGRAMS = c-smile

c_smile_SOURCES = main.cpp c-smile-ni.h sal/async.cxx sal/console.cxx \
	sal/ptask.cxx sal/multfile.cxx sal/unifile.cxx sal/unisock.cxx \
	tool/cs_base64.cpp tool/cs_datetime.cpp tool/cs_lzf.cpp \
	tool/cs_parser.cpp tool/cs_regexp.cpp \
	tool/cs_string.cpp tool/cs_url.cpp \
	vm/archive.cpp vm/compiler.cpp vm/debug.cpp \
	vm/mm.cpp vm/rtl_array.cpp vm/rtl_blob.cpp vm/rtl.cpp \
	vm/rtl_date.cpp vm/rtl_file.cpp vm/rtl_map.cpp vm/rtl_regexp.cpp \
	vm/rtl_socket.cpp vm/rtl_stream.cpp vm/rtl_string.cpp vm/scanner.cpp \
	vm/streams.cpp vm/sym_table.cpp vm/things.cpp vm/threads.cpp \
	vm/cs_main.cpp vm/VM.cpp vm/vm_interpret.cpp \
	vm/c-smile-ni.cpp sal/async.h sal/console.h sal/file.h \
	sal/mmapfile.h sal/multfile.h sal/osfile.h sal/sockio.h \
	sal/stdtp.h sal/support.h sal/ptask.h sal/task.h sal/unisock.h \
	tool/tool.h tool/cs_array.h tool/cs_bag.h tool/cs_base64.h \
	tool/cs_basic.h tool/cs_datetime.h tool/cs_dictionary.h \
	tool/cs_hash.h tool/cs_hash_table.h tool/cs_list.h tool/cs_lzf.h \
	tool/cs_parser.h tool/cs_regexp.h tool/cs_string.h tool/cs_url.h \
	vm/arithmetic.h vm/compiler.h vm/mm.h vm/opcodes.h vm/rtl.h vm/scanner.h \
	vm/streams.h vm/sym_table.h vm/c-smile.h vm/vm.h

INCLUDES = -I$(srcdir)/tool -I$(srcdir)/sal -I$(srcdir)/vm
LIBS = -lpthread -ldl

DEFS = -ggdb3 -DCOMPILER -DPTHREADS
CXXFLAGS =
CFLAGS =
SUFFIXES = .cpp .cxx
%.o: %.cpp
	@echo '$(CXXCOMPILE) -c $< -o $@'; \
	$(CXXCOMPILE) -c $< -o $@
%.o: %.cxx
	@echo '$(CXXCOMPILE) -c $< -o $@'; \
	$(CXXCOMPILE) -c $< -o $@

