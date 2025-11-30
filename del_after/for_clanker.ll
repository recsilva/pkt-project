; ModuleID = 'ECTLang module'
source_filename = "ECTLang module"

@0 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
@1 = private unnamed_addr constant [4 x i8] c"%f\0A\00", align 1
@2 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
@3 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

define void @main() {
body:
  %0 = call i32 (ptr, ...) @printf(ptr @0, i32 7)
  %1 = call i32 (ptr, ...) @printf(ptr @1, double 0xC132D837FBCCD339)
  %2 = call i32 (ptr, ...) @printf(ptr @2, i32 409476)
  %3 = call i32 (ptr, ...) @printf(ptr @3, i32 poison)
  ret void
}

declare i32 @printf(ptr, ...)
