define dso_local i32 @main() #0{
    %1 = alloca float
    store float 0x40163851E0000000, float* %1
    %2 = load float, float* %1
    %3 = fcmp ugt float %2, 0x3FF0000000000000
    br i1 %3, label %4, label %5

4:
    ret i32 233
5:
    ret i32 0
}
