## <center>实验一	实验报告</center>

<center>PB19030801	甘文迪</center>



### 实验环境

- ubuntu 20.04.3
- flex 2.6.4
- bison 3.5.1
- llvm 10.0.0
- gcc 9.3.0



### 实验要求

- 阅读实验文档，学习使用 `flex` 进行词法分析和使用 `bison` 进行语法分析。
- 完成一个完整的 `Cminus-f` 解析器，包括基于 `flex` 的词法分析器和基于 `bison` 的语法分析器；需要完善 [lexical_analyzer.l](../../src/parser/lexical_analyzer.l) 和 [syntax_analyzer.y](../../src/parser/syntax_analyzer.y) ，要求通过所有测试。
- 书写实验报告。



### 实验难点

#### 正则表达式的书写和匹配

#### 书写

- 标识符与关键字应写成 `[a-zA-Z]+`

- 浮点数写成 `[0-9]+\.|[0-9]*\.[0-9]+ `

- 注释写成 `\/\*([^\*]|\*[^\/])*\*+\/`。

  > 其中括号中的内容 `[^\*]|\*[^\/]` 表示除 `*` 外的字符以及后一个字符不是 `/ ` 的 `*` 字符串。但是这不能解决字符串末尾的 `*/` 前有 `*` 的情况，所以末尾用 `\*+\/` 表示。

#### 匹配

flex 的正则表达式采取优先匹配最长字符串的原则，若长度相同则匹配顺序靠前的。因此，

- 应避免先写标识符再写关键字。本程序将标识符与关键字同时匹配，再在内部判断是否为关键字。

- `.` 应在最后进行匹配。



### 实验设计

#### 1. 阅读实验文档和代码，完成 `%union` 部分，通过编译

```c
void pass_node(char *text){
    yylval.node = new_syntax_tree_node(text);
}
```

根据这段代码，`%union` 部分应该定义 `node`，表示语法树的叶结点；其类型与 `new_syntax_tree_node` 返回值类型相同，为 `struct _syntax_tree_node *` 。



#### 2. 书写文法

在 [syntax_analyzer_raw.y](../../src/parser/syntax_analyzer_raw.y) 书写 31 条文法的解析规则，并使用 `%type` 定义相关的非终结符，使用 `%token` 定义相关的 token



#### 3. 书写正则表达式及执行动作

在 [lexical_analyzer.l](../../src/parser/lexical_analyzer.l) 中书写注释、单词、数字、符号、空白等字符的正则表达式
需要处理字符串位置 `lines` `pos_start` `pos_end`，使用 `pass_node` 函数存储 `yytext`，并返回相应的 token



#### 4. 编写处理步骤构建语法树

编写脚本 [makeGrammarTree.py](../../src/parser/makeGrammarTree.py)，它会在每条解析规则后增加形如 ` {$$ = node("declaration-list", 2, $1, $2);}` 的内容，以添加语法树的结点。

用其处理 [syntax_analyzer_raw.y](../../src/parser/syntax_analyzer_raw.y)，得到 [syntax_analyzer.y](../../src/parser/syntax_analyzer.y) 。



### 实验结果验证

通过了 `easy` `normal` `hard` 等难度的测试，下面是自行设计的测试

#### 其它字符

```c
void main(void) {
    int a;
    a = 3 % 2;
}
```

不通过。因为 `%` 不是 `Cminus-f` 中的符号。

#### 未定义的语法

```c
void main(void) {
    int a, b;
}
```

语法分析不通过。因为 `Cminus-f` 未规定这样的语法。

#### 测试注释

```c
int main(void) {
    int a;
    /*/*a=1*/*/
    return 0;
} 
```

词法分析显示第一个 `/*` 至第一个 `*/` 中的内容为注释，无法通过语法分析。

#### 测试 `if` 嵌套语句

```c
int main(void) {
    int a, b;
    a = b = 1;
    if (a == 0)
        if (b == 1)
            return 0;
        else
            return 1;
    return 2;
}
```

语法分析通过。

通过观察语法树，可以发现 `else` 对应的是后一个 `if` 。



### 实验反馈

#### 感想

在此次实验中，我学习了 flex 正则表达式、bison 的文法解析规则的书写，了解了两者如何进行交互。

此外，在尝试调试 bison 生成的文件时我了解了预处理命令 `#line` 以及 gdb 的使用，在遇到重复代码时尝试使用 python 脚本处理。

#### 建议

不建议在 [基础知识 Flex用法简介](../../Documentations/1-parser/Basics.md#Flex用法简介) 的代码中插入大量的注释



### 思考题

#### 1. 基础知识中的计算器例子的文法中存在左递归，为什么 `bison` 可以处理？

> 提示：不用研究 `bison` 内部运作机制，在下面知识介绍中有提到 `bison` 的一种属性，请结合课内知识思考

`bison` 使用归约，基于的是自底向上的语法分析，可以处理左递归的文法。



#### 2. 请在代码层面上简述下 `yylval` 是怎么完成协同工作的。

> 提示：无需研究原理，只分析维护了什么数据结构，该数据结构是怎么和`$1`、`$2`等联系起来？

以计算器为例，
维护了一个 `char` 和 `double` 的共用体类型 YYSTYPE，`yylval` 是一个此类型的变量。
flex 文件中需要指定 `yylval.op`, `yylval.num`
bison 文件中`%token` 定义接收的 token 的类型，`%type` 定义非终结符的类型，bison 会将它们所指代的类型自动作用于 `$$`, `$1` 等。



#### 3. 在计算器例子中，除 0 时会发生什么？如果把 `yylval` 修改为整形（`int`, `long` 等），这时候又会发生什么？

 `yylval.num` 的类型为 `double` 时

- 非 0 数除 0 时会打印 inf，可继续运行
- 0 除 0 时会打印 -nan，可继续运行

如果修改为 `int`，会显示除 0 异常—— `floating point exception (core dumped)`



#### 4. 能否修改计算器例子的文法，使得它支持除数0规避功能？

> 提示：这道题很难！尚未有同学给出正确答案。

首先，不能仅仅在词法部分改，因为它无法处理中间结果为 0 的情况，例如 `2/(1-1)`

不太理解什么是除数 0 规避。是说遇到 `2/0` 时返回整个字符串，还是报错呢？如果是前者，那遇到 `2/0*1` 又应该返回什么？
