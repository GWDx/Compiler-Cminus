## Lab5 实验报告

甘文迪	PB19030801

赛道一：后端代码的生成



### 实验环境

+ OS：Ubuntu 20.04.3 LTS x86_64
+ CPU：Intel i5-9400



### 实验内容

生成 `Light IR` 对应的 x86_64 汇编指令，用 `clang` 产生可执行文件。

> 支持所有类型的 IR 指令，不过在生成函数调用、数组的相关指令时可能存在问题



### 实验设计

#### 类

##### 位置

用 `Position` 表示位置，主要种类有 `Register`, `MemoryAddress`, `ConstInteger`

```c++
class Position {
public:
    string name;	// 位置的显示结果

    Position() {}
    Position(string name) { this->name = name; }
};
```

```c++
class Register : public Position
class MemoryAddress : public Position
class ConstInteger : public Position
```

##### 指令

```c++
class AsmInstruction {
public:
    string name;					// 指令名
    vector<Position> positions;		// 位置

    AsmInstruction(string name);	// 无位置指令
    AsmInstruction(string name, Position& p1);
    AsmInstruction(string name, Position& p1, Position& p2);
    
    string print();		// 打印语句
};
```

##### 汇编语句块

```c++
class AsmBlock {
public:
    string name;
    vector<AsmInstruction> normalInstructions;	// 普通语句
    vector<AsmInstruction> endInstructions;		// 结束语句（phi, ret, br）
    BasicBlock* basicBlock;						// 对应的 BasicBlock

    AsmBlock(BasicBlock* basicBlock) {
        this->basicBlock = basicBlock;
        name = basicBlock->get_name();
    }

    Position& getPosition(Value* value);
    void normalGenerate();	// 生成普通语句
    void endGenerate();		// 生成结束语句
    void stash();			// 存储寄存器

    void retInstGenerate(Instruction* instruction);		// 生成各种指令
    void binaryInstGenerate(Instruction* instruction);
    void cmpInstGenerate(Instruction* instruction);
    void fcmpInstGenerate(Instruction* instruction);
    void zextInstGenerate(Instruction* instruction);
    void fpToSiInstGenerate(Instruction* instruction);
    void siToFpInstGenerate(Instruction* instruction);
    void callInstGenerate(Instruction* instruction);
    void brInstGenerate(Instruction* instruction);
    void phiInstGenerate(Instruction* instruction);
    void loadInstGenerate(Instruction* instruction);
    void storeInstGenerate(Instruction* instruction);
    void allocaInstGenerate(Instruction* instruction);
    void gepInstGenerate(Instruction* instruction);

private:
    Value* tempInt = ConstantInt::get(0, module);	// 用于存储临时变量
    Value* tempFloat = ConstantFP::get(0, module);
};
```

##### 汇编函数

```c++
class AsmFunction {
public:
    vector<AsmBlock> allBlock;				// 汇编语句块
    vector<AsmInstruction> initInst;		// 初始指令
    vector<AsmInstruction> argMoveInst;		// 用于移动函数参数及全局变量
    vector<int> allConstInteger;			// 浮点数对应的整数
    vector<string> allConstLabel;			// 浮点数对应的位置
    Function* function;
    int functionEndNumber;			// 函数序号
    string functionName;

    AsmFunction(Function* function, int functionEndNumber);
    void generate();				// 生成指令
    string appendConst(int value);	// 标记浮点数
    string print();					// 打印
}
```

#### 其他函数

```c++
void updateRegister(Value* value);				// 使用 LRU 更新寄存器状态
MemoryAddress& getAddress(Value* value);		// 获取地址
Register& getEmptyRegister(Value* value);		// 获得空的寄存器
Position& AsmBlock::getPosition(Value* value);	// 寻找位置
```



### 实验难点

#### 函数调用及返回

参考 `clang -S` 得到的文件，参数优先进入寄存器 `edi, esi, edx, ecx, r8d, r9d`，否则压入栈

```c++
FOR (i, 1, operandNumber - 1) {
    Position& position = getPosition(operands[i]);
    auto type = operands[i]->get_type();
    if (type == int32Type) {
        if (intRegisterIndex < argIntRegister.size())
            appendInst(movl, position, *argIntRegister[intRegisterIndex++]);
        else
            appendInst(pushq, position);
    }
}
```

以 `call` 语句中的整数参数为例，对于每一个参数，若用于传参数的寄存器未满，则将此参数的值 `movel` 进寄存器，否则 `pushq` 进栈。



#### phi 语句的生成

直接在来自的基本块的末尾加指令存在问题。因为如果这个基本块在后方，其中还没有指令，会在最前面生成指令。

因此增加 `endInstructions`，在其最前面插入指令。

```c++
FOR (i, 0, operands.size() / 2 - 1) {
    auto value = operands[2 * i];
    auto label = operands[2 * i + 1];
    auto basicBlock = static_cast<BasicBlock*>(label);
    auto asmBlock = basicBlockToAsmBlock[basicBlock];
    auto& endInstructions = asmBlock->endInstructions;
    endInstructions.insert(endInstructions.begin(), AsmInstruction(movl, eax, getAddress(instruction)));
    endInstructions.insert(endInstructions.begin(), AsmInstruction(movl, getPosition(value), eax));		// 在 begin() 处插入需要倒过来写
}
```



#### 申请临时数组空间及获取数组元素的指针

对于数组的 `alloca`，先获取空间，再设置值 `instruction` 对应的地址

```c++
if (allocaType->get_type_id() == Type::ArrayTyID) {
    auto arrayType = static_cast<ArrayType*>(allocaType);
    int nums = arrayType->get_num_of_elements();
    stackSpace += nums * 4;
}
getAddress(instruction);
```

`getAddress` 对于临时数组会设置初值 `rbp + 8 - n`，其中 n 表示当前栈大小

```c++
if (value->get_type()->get_pointer_element_type()->get_type_id() == Type::ArrayTyID) {
	appendInst(leaq, MemoryAddress(8 - stackSpace, rbp), rax);
	appendInst(movq, rax, *valueToAddress[value]);
}
```

至于获取数组元素的指针，`ans = pointer + 4 * index`

```c++
auto& reg = getEmptyRegister(instruction);
appendInst(movq, getPosition(pointer), reg);
appendInst(movl, getPosition(index), eax);
appendInst(imull, ConstInteger(4), eax);
appendInst(addq, rax, reg);
```



#### 其他问题

+ 不仅应当在基本块前后保存所有寄存器的值，而且在函数调用前也要保存所有寄存器的值。
+ 全局变量的获取指针不应在首次在基本块中出现时设置，而应设置在函数的开始部分，因为基本块中的顺序并不是执行的顺序。



### 添加、修改的文件

代码部分

+ 在 `src/cminusfc` 目录下，修改了 `CMakeLists.txt` `cminusfc.cpp`
+ 在 `src/CodeGenerate` 目录下，新增 `Position.hpp` `CodeGenerate.cpp` `CMakeLists.txt` `AsmInstruction.hpp`
+ 新增 `include/CodeGenerate.hpp`
+ 修改 `src/CMakeLists.txt`

测试部分

+ 新增 `Current/eval/eval.py`
+ 新增 `init.sh`
+ 在 `Current/test` 目录下，新增 `eval.sh` `test.c` `test.sh`

其他

+ 更改 `.gitignore` 
+ 实验报告



### 运行及测试

#### 生成单个文件对应的汇编代码

```bash
./init.sh
cd Current/test
./test.sh
```

最后一行可改成

```assembly
../../build/cminusfc test.cminus -mem2reg -S -o ans
clang ans.s -o ans.out -L. -lcminus_io
./ans.out
```

结果见 [简单示例](#简单示例)



#### lab3 的测试文件测试

```bash
./init.sh
cd Current/eval
python3 eval.py
```

使用助教的 lab3 实现，可以通过 lab3 的测试文件



### 存在的问题

+ 某些样例无法通过
+ 产生了许多冗余的 `mov` 指令
+ 代码有些混乱



### 实验总结

1. 了解了 x86_64 汇编语言，学习使用 [edb](https://github.com/eteran/edb-debugger) 等调试工具。
2. 加深了对后端的理解。
2. 提高了编程能力。



### 简单示例

`test.cminus` 为

```c
int main(void){
	float x;
	x = 1 + 2.0;
	outputFloat(x);
	return 4;
}
```

生成的汇编为

```assembly
.main_0:
	.long	1073741824
.text
.globl main
main:
	.cfi_startproc
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$16, %rsp
.main_label_entry:
	movss	-4(%rbp), %xmm8
	movl	$1, %eax
	cvtsi2ssl	%eax, %xmm8
	movss	-8(%rbp), %xmm9
	movss	%xmm8, %xmm9
	addss	.main_0(%rip), %xmm9
	movss	%xmm9, %xmm0
	movss	%xmm8, -4(%rbp)
	movss	%xmm9, -8(%rbp)
	call	outputFloat
	movl	$4, %eax
	addq	$16, %rsp
	popq	%rbp
	retq
.Lfunc_end1:
	.cfi_endproc
```

执行后输出 `3.000000`，返回 4



### 备注

后续可能会在 https://github.com/GWDx/Compiler-Cminus 更新
