target datalayout = "e-m:w-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-w64-windows-msvc19.0.23918"

%box = type { i32, i32, [0 x i64] }

define i8 addrspace(1)* @allocTree(i32 %nint, i32 %nnest) unnamed_addr gc "statepoint-example" {
  %tree.ref = tail call i8 addrspace(1)* @allocInts(i32 %nint, i32 %nnest)
  call void @printNode(i8 addrspace(1)* %tree.ref)
  %tree.box = bitcast i8 addrspace(1)* %tree.ref to %box addrspace(1)*
  %children.i64 = getelementptr inbounds %box, %box addrspace(1)* %tree.box, i64 0, i32 2, i32 %nint
  %children = bitcast i64 addrspace(1)* %children.i64 to i8 addrspace(1)* addrspace(1)*
  %nint.child = udiv i32 %nint, 2
  %nnest.child = udiv i32 %nnest, 3

  br label %header.zero
header.zero:
  %i.zero = phi i32 [ 0, %0], [%i.zero.next, %loop.zero]
  %done.zero = icmp uge i32 %i.zero, %nnest
  br i1 %done.zero, label %header, label %loop.zero
loop.zero:
  %child.p.zero = getelementptr i8 addrspace(1)*, i8 addrspace(1)* addrspace(1)* %children, i32 %i.zero
  store i8 addrspace(1)* zeroinitializer, i8 addrspace(1)* addrspace(1)* %child.p.zero
  %i.zero.next = add i32 %i.zero, 1
  br label %header.zero

header:
  %i = phi i32 [ 0, %header.zero], [%i.next, %loop]
  %done = icmp uge i32 %i, %nnest
  br i1 %done, label %exit, label %loop
loop:
  %child = tail call i8 addrspace(1)* @allocTree(i32 %nint.child, i32 %nnest.child)
  %child.p = getelementptr i8 addrspace(1)*, i8 addrspace(1)* addrspace(1)* %children, i32 %i
  tail call void @printChild(i8 addrspace(1)* addrspace(1)* %children, i32 %i, i32 %nnest, i8 addrspace(1)* %child)
  store i8 addrspace(1)* %child, i8 addrspace(1)* addrspace(1)* %child.p
  tail call void @verifyTree(i8 addrspace(1)* %tree.ref)
  %i.next = add i32 %i, 1
  br label %header
exit:
  ret i8 addrspace(1)* %tree.ref
}

declare void @verifyTree(i8 addrspace(1)*)
declare void @printNode(i8 addrspace(1)*) unnamed_addr
declare void @printChild(i8 addrspace(1)* addrspace(1)*, i32, i32, i8 addrspace(1)*) unnamed_addr

declare i8 addrspace(1)* @allocInts(i32, i32) local_unnamed_addr
