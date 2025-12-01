; ModuleID = 'ECTLang module'
source_filename = "ECTLang module"

@0 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
@1 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
@2 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
@3 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

define void @main() {
entry:
  %d = alloca i32, align 4
  %c = alloca i32, align 4
  %b = alloca i32, align 4
  %a = alloca i32, align 4
  br label %body

body:                                             ; preds = %entry
  store i32 5, ptr %a, align 4
  store i32 4, ptr %b, align 4
  store i32 10, ptr %c, align 4
  %a1 = load i32, ptr %a, align 4
  %0 = call i32 (ptr, ...) @printf(ptr @0, i32 %a1)
  %b2 = load i32, ptr %b, align 4
  %1 = call i32 (ptr, ...) @printf(ptr @1, i32 %b2)
  %c3 = load i32, ptr %c, align 4
  %2 = call i32 (ptr, ...) @printf(ptr @2, i32 %c3)
  %a4 = load i32, ptr %a, align 4
  %b5 = load i32, ptr %b, align 4
  %3 = add i32 %a4, %b5
  %c6 = load i32, ptr %c, align 4
  %4 = sub i32 %c6, 2
  %5 = mul i32 %3, %4
  %6 = add i32 %5, 10
  store i32 %6, ptr %d, align 4
  %d7 = load i32, ptr %d, align 4
  %7 = call i32 (ptr, ...) @printf(ptr @3, i32 %d7)
  ret void
}

declare i32 @printf(ptr, ...)
