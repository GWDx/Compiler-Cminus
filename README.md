## Compiler-Cminus

2021 年秋季编译原理实验  
实现 cminus-f 编译器

> cminus-f 是 C 语言的子集，语法与语义规则见 [cminusf.md](./Documentations/common/cminusf.md)

### 实验内容

| 编号   | 内容        | 描述                          | 实验任务                                       | 实验报告                                                                                      |
|:----:|:---------:|:---------------------------:|:------------------------------------------:|:-----------------------------------------------------------------------------------------:|
| lab1 | 词法分析、语法分析 | 写 cminus-f 的词法、语法规则         | [task1](./Documentations/1-parser/)        | [report1](./Reports/1-parser/README.md)                                                   |
| lab2 | 了解中间代码    | 学习使用用 `Light IR` 的接口        | [task2](./Documentations/2-ir-gen-warmup/) | [report2](./Reports/2-ir-gen-warmup/report.md)                                            |
| lab3 | 中间代码生成    | 根据抽象语法树生成中间代码               | [task3](./Documentations/3-ir-gen/)        | [report3](./Reports/3-ir-gen/report.md)                                                   |
| lab4 | 中间代码优化    | 实现常量传播、循环不变式外提、活跃变量分析等 Pass | [task4](./Documentations/4-ir-opt)         | [report4](./Reports/4-ir-opt/report-phase2.md)                                            |
| lab5 | 后端代码生成    | 由中间代码生成汇编代码                 | [task5](./Documentations/5-bonus/)         | [report5](./Reports/5-bonus/report.md)<br/>[slides5](./Reports/5-bonus/slides/slides.pdf) |

> 实验 3, 4 与彭炫超共同完成

### 编译及运行

#### 环境配置

```bash
sudo apt install llvm clang flex bison
```

#### 编译

```bash
./init.sh
```

#### 测试

```bash
cd Current/test
./test.sh
```
