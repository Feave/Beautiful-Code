#Mac下实现《编程之美》第一章-CPU使用率

##实现过程
###实现50%占用率
***
看完1.1节后直接撸代码，V0.1版本：
```c
    while (1) {
        for (int k = 0; k < 9600000; k++) {}
        //usleep()参数为微秒(us)
        usleep(10000);
    }
```
<p>&emsp;&emsp;这段代码很简单，原理书上都有，但是有一个问题就是怎么放在理想环境下测试，不然很难验证程序的效果，我尝试使用系统自带的活动监视器来监视CPU曲线，结果发现 so stupid ，完全不是那回事儿。但是在打断点进入汇编代码的过程中，我发现XCode的Debug工具自带CPU使用分析程序……。打开面板以后发现，窝草，简直完美！生活中不缺少美，而是缺少一双发现美的眼睛(ಥ_ಥ)_)。</p>
![图片被吞了](http://oi9lc0fjo.bkt.clouddn.com/image/beautiful-code/CPUManager/CPUManagerDebug_1.png "第一次运行结果")
<p>&emsp;&emsp;打开Xcode的调试面板我们就会惊喜的发现CPU使用稳定在了……66%，这TM就有点尴尬了，不过这至少证明该方法是可行而且成功了的。下面就是纠正了，这个很简单，使用率 `66% = busytime/busytime+idletime` ,可以得知 `busytime = 2 * idletime` ,所以将 for 循环中的 9600000 改为 4800000 即可。</p>
<p>&emsp;&emsp;但是很显然只是这样做是不够的，我们需要知道为什么第一次的运行结果是 66% 。书中是如何得出 9600000 这个数字的呢，因为 CPU 为 `2.4Ghz` ，而书中的空循环代码在转为汇编代码后为 `5` 条指令，而 CPU 每个时钟周期可以执行两条指令，所以一秒内可以循环 `（2400000000*2)/5 = 960000000` 次, 10 毫秒对应就是 9600000 。首先想到的问题就是 CPU 频率，这点可以很方便的从本机信息中知道，我的机器是 `2.7 GHz Intel Core i5` ，但是很显然这样做得出的循环次数比原来还要多，所以问题应该是汇编指令条数应该多于5条。然而像我这种汇编挂到大四的超级学渣很显然不可能根据 C 代码去反推汇编过程，所幸 XCode 的断点调试支持汇编 Debug (菜单栏 Product -> Debug Wokflow ，Show disassembly When Debugging 选项，打钩即可)。我们在空循环内打个断点，可以看到汇编代码如下:</p>
![](http://oi9lc0fjo.bkt.clouddn.com/image/beautiful-code/CPUManager/CPUManagerDebug_2.png "汇编代码")
<p>&emsp;&emsp;在 Google 的帮助下我们可以知道第 7 行是循环的开始——为 i 赋值，而结束就比较明显了——17 行调用了 usleep()，可以得出空循环执行了 `11` 条汇编指令,所以在一个周期中， busyloop 应该执行 ` (27000000*2)/11 = 4909090 `次，与调整后的循环次数 4800000 基本一致！撒花ヾ(^▽^ヾ)</p>
###动态调节忙/闲时间比
***
<p>&emsp;&emsp;思路就是在空循环中获取当前时间与起始时间对比，这样就可以直观的限定循环运行时间~~(所以说上面搞了这么久就是在瞎折腾)~~。与 Windows 不同的就是 Linux 系统的系统获取时间函数是 `gettimeofday`。这里要注意的就是返回值是一个结构体，其中秒数 tv_sec 与 微秒 tv_usec 是分开的，程序中以微秒为单位，所以当前时间应表示为 `tv.tv_sec * 1000000 + tv.tv_usec` 。代码如下:</p>
```c
    while (1) {
        gettimeofday(&tv, NULL);
        long endTime = tv.tv_sec * 1000000 + tv.tv_usec;
        long startTime = endTime;
        while (endTime - startTime <= busy_time) {
            gettimeofday(&tv, NULL);
            endTime = tv.tv_sec * 1000000 + tv.tv_usec;
        }
        usleep(idle_time*0.82);
    }
```
<p>&emsp;&emsp;这里执行后会发现使用率并没有稳定在 50% ，而是 45% ,具体原因并不知道……。所以我给 idletime 乘了一个 0.82 的系数。</p>
###正弦函数
<p>&emsp;&emsp;基本思路是调整每次循环的忙/闲时间比，使其符合正弦曲线。其实原理就跟初中时刚学函数一样，把函数理解为连续的点或者说线段连接起来就行了，只要点-即“时间片”足够小，表现出来就是曲线。经过几次调试，我把时间片定在0.1秒，数量为300个，这样在XCode上的显示效果比较好😁。</p>
<p>&emsp;&emsp;最终为了方便调试我把直线和正弦合并了，其实理论应该可以写成一个画出任意二元函数曲线的程序，代码如下:</p>
```c
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

int main(int argc, const char * argv[]) {
    struct timeval tv;
    int count = 300;
    char type[20];
    double busy_time[count];
    double idle_time[count];
    scanf("%s",type);
    if (!strcmp(type, "triggle")) {
        for (int i = 0; i < count; i++) {
            busy_time[i] = 100000*(sin(i*M_PI*2/count)+1)*0.5;
            idle_time[i] = 100000 - busy_time[i];
        }
    } else {
        for (int i = 0; i < count; i++) {
            busy_time[i] = 50000;
            idle_time[i] = 100000 - busy_time[i];
        }
    }
    int j = 0;
    while (1) {
        gettimeofday(&tv, NULL);
        long endTime = tv.tv_sec * 1000000 + tv.tv_usec;
        long startTime = endTime;
        while (endTime - startTime <= busy_time[j]) {
            gettimeofday(&tv, NULL);
            endTime = tv.tv_sec * 1000000 + tv.tv_usec;
        }
        usleep(idle_time[j]*0.82);
        j = (j+1)%count;
    }
    return 0;
}
```
<p>&emsp;&emsp; command+R 后我们就可以欣赏到 CPU 使用率画出优美的正弦曲线了，虽然不是很齐😂，但是还是很激动哒！</p>
![](http://oi9lc0fjo.bkt.clouddn.com/image/beautiful-code/CPUManager/CPUManagerDebug_3.png "美丽的正弦曲线")
####Tips
1. 系统sin()(或者说所有Trigger函数)参数为弧度制，所以参数应乘以PI(xcode中可使用M_PI)。
2. 在分开计算正弦函数分布的时间片时，`(i/count*M_PI*2)` 会导致结果一直为零，纠结了很久，结果发现把 count 放到后面就行了 `(i*M_PI*2/count)` ,原因是 i 与 count 都是 int 类型，直接除会直接导致取整为 0 ，感觉自己还是 naive ，毕竟 C 当初都是混过去的。。。

##总结
<p>&emsp;&emsp;刚开始看这一章的题目时候我还觉得很神奇，还可以控制 CPU 使用率画正弦曲线？但其实问题很简单就解决。当经过一番努力，最后看到那条曲线的时候，感觉突然就有点理解“编程之美”了。</p>
