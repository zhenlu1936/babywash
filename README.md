## CH1

对 Brainfuck 程序地址数值与地址所存储的数值，从二进制原数值映射至 Gray Code，如 0010 => 0011、0011 => 0010。在这样的映射下，对于 Brainfuck 的各条指令，与 Brainwash 的指令集间有如下映射：

- `>` => `qyx`
- `<` => `qxy`
- `+` => `pzs`
- `-` => `psz`
- `.` =>  `o`
- `,` =>  `i`
- `[` =>  `[`
- `]` =>  `]`

其中，`mem[i]` = 255 时，`pzs` 将 `mem[i]` 赋值为 0。

因此，Brainfuck 指令集可由 Brainwash 指令集完全表达。而 Brainfuck 为图灵完备语言，因此 Brainwash 亦为图灵完备语言。

## CH2

### helloworld

样例代码：

```
[o[o]iooiiipzsipsziqxyqyxi[]oo]pzspzspzspzspzspzspzspzs[qyxpzspzspzspzs[qyxpzspzsqyxpzspzspzsqyxpzspzspzsqyxpzsqxyqxyqxyqxypsz]qyxpzsqyxpzsqyxpszqyxqyxpzs[qxy]qxypsz]qyxqyxoqyxpszpszpszopzspzspzspzspzspzspzsoopzspzspzsoqyxqyxoqxypszoqxyopzspzspzsopszpszpszpszpszpszopszpszpszpszpszpszpszpszoqyxqyxpzsoqyxpzspzso$
```

输出：

```
Hello World!
```

### echo

该样例会循环读取输入，并将输入的字符串原样输出。

样例代码：

```
pzs[io]$
```

样例输入：

```
1145141919810
```

样例输出：

```
1145141919810
```

### rot13

该样例会对输入中的大写字母 A-M 与 N-Z (vice versa)、小写字母 a-m 与 n-z (vice versa) 进行映射，并输出结果。

样例代码：

```
pszipzs[psz[qyxqyxpzspzspzspzs[qyxpzspzspzspzspzspzspzspzsqxypsz]qxypzsqxypsz[qyxpzsqyxpzsqyxpsz[qyxqyxqyx]qxy[[qyxpzsqxypsz]qyxqyxpzsqyx]qxyqxyqxyqxyqxypsz]]qyxqyxqyx[psz]pzsqyxpszpsz[psz[qxypszqyxpzspzspzs[psz]]]qxy[pzspzspzspzspzspzspzspzspzspzspzspzsqxy[qyxpsz[qyxpzsqyxqyx]qyx[pzs[qxypzsqyxpsz]qyxpzsqyxqyx]qxyqxyqxyqxyqxypsz]qyxqyx[qxypzsqyxpsz]qyx[psz[pszqxyqxy[psz]qyxqyx]qxyqxy[qxyqxypszqyxqyxpsz]qyxqyx]qxyqxy[qxyqxypzsqyxqyxpsz]]qxy[psz]qxyo[psz]qxypszipzs]$
```

输入：

```
abcdefg
opqrst
ABC
```

输出：

```
nopqrst
bcdefg
NOP
```

## Bonus Task

平台：Mac OS

架构：AArch64

### 顺序指令

对于各个顺序执行的普通指令，只需要按照字面翻译出对应的机器码即可。对 `countr_zero` 函数，可利用 Arm 架构的 `RBIT` 和 `CLZ` 实现。

### 分支结构

对于分支结构，在填充完两个分支的指令后，需要将相对地址填入分支前的跳转指令。

### 循环结构

对于循环结构，可以用栈结构保存各个循环起始的跳转指令地址，在处理循环末尾时，再计算跳转至末尾的相对地址，将其填入栈顶对应的跳转指令，并出栈。同时，计算末尾跳转至起始的机器码，并将其填入循环末尾的跳转指令。

### 输入/输出

为方便查看输入输出情况，需要将输入的 ASCII 字符二进制原数值转换为 Gray Code 值，并将输出的 Gray Code 值转换为二进制原数值。

### 函数调用

在调用 `putchar` 函数与 `getchar` 函数前后，需要保存内存基址、index 与返回地址，调用结束后恢复相关寄存器。

### JIT 执行

为了执行 JIT 指令，最终需要映射一块可读可执行的内存。Mac OS 平台无法一次性映射可读写可执行的内存，因此只能先用 `mmap` 获取一块可读写的内存，再用 `mprotect` 将这块内存映射为可读可执行的内存。









0

1

11
