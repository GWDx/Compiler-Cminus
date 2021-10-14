define dso_local i32 @main() #0{
    %1 = alloca [10 x i32]
    %2 = getelementptr [10 x i32], [10 x i32]* %1, i32 0, i32 0
    store i32 10, i32* %2

    %3 = load i32, i32* %2
    %4 = mul i32 %3, 2
    %5 = getelementptr [10 x i32], [10 x i32]* %1, i32 0, i32 1
    store i32 %4, i32* %5

    %6 = load i32, i32* %5
    ret i32 %6
}
