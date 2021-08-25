# CharAnim
使用字符在终端中播放动画

效果请参见[人类高质量C++播放器](https://www.bilibili.com/video/bv11L4y1a7qj)

## 如何使用
### **注意：在Windows上可能不会正确显示**
确保已安装 `opencv4`, `g++` (或 `clang++`) 以及 `pkg-cofig` 并运行以下命令
```
pkg-config opencv4 --libs --cflags
```
复制该命令的输出，并运行
```
g++ -g main.cpp -o main -pthread -std=c++14 -I /usr/include/opencv4 刚刚复制的内容
```
编译完成，执行`./main`，输入视频路径以及目标宽高，即可看到效果