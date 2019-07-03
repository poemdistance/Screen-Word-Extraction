/* 本程序功能:
 *
 * 通过操作鼠标设备文件，监听当前鼠标左键双击事件
 * 以及长按左键进行区域选择的事件。
 *
 * 通过键盘设备文件event3进行模拟键盘操作
 *
 * 当检测到双击或者区域选择操作，在操作结束后，
 * 模拟键盘发送CTRL-C
 *
 * 调用getClipboard()函数获取剪贴板内容
 * */

#include <string.h>  
#include <math.h>  
#include <stdlib.h>  
#include <sys/stat.h>  
#include <stdio.h>  
#include <signal.h>
#include <sys/types.h>  
#include <sys/stat.h>  
#include <fcntl.h>  
#include <linux/input.h>  
#include <time.h>
#include <linux/uinput.h>  
#include <stdio.h>  
#include <sys/time.h>  
#include <sys/types.h>  
#include <unistd.h>  
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>
#include "common.h"

const static char termName[3][19] = 
{
    "terminator",
    "gnome-terminal-",
    "konsole"
};

char *text = NULL;

int isTerminal(char *name) {

    int i, j;
    int n = sizeof(termName) / sizeof(termName[0]);
    printf("name=%s\n", name);
    char *p = name;
    while(*p++ != '\n');
    *(p-1) = '\0';

    for ( i = 0; i < n; i++ ) {
        if ( strcmp ( termName[i], name ) == 0 )
            return 1;
    }
    return -1;
}

/*获取当前数组下标的前一个下标值,
 *数组元素为4*/
int previous( int n )
{
    if ( n != 0 )
        return n - 1;
    else
        return  3;
}

/*退出前加个回车*/
void quit() {

    fprintf(stdout, "\n");
    if ( text != NULL )
        free(text);
    exit(0);
}

/*同步键盘*/
void sync_key(
        int *fd,
        struct input_event *event,
        int *len)
{
    event->type = EV_SYN;
    event->code = SYN_REPORT;
    event->value = 0;
    write(*fd, event, *len);
}


/*发送按键keyCode*/
void press(int fd, int keyCode)
{
    struct input_event event;

    //发送
    event.type = EV_KEY;
    event.value = 1;
    event.code = keyCode;
    gettimeofday(&event.time,0);
    write(fd,&event,sizeof(event)) ;

    //同步
    int len = (int)sizeof(event);
    sync_key(&fd, &event, &len);
}

/*释放按键*/
void release(int fd, int keyCode)
{
    struct input_event event;

    //释放
    event.type = EV_KEY;
    event.code = keyCode;
    event.value = 0;
    gettimeofday(&event.time, NULL);
    write(fd, &event, sizeof(event));

    //同步
    int len = (int)sizeof(event);
    sync_key(&fd, &event, &len);
}

int main(int argc,char **argv)  
{  
    int fd, retval, fd_key=0;  
    char buf[3];  
    FILE *fp = NULL;
    char appName[100];
    int releaseButton = 1;
    time_t current;
    fd_set readfds;  
    struct timeval tv;  
    struct timeval old, now;
    double oldtime = 0;
    double newtime = 0;
    int thirdClick;

    // 打开鼠标设备  
    fd = open("/dev/input/mice", O_RDONLY );  
    if ( fd < 0 ) {  
        fprintf(stderr, "Failed to open mice");
        exit(1);  
    } else {  
        fprintf(stdout, "Open mice successful\n");
    }  

    int history[4] = { 0 };
    int i = 0, n = 0, m = 0, j = 0, q = 0,\
            t = 0, y = 0;

    /*捕捉Ctrl-c退出信号*/
    signal(SIGINT, quit);
    while(1) {  
        // 设置最长等待时间  
        tv.tv_sec = 5;  
        tv.tv_usec = 0;  

        FD_ZERO( &readfds );  
        FD_SET( fd, &readfds );  

        retval = select( fd+1, &readfds, NULL, NULL, &tv );  
        if(retval==0) {  
            continue;
        }  
        if(FD_ISSET(fd,&readfds)) {  
            // 读取鼠标设备中的数据  
            if(read(fd, buf, 3) <= 0) {  
                continue;  
            }  

            /*循环写入鼠标数据到数组*/
            history[i++] = buf[0] & 0x07;
            if ( i == 4 )
                i = 0;

            /*m为最后得到的鼠标键值*/
            m = previous(i);
            n = previous(m);
            j = previous(n);
            q = previous(j);


            /* 0 1 表示按下鼠标左键, 则0101表示双击，这里不考虑间隔过久的双击事件,
             * 0 1 1 1 表示左键一直处于按下状态，进行区域选择，最后释放鼠标左键，
             * 则最后的值会变成0, 因为所有值都是在4个数组空间里循环，而m表示的是
             * 最后一个的值，所以是0111.
             *
             * 这里的逻辑是如果检测到鼠标双击或者区域选择事件，则发送ctlr-c进行
             * 复制操作.如果是在终端，则发送的ctrl-c将使程序停止运行.
             * */
            //fprintf(stdout, "%d / %d / %d / %d releaseButton=%d\n",\
            history[m],history[n],history[j],history[q], releaseButton );

            /* 鼠标稍微移动会造成数组值全部归零
             * 此时应该释放按钮，让左键按下后能够更新oldtime
             * 不然在任意处点击造成releaseButton=0,此时若双击
             * 任意字符串获得oldtime不是这次的更新值，将会被超时丢弃*/
            if ( history[m] == 0 && history[n] == 0 )
                releaseButton = 1;

            /*按下左键*/
            if ( history[m] == 1 && history[n] == 0 ) {

                if ( releaseButton ) {
                    time(&current);
                    gettimeofday(&old, NULL);

                    if (newtime != 0 && \
                            abs(newtime - ((old.tv_usec + old.tv_sec*1000000) / 1000)) < 600 )
                        thirdClick = 1;
                    else {
                        oldtime = (old.tv_usec + old.tv_sec*1000000) / 1000;
                        //fprintf(stdout, "oldtime=%f\n", oldtime);
                        thirdClick = 0;
                    }
                    releaseButton = 0;
                    if ( !thirdClick )
                        continue;
                }
            }

            /*检测双击时间间隔*/
            if ( history[m] == 0 && history[n] == 1\
                    && history[j] == 0 && history[q] == 1 )  {
                releaseButton = 1;
                gettimeofday( &now, NULL );
                newtime = (now.tv_usec + now.tv_sec*1000000) / 1000;
                //fprintf(stdout, "newtime=%f\n", newtime);
                //fprintf(stdout, "newtime=%f -oldtime= %f\n", newtime, oldtime);

                /*双击超过600ms的丢弃掉*/
                if ( abs (newtime - oldtime) > 600)  {
                    fprintf(stderr, "No effective = %f\n", newtime - oldtime);
                    memset(history, 0, sizeof(history));
                    continue;
                }
            }

            /*双击或者区域选择事件处理*/
            if ( ( history[m] == 0 && history[n] == 1 && history[j] == 0 && history[q] == 1 )
                    || ( history[m] == 0 && history[n] == 1 && history[j] == 1 && history[q] == 1 ) 
                    || thirdClick == 1)

            {
                if ( thirdClick == 1 ) {
                    thirdClick = 0;
                    /*通知已释放左键，让左键按下后前面的检测程序能更新oldtime*/
                    releaseButton = 1;
                }

                if ( fd_key <= 0 )
                    fd_key = open("/dev/input/event3", O_RDWR);
                if(fd_key < 0) {
                    fprintf(stderr, "Open keyboard device failed\n");
                    exit(1);
                }

                fp = popen("ps -p `xdotool getwindowfocus getwindowpid`\
                        | awk '{print $NF}' | tail -n 1", "r");
                if ( fp == NULL ) {
                    fprintf(stderr, "command execute failed\n");
                    continue;
                }

                memset ( appName, 0, sizeof(appName) );
                if ( fread(appName, sizeof(appName), 1, fp) < 0) {
                    fprintf(stderr, "fread error\n");
                    continue;
                }

                if ( pclose(fp) < 0) {
                    fprintf(stderr, "close pipe error \n");
                    continue;
                }

                if ( isTerminal(appName) == 1) {

                    fprintf(stdout, "Send ctrl-shift-c\n");

                    /*发送CTRL-SHIFT-C*/
                    press(fd_key, KEY_LEFTCTRL);
                    press(fd_key, KEY_LEFTSHIFT);
                    press(fd_key, KEY_C);

                    release(fd_key, KEY_C);
                    release(fd_key, KEY_LEFTSHIFT);
                    release(fd_key, KEY_LEFTCTRL);

                } else {

                    fprintf(stdout, "Send ctrl-c\n");

                    //发送CTRL-C
                    press(fd_key, KEY_LEFTCTRL);
                    press(fd_key, KEY_C);
                    release(fd_key, KEY_C);
                    release(fd_key, KEY_LEFTCTRL);
                }


                /*等待数据被写入剪贴板*/
                for(int i = 0; i < 1024; i++)
                    for ( int j = 0; j < 6024; j++ );

                if ( text == NULL )
                    text = malloc(1024*1024);

                //getClipboard(text);
                memset(text, 0, sizeof(text));
                getClipboard(text);

                if ( strcmp ( text, " ") != 0 
                        && strcmp ( text, "\n") != 0 
                   ) {
                    fprintf(stdout, "get text successful: %s\n", text);
                }

                memset(history, 0, sizeof(history));
            }
        }  
    }  

    close(fd);  
    return 0;  
}  
