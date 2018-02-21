MAKEFLAGS += -r
TARGET=x86_64-w64-windows
VCROOT=C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC
UCRT=C:\Program Files (x86)\Windows Kits\10\Lib\10.0.10240.0\ucrt\x64

default : test.exe testc.exe

%.ll : %.c
	clang -O3 -g -target ${TARGET} -emit-llvm -S $*.c -o $@

test.ll : prng.h

test.bc : test.ll
	sed -e 's/\(define .* @test[^!{]*\)/\1gc "statepoint-example" /' $^ | \
	sed -e 's/bitcast i16\*/addrspacecast i8 addrspace(1)*/g' | \
	sed -e 's/i16\*/i8 addrspace(1)*/g' | \
	sed -e 's/i16/i8/g' | \
	  opt -rewrite-statepoints-for-gc -o $@

gc.ll : rt.h align.h box.h gc.copy.h

%.bc : %.ll
	opt -O3 $^ -o $@

%.s : %.bc
	llc -O3 $^ -o $@

%.o : %.s
	llvm-mc -g -filetype=obj $^ -o $@

test.exe : gcbegin.o test.o alloc.bc utils.o gc.bc stackmap.bc
	../llvm/_build/bin/lld-link $^ /subsystem:console /entry:start /out:$@ \
		 /defaultlib:kernel32 \
		 /defaultlib:ucrt /libpath:'${UCRT}' \
		 /dynamicbase:no \
		 /pdb:test.pdb \
		 /merge:.llvm_stackmaps=.rdata

testc.exe : main.o test.o crt.c
	clang -O3 -g -target ${TARGET} $^ -o $@ \
	  -L'${UCRT}' \
	  -L'${VCROOT}\lib\amd64'

.PHONY : default

.PRECIOUS : %.ll %.bc %.s %.o test.exe
