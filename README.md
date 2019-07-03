# 程序功能
* 监测鼠标，获取屏幕上选中的词或段落

# 依赖
* C语言库: Xlib
* 终端命令行工具: xdotool, ps, awk, tail
 
# 编译
* $ gcc -g getClipboard.c  DetectMouse.c -o main -lX11 -lXtst

# 运行
* $ ./main

# 注意事项
1. 相关依赖如果没有请自行安装
2. 终端要正常运行，请执行命令 

    $ ps -p \`xdotool getwindowfocus getwindowpid\` |awk '{print $NF}' | tail -n 1 

    将得到的终端应用名添加到DetectMouse.c的termName数组中并拓展数组容量，否则在监测终端复制文字的时候发送的是Ctrl-C，而不是真正的复制快捷键Ctrl-Shift-C.

3. **如果终端使用了Smart copy，在没有选中任何文字的时候，可能会导致模拟发送的Ctrl-Shift-C被终端视为Ctrl-C而导致运行中的程序意外结束，这不是本程序的Bug，可以将Smart Copy关闭防止此类危险情况发生** 

<br>

# Features
* Detect the action of mouse, and copy the selected text of the screen

# Dependencies
* C library: Xlib
* Terminal tools: xdotool, ps, awk, tail

# Compile the program
* $ gcc -g getClipboard.c  DetectMouse.c -o main -lX11 -lXtst

# Run
* $ ./main

# Notifications
1. Please install the dependencies yourself if you didn't install before
2. If you want to make the terminal works normally, please execute the following command 

    $ ps -p \`xdotool getwindowfocus getwindowpid\` |awk '{print $NF}' | tail -n 1 

    and then add the application name from output into the termName array of C source file  which named DetectMouse.c. Sure we should expand the capacity of array. Or the program might send Ctrl-C to terminal rather than Ctlr-Shift-C.

3. **If the terminal enable the Smart copy feature, it might treat the Ctrl-Shift-C as Ctrl-C if there is no text selected, which will case the unexpected stopping of running program in the terminal. This is not the bug of program. We could disable Smart copy to prevent this dangurous thing happening**

