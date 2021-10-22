; main 函数
define i32 @main() #0{
    %1 = alloca [10 x i32]
    %2 = getelementptr [10 x i32], [10 x i32]* %1, i32 0, i32 0
    store i32 10, i32* %2

    %3 = getelementptr [10 x i32], [10 x i32]* %1, i32 0, i32 0
    %4 = load i32, i32* %3
    %5 = mul i32 %4, 2
    %6 = getelementptr [10 x i32], [10 x i32]* %1, i32 0, i32 1
    store i32 %5, i32* %6

    %7 = load i32, i32* %6
    ret i32 %7
}
