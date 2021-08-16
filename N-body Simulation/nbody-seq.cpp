
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>


const int WIDTH = 800, HEIGHT = 800;
const double G = 6.67259;
const int MAX_x = 500, MIN_x  = 300, MAX_y = 500, MIN_y  = 300, MAX_w = 100, MIN_w  = 50;

struct Body{
    double x,y,vx,vy,w;
};

void randAssign(int body_num, Body Nbody[]){
    for(int i=0;i<body_num;i++)
    {
        Nbody[i].x = rand() % (MAX_x-MIN_x)+MIN_x;
        Nbody[i].y = rand() % (MAX_y-MIN_y)+MIN_y;
        Nbody[i].vx = 0;
        Nbody[i].vy = 0;
        Nbody[i].w = rand() % (MAX_w-MIN_w)+MIN_w;
    }
}

void renewAssign(int body_num, Body Nbody[],double vx[], double vy[]){
    for(int i=0;i<body_num;i++)
    {
        Nbody[i].x = Nbody[i].x + vx[i] * 0.01;
        Nbody[i].y = Nbody[i].y + vy[i] * 0.01;
        Nbody[i].vx = vx[i];
        Nbody[i].vy = vy[i];
    }
}


int main (int argc,char *argv[]) {
        int body_num = atoi(argv[1]);
        int iteration_num = atoi(argv[2]);
        printf("Name: Chen Yanyu\nStudent ID: 118010029\nAssignment 3, N-Body Simulation, Sequential Implementation\n");

        Window          win;
        unsigned int x = 0, y = 0;

        char *display_name = NULL;
        GC gc;
        unsigned long valuemask = 0;
        XGCValues values;
        Display *display;
        XSizeHints size_hints;
        XInitThreads();

        XSetWindowAttributes attr[1];

        if ((display = XOpenDisplay(display_name)) == NULL) {
           fprintf(stderr, "Something wrong with the X server %s\n",
                                XDisplayName(display_name));
           exit(-1);
        }

        unsigned int border_width = 4;
        unsigned int screen = DefaultScreen(display);
        unsigned int display_width = DisplayWidth(display, screen);
        unsigned int display_height = DisplayHeight(display, screen);

        win = XCreateSimpleWindow (display,
                                RootWindow(display, screen),
                                x, y, WIDTH, HEIGHT, border_width,
                                BlackPixel(display, screen),
                                WhitePixel(display, screen));

        size_hints.flags = USPosition|USSize;
        size_hints.x = x;
        size_hints.y = y;
        size_hints.width = WIDTH;
        size_hints.height = HEIGHT;
        size_hints.min_width = 300;
        size_hints.min_height = 300;

        XSetNormalHints(display, win, &size_hints);

        gc = XCreateGC(display, win, valuemask, &values);

        XSetBackground(display, gc, WhitePixel(display, screen));
        XSetForeground(display, gc, BlackPixel(display, screen));
        XSetLineAttributes(display, gc, 1, LineSolid, CapRound,
                        JoinRound);

        attr[0].backing_store = Always;
        attr[0].backing_planes = 1;
        attr[0].backing_pixel = BlackPixel(display, screen);

        XChangeWindowAttributes(display, win,
            CWBackingStore | CWBackingPlanes | CWBackingPixel, attr);

        XMapWindow(display, win);
        XSync(display, 0);

        struct timespec start, finish;
        double count_time;
        clock_gettime(CLOCK_MONOTONIC, &start);

        struct Body Nbody[body_num];
        double vx[body_num];
        double vy[body_num];
        double deltaX, deltaY;
        double distance;
        double F;
        int scr = DefaultScreen(display);
        int pm = XCreatePixmap(display,win,WIDTH,HEIGHT,DefaultDepth(display,scr));

        srand(time(NULL));
        randAssign(body_num,Nbody);
        for(int k=0;k<iteration_num;k++)
        {
            XSetForeground(display,gc,0);
            XFillRectangle(display,pm,gc,0,0,WIDTH,HEIGHT);

            for(int i=0;i<body_num;i++)
            {
                for(int j=0;j<body_num;j++)
                {
                    if (j==i) continue;
                    deltaX = Nbody[j].x - Nbody[i].x;
                    deltaY = Nbody[j].y - Nbody[i].y;
                    distance = sqrt((deltaX * deltaX) + (deltaY * deltaY));
                    if(distance <= 3) continue;
                    F = G * Nbody[j].w / (distance*distance);
                    vx[i] = vx[i] + 0.01 * F * deltaX/distance;
                    vy[i] = vy[i] + 0.01 * F * deltaY/distance;
                }
            }
            renewAssign(body_num,Nbody,vx,vy);

            XSetForeground(display, gc, WhitePixel(display,scr));
            for(int i=0;i<body_num;i++)
            {
                XDrawPoint(display, pm, gc, Nbody[i].y, Nbody[i].x);
            }
            XCopyArea(display,pm,win,gc,0,0,WIDTH,HEIGHT,0,0);
        }

        clock_gettime(CLOCK_MONOTONIC, &finish);
        count_time = finish.tv_sec-start.tv_sec + (double)(finish.tv_nsec - start.tv_nsec)/1000000000.0;
        printf("Execution time is: %f seconds\n", count_time);


        XFreePixmap(display,pm);
        XCloseDisplay(display);

        return 0;
}
