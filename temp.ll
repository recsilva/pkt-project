; ModuleID = 'ECTLang module'
source_filename = "ECTLang module"

@0 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
@1 = private unnamed_addr constant [4 x i8] c"%f\0A\00", align 1
@2 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
@3 = private unnamed_addr constant [4 x i8] c"%f\0A\00", align 1
@4 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
@5 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
@6 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

define void @main() {
entry:
  %some_random_number = alloca i32, align 4
  %i = alloca i32, align 4
  %array = alloca [100 x i32], align 4
  %pi = alloca double, align 8
  br label %body

body:                                             ; preds = %entry
  %0 = call i32 (ptr, ...) @printf(ptr @0, i32 6)
  store double 0x400921FB60000000, ptr %pi, align 8
  store i32 0, ptr %i, align 4
  br label %loop.header

loop.header:                                      ; preds = %loop.body, %body
  %i.val = load i32, ptr %i, align 4
  %cmp = icmp slt i32 %i.val, 100
  br i1 %cmp, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.header
  %1 = load i32, ptr %i, align 4
  %elem.ptr = getelementptr inbounds [100 x i32], ptr %array, i32 0, i32 %1
  store i32 0, ptr %elem.ptr, align 4
  %i.inc = add i32 %1, 1
  store i32 %i.inc, ptr %i, align 4
  br label %loop.header

loop.end:                                         ; preds = %loop.header
  store i32 1, ptr %some_random_number, align 4
  %pi1 = load double, ptr %pi, align 8
  %2 = call i32 (ptr, ...) @printf(ptr @1, double %pi1)
  %some_random_number2 = load i32, ptr %some_random_number, align 4
  %3 = call i32 (ptr, ...) @printf(ptr @2, i32 %some_random_number2)
  %pi3 = load double, ptr %pi, align 8
  %some_random_number4 = load i32, ptr %some_random_number, align 4
  %4 = sitofp i32 %some_random_number4 to double
  %5 = fadd double %pi3, %4
  store double %5, ptr %pi, align 8
  %pi5 = load double, ptr %pi, align 8
  %6 = call i32 (ptr, ...) @printf(ptr @3, double %pi5)
  %elem.ptr6 = getelementptr inbounds [100 x i32], ptr %array, i32 0, i32 0
  %elem.val = load i32, ptr %elem.ptr6, align 4
  %7 = call i32 (ptr, ...) @printf(ptr @4, i32 %elem.val)
  %elem.ptr.assign = getelementptr inbounds [100 x i32], ptr %array, i32 0, i32 0
  store i32 100, ptr %elem.ptr.assign, align 4
  %elem.ptr.assign7 = getelementptr inbounds [100 x i32], ptr %array, i32 0, i32 5
  store i32 5, ptr %elem.ptr.assign7, align 4
  %elem.ptr8 = getelementptr inbounds [100 x i32], ptr %array, i32 0, i32 5
  %elem.val9 = load i32, ptr %elem.ptr8, align 4
  %8 = call i32 (ptr, ...) @printf(ptr @5, i32 %elem.val9)
  %elem.ptr10 = getelementptr inbounds [100 x i32], ptr %array, i32 0, i32 0
  %elem.val11 = load i32, ptr %elem.ptr10, align 4
  %9 = call i32 (ptr, ...) @printf(ptr @6, i32 %elem.val11)
  ret void
}

declare i32 @printf(ptr, ...)
