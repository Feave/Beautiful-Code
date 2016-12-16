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
//    while (1) {
//        for (int k = 0; k < 9600000; k++) {}
//        usleep(10000);
//    }
    return 0;
}
