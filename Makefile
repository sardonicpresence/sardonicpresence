MAKEFLAGS += -r
TARGET=x86_64-w64-windows
VCROOT=C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC
UCRT=C:\Program Files (x86)\Windows Kits\10\Lib\10.0.10240.0\ucrt\x64

default : test.exe testc.exe

%.ll : %.c
	clang -O3 -g -target ${TARGET} -emit-llvm -S $*.c -o $@

test.ll : prng.h

gc.ll : rt.h align.h box.h gc.copy.h

test.gc.ll : test.ll
	sed -e 's/\(define .* @test[^!{]*\)/\1gc "statepoint-example" /' $^ | \
	sed -e 's/getelementptr inbounds i16, i16\* \([^,]*\), i64 2/getelementptr inbounds i8, i8 addrspace(1)* \1, i64 4/g' | \
	sed -e 's/getelementptr inbounds i16, i16\* \([^,]*\), i64 4/getelementptr inbounds i8, i8 addrspace(1)* \1, i64 8/g' | \
	sed -e 's/bitcast i16\*/addrspacecast i8 addrspace(1)*/g' | \
	sed -e 's/i16\*/i8 addrspace(1)*/g' | \
	sed -e 's/i16/i8/g' > $@

rt.ll : alloc.ll gc.ll stackmap.ll
	llvm-link $^ | llvm-link - -internalize -internalize-public-api-list=allocInts,freeInts,report -only-needed -S -o $@

app.ll : test.gc.ll allocTree.ll
	llvm-link $^ -S -o $@

%.bc : %.ll
	opt -O3 -rewrite-statepoints-for-gc $^ -o $@

%.s : %.bc
	../llvm/_build/bin/llc -O3 $^ -o - | sed -e 's/@\([0-9]\+\)/\1/g' > $@

%.o : %.s
	llvm-mc -g -filetype=obj $^ -o $@

test.exe : gcbegin.o app.o rt.o utils.o
	../llvm/_build/bin/lld-link $^ /subsystem:console /entry:start /out:$@ \
		 /defaultlib:kernel32 \
		 /defaultlib:ucrt /libpath:'${UCRT}' \
		 /dynamicbase:no \
		 /pdb:test.pdb \
		 /merge:.llvm_stackmaps=.rdata

testc.exe : main.o test.o allocTree.o crt.c
	clang -O3 -g -target ${TARGET} $^ -o $@ \
	  -L'${UCRT}' \
	  -L'${VCROOT}\lib\amd64'

.PHONY : default

.PRECIOUS : %.ll %.bc %.s %.o
