; ModuleID = 'ECTLang module'
source_filename = "ECTLang module"

@0 = private unnamed_addr constant [4 x i8] c"%f\0A\00", align 1
@1 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
@2 = private unnamed_addr constant [4 x i8] c"%f\0A\00", align 1

define void @main() {
body:
  %0 = call i32 (ptr, ...) @printf(ptr @0, double 0x4018CCCCD0000000)
  %1 = call i32 (ptr, ...) @printf(ptr @1, i32 3)
  %2 = call i32 (ptr, ...) @printf(ptr @2, double 0.000000e+00)
  ret void
}

declare i32 @printf(ptr, ...)
