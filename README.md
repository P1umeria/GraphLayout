### 概述
一个图可视化布局的工具，目前实现了fruchterman reingold、kamada kawaii、yifanhu、sugiyama四种图可视化算法。

### 编译环境
visual studio 2022
ISO C++17 标准

### 运行
```
<execute file> <in.dot> <out.png> [options]
Options:
    -m <method>           算法选择：Fruchterman Reingold, Kamada Kawai, Yifan Hu, Sugiyama [fr|kk|yfh|sgym, default: sgym]
    -w <width>            生成的png图像宽度(pixels) [default: 1024]
    -h <height>           生成的png图像高度(pixels) [default: 760]
    -k <strength>         Strength factor [default: 10 for fr, 300 for kk]
    -i <nb_iterations>    Number of iterations for fr [default: 100]
    -e <epsilon>          Energy threshold for kk [default: 1e-2]
    -a                    Output all intermediary frames for fr [default: false]
    -s <step ratio>       Step ratio for yfh [default: 0.95]
    -l <max level>        Quad tree max level for yfh [default: 20]
    -b <barnes hut theta> Barnes hut theta for yfh [default: 1.2]
    -c <convergence threshold>    for yfh
```
例如：
``` sh
.\GraphLayout.exe .\samples\dotfile\Huffman7.dot .\samples\result\Huffman_sgym.png -w 1024 -h 1024 -m sgym
```

生成的图片如下：

![](./samples/result/Huffman_sgym.png)

### 参考
* https://github.com/olvb/nodesoup
* https://github.com/Kali-Hac/YifanHu_python
* https://github.com/gml4gtk/demekgraph
