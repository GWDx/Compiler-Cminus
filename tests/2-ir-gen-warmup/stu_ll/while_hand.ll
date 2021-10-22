; main 函数
define i32 @main() #0{
    %1 = alloca i32
    %2 = alloca i32
    store i32 10, i32* %1
    store i32 0, i32* %2
    br label %3

; 循环判断部分
3:
    %4 = load i32, i32* %2
    %5 = icmp slt i32 %4, 10
    br i1 %5, label %6, label %12

; 循环体内部
6:
    %7 = load i32, i32* %2  ; 取出 i
    %8 = add i32 %7, 1
    store i32 %8, i32* %2

    %9 = load i32, i32* %1  ; 取出 a
    %10 = load i32, i32* %2
    %11 = add i32 %9, %10
    store i32 %11, i32* %1
    br label %3

; return 部分
12:
    %13 = load i32, i32* %1
    ret i32 %13
}
