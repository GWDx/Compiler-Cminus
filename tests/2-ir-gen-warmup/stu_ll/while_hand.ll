define dso_local i32 @main() #0{
    %1 = alloca i32
    %2 = alloca i32
    store i32 10, i32* %1
    store i32 0, i32* %2
    br label %3
3:
    %4 = load i32, i32* %2
    %5 = icmp slt i32 %4, 10
    br i1 %5, label %6, label %11
6:
    %7 = load i32, i32* %2
    %8 = add i32 %7, 1
    store i32 %8, i32* %2

    %9 = load i32, i32* %1
    %10 = add i32 %9, %8
    store i32 %10, i32* %1
    br label %3
11:
    %12 = load i32, i32* %1
    ret i32 %12
}
