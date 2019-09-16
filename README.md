## 更新：
### 取消了模拟复制操作，直接从Primary Selection中获取选中字符，相关代码已注释，未移除，当做来日学习之用。

<br>

# 程序功能
* 监测鼠标，获取屏幕上选中的词或段落

# 依赖
* C语言库: Xlib (在类Debian系统上命名为libx11-dev libxtst-dev， 在Arch Linux下是libxtst libx11)
 <del>* 终端命令行工具: xdotool, ps, awk, tail
 
# 编译 

     $ gcc -g getClipboard.c  DetectMouse.c -o main -lX11 -lXtst

# 运行
     $ ./main

# 注意事项
 1. 相关依赖如果没有请自行安装
  
 <del>2. 终端要正常运行，请执行命令 

        $ ps -p `xdotool getwindowfocus getwindowpid` |awk '{print $NF}' | tail -n 1 

    **将得到的终端应用名添加到DetectMouse.c的termName数组中并拓展数组容量**，否则在监测终端复制文字的时候发送的是Ctrl-C，而不是真正的复制快捷键Ctrl-Shift-C.

 <del>3. **如果终端使用了Smart copy，在没有选中任何文字的时候，可能会导致模拟发送的Ctrl-Shift-C被终端视为Ctrl-C而导致运行中的程序意外结束，这不是本程序的Bug，可以将Smart Copy关闭防止此类危险情况发生**  

 <del>4. 如果程序提取到用鼠标获取的结果一直是同一个或者为空，但是用键盘Ctrl-c操作能够获取到对应新的文本,换而言之就是鼠标取词不起作用，这说明打开的键盘设备文件是错的，模拟发送的Ctrl-c不会被捕捉到，此时可以用如下命令得到真正的键盘设备: <br> 
    
         cat  /var/log/Xorg.xx.log | grep keyboard | grep event | tail -n 1  

    (请将Xorg.xx.log替代为你当前系统的实际日志文件)，其输出类似:<br> 

        [275.556] (II) event3  - AT Translated Set 2 keyboard: device removed 

    可以看到，当前我系统使用的键盘设备文件是`/dev/input/event3`<br> 
    当然，我们也可以用其他方法来查明。

    **最后找到DetectMouse.c中/dev/input/eventX这条语句，将eventX修改成你实际得到的结果重新编译运行即可**
  </del>
   
5. 如果**运行报错failed to open mice的问题**，这个是因为没有权限打开文件进行读写导致的，可以有如下解决办法：    
    * **方法一: 添加当前用户到/dev/input/mice的用户组中**：<br>
        * a. 先查明此文件设备所在用户组,使用命令:<br> 
  
                ls -l /dev/input/mic

           b. 得到结果类似如下:<br>  

                crw-rw---- 1 root input 13, 63 Jul 27 09:09 /dev/input/mice

           c. 其中的input即是其所在用户组，得到后使用如下命令**添加用户到input用户组并重启系统**:<br> 

                sudo usermod -aG input userName
            
            **Note**: **如果设备文件没有用户组，请先手动设置规则添加，网上很多相关内容，这里不赘述.** <br><br>

    * **方法二: 使用sudo执行此程序**: <br> 
     
           sudo ./main 

         中途测试过程中以root用户或者说sudo执行时Xdisplay发生过 No protocol specified的错误，所以此方法不一定奏效，但也可能是当时系统忘记关闭Wayland导致的。

<br>

# Features
* Detect the action of mouse, and get the selected text of the screen

# Dependencies
* C library: Xlib (might be named libx11-dev and libxtst-dev on the system which base on Debian, and named libxtst libx11 in Arch Linux) <br> 
  
* Terminal tools: xdotool, ps, awk, tail

# Compile the program
     $ gcc -g getClipboard.c  DetectMouse.c -o main -lX11 -lXtst

# Run
     $ ./main

# Notifications
1. Please install the dependencies yourself if you didn't install before 

 <del>2. If you want to make the terminal works normally with this program, please execute the following command 

         $ ps -p `xdotool getwindowfocus getwindowpid` |awk '{print $NF}' | tail -n 1 

    and then add the application name from output into the termName array of C source file  which named DetectMouse.c. Sure we should expand the capacity of array. Or the program might send Ctrl-C to terminal rather than Ctlr-Shift-C.

 <del>3. **If the terminal enable the Smart copy feature, it might treat the Ctrl-Shift-C as Ctrl-C if there is no text selected, which will case the unexpected stopping of running program in the terminal. This is not the bug of program. We could disable Smart copy to prevent this dangurous thing happening** 

 <del>4. If the program get the null or same message all the time, it other words, the selected text on the screen do not be captured by the program. It might be the reason of the wrong selected of the keyboard device. Please try to fingure it out which the actual keyboard device is in your current system by using: <br>  
   
        cat  /var/log/Xorg.xx.log | grep keyboard | grep event | tail -n 1
    (Please replace the Xorg.xx.log as your actual log file, which might be like Xorg.0.log or Xorg.1.log) , the output just like this:  

       [275.556] (II) event3  - AT Translated Set 2 keyboard: device removed 

    As we can see, the keyboard device in my system use is `/dev/input/event3` <br> 
    Sure we can use other ways to figure it out. 

    **Finally find the sentence `/dev/input/eventX` in DetectMouse.c and replace the eventX with your result above and recompile 、run the program**.
<br> 

   
5. The problem of **'failed to open mice'**. This is because of the none permission of read and write of this device file. Please see the following solutions:
    * **First Method: Add current user to the group of /dev/input/mice**：<br>
        * a. Check to see which group the device file belong to by using command:<br> 
         
                ls -l /dev/input/mice

           b. The output just like this:<br>  

                crw-rw---- 1 root input 13, 63 Jul 27 09:09 /dev/input/mice

           c.__intpu__ is the result we need. Now we need to **add the current user to this    group by using following command line and then reboot our system**:<br>
                
               sudo usermod -aG input userName 
            
            **Note**: **If the device file do not belong to any group. Please write the related rule file to add the device to a group and then execute above steps** <br><br>

    * **Second method: execute through sudo**: <br>
        * `sudo ./main`<br>
        When I try to do this. I have met the error ` 'No protocol specified'` occur with Xdisplay, This might be also the reason of the opening of Waylan. But now it works for me (Sure I disable the Wayland completely).

<br>