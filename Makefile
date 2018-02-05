MAKEFLAGS += -r

default : test.exe

%.ll : %.c
	clang -O3 -target x86_64-w64-windows -emit-llvm -S $*.c -o $@

test.ll : prng.h

%.bc : %.ll
	opt -O3 $^ -o $@

%.s : %.bc
	llc -O3 $^ -o $@

%.o : %.s
	llvm-mc -filetype=obj $^ -o $@

test.exe : test.o alloc.o
	../llvm/_build/bin/lld-link $^ /subsystem:console /entry:start /out:$@ /defaultlib:kernel32 /dynamicbase:no

.PHONY : default

.PRECIOUS : %.ll %.bc %.s %.o test.exe
