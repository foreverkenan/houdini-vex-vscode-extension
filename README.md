# Houdini VEX for VS Code

改进不在main分支，在auto_vexzip分支

fork改进：从houdini安装目录的vex.zip加载vex函数声明的提示

作为HDK扩展的vex函数，只需要维护hdk的dll和vex.zip即可

vex.zip内的txt必须保证函数名为txt的文件名，同时函数声明必须以:usage:开头，使用`把声明括起来

其他内容暂时可有可无

如vex.zip/functions/abs.txt

```txt
= abs =

#type: vex
#context: all
#tags: math, number
#group: math

"""Returns the absolute value of the argument."""

:usage:`int abs(int n)`
:usage:`float abs(float n)`
:usage:`<vector> abs(<vector> v)`

    Returns the absolute (positive) equivalent of the number. For vectors, this is done per-component.


@examples

:box:Scalar example
    {{{
    #!vex
    if (abs(n) > 1) {
        // n is greater than 1 or less than -1
    }
    }}}

:box:Vector example
    {{{
    #!vex
    vector v = {1.0, -0.5, 1.1}
    if (abs(v) > 1.0) {
        // vector is greater than unit scale
    }
    }}}

@related

- [Vex:sign]
```

测试用npm run compile然后vsc里打开ts文件再选run and debug再选vscode dev

打包用vsce package

## Features

- Syntax Highlighting
![](https://raw.githubusercontent.com/supernova-explosion/houdini-vex-vscode-extension/main/images/syntax.png)

- Auto Completion
![](https://raw.githubusercontent.com/supernova-explosion/houdini-vex-vscode-extension/main/images/completion.png)

- Hover Document
![](https://raw.githubusercontent.com/supernova-explosion/houdini-vex-vscode-extension/main/images/hover.png)

- Base Snippets
![](https://raw.githubusercontent.com/supernova-explosion/houdini-vex-vscode-extension/main/images/snippets.png)

## More powerful houdini code editor

* [MagiCode](https://unrealhoudini.gumroad.com/l/rawnh)