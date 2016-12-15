//
//  main.c
//  CPUManager
//
//  Created by 余笃 on 2016/12/15.
//  Copyright © 2016年 Feave. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <math.h>

int main(int argc, const char * argv[]) {
    struct timeval tv;
    int count = 300;
    double busy_time[300];
    double idle_time[300];
    for (int i = 0; i < count; i++) {
        busy_time[i] = 100000*(sin(i*M_PI*2/300)+1)*0.5;
        idle_time[i] = 100000 - busy_time[i];
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
