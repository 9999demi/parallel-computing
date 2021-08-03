
#include <string>
#include <cstring>
#include <vector>
#include <malloc.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <time.h>
#include <iostream>
#include "pthread.h"
#include <X11/Xlib.h>

#define ROOT 0
#define MAX_THREADS 10000

using namespace std;

GC gc;
Display *display;
Window window;
int screen;
XGCValues values;
long valuemask = 0;

double x1 = 2, x2 = -2, y1 = 2, y2 = -2;
int nthread;
int width;
int height;
const int MAX_ITERATION = 1000;
bool enable_flag = 0;

int tasks[MAX_THREADS];
int taskRemaining;
pthread_mutex_t	mutexJob;
pthread_mutex_t mutexX;

struct Compl{
    double real, imag;
};

void* Mandelbrot_calc(void *t){
    int pid;
    int j;
    int k;
    Compl z;
    Compl c;
    float lengthsq;
    float temp;
    pid = *((int*)t);
    while (true){
        for (j=0;j<height;j++){
            z.real = 0.0;
            z.imag = 0.0;
            double scaleX = width / (x1 - x2);
            double scaleY = height / (x1 - y2);
            c.imag = ((double)pid + scaleX * x2) / scaleX;
            c.real = ((double)j + scaleY * y2) / scaleY;
            k = 0;
            lengthsq=0.0;
            while (k<MAX_ITERATION && lengthsq<4.0){
                temp = z.real*z.real - z.imag*z.imag + c.real;
                z.imag = 2.0*z.real*z.imag + c.imag;
                z.real = temp;
                lengthsq = z.real*z.real+z.imag*z.imag;
                k++;
            }
            /* Draw Points */
            if (enable_flag){
                pthread_mutex_lock(&mutexX);
                XSetForeground (display, gc, 1024 * 1024 * (k % 256));
                XDrawPoint(display,window,gc,j,pid);
                pthread_mutex_unlock(&mutexX);
            }
        }
        pthread_mutex_lock(&mutexJob);
        if (taskRemaining<0){
            pthread_mutex_unlock(&mutexJob);
            pthread_exit(NULL);
        }
        else pid=taskRemaining--;
        pthread_mutex_unlock(&mutexJob);
    }
}
void initGraph(int x, int y, int width, int height){
    display = XOpenDisplay(NULL);
    if(display == NULL){
        exit(1);
    }
    screen = DefaultScreen(display);

    XGCValues values;
    long valuemask = 0;
    int border_width = 0;

    window = XCreateSimpleWindow(display, RootWindow(display, screen), x, y, width, height, border_width,
                    BlackPixel(display, screen), WhitePixel(display, screen));


    gc = XCreateGC(display, window, valuemask, &values);
    XSetForeground(display, gc, BlackPixel (display, screen));
    XSetBackground(display, gc, 0X0000FF00);
    XSetLineAttributes (display, gc, 1, LineSolid, CapRound, JoinRound);

    XMapWindow(display, window);
    XSync(display, 0);

    XSetForeground(display,gc,BlackPixel(display,screen));
    XFillRectangle(display,window,gc,0,0,width,height);
    XFlush(display);
}


int main(int argc, char* argv[])
{

    width = atoi(argv[1]);
    height = atoi(argv[1]);
    nthread = atoi(argv[2]);
    enable_flag = 1;

    printf("Name: Chen Yanyu\nStudent ID: 118010029\nAssignment 2, Mandelbrot Set, Pthread Implementation\n");

    if(enable_flag){
        initGraph((int)(x1+x2)/2, (int)(y1+y2)/2, width, height);
    }

    struct timespec start, finish;
    double time;

    clock_gettime(CLOCK_MONOTONIC, &start);


    pthread_mutex_init(&mutexJob, NULL);
    pthread_mutex_init(&mutexX, NULL);
    pthread_t *threads=(pthread_t*)malloc(sizeof(pthread_t)*nthread);

    taskRemaining=width-nthread;
    for (int i=0;i<nthread;i++){
        tasks[i]=width-i-1;
        pthread_create(&threads[i],NULL,Mandelbrot_calc,(void*)&tasks[i]);
    }

    for (int i=0;i<nthread;++i){
        pthread_join(threads[i],NULL);
    }

    if (enable_flag){
        XFlush(display);
    }
    clock_gettime(CLOCK_MONOTONIC, &finish);
    time = finish.tv_sec-start.tv_sec + (double)(finish.tv_nsec - start.tv_nsec)/1000000000.0;
    printf("Execution Time is: %f seconds\n", time);
    sleep(5);
    pthread_exit(NULL);
    return 0;
}
