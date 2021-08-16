
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <pthread.h>

const int WIDTH = 800, HEIGHT = 800;
const double G = 6.67259;
const int MAX_x = 500, MIN_x  = 300, MAX_y = 500, MIN_y  = 300, MAX_w = 100, MIN_w  = 50;

struct Body{
    double x,y,vx,vy,w;
};

int body_num = 200;
int iteration_num = 1000;

void randAssign(Body Nbody[],int body_num){
    for(int i=0;i<body_num;i++) {
        Nbody[i].x = rand() % (MAX_x-MIN_x)+MIN_x;
        Nbody[i].y = rand() % (MAX_y-MIN_y)+MIN_y;
        Nbody[i].vx = 0;
        Nbody[i].vy = 0;
        Nbody[i].w = rand() % (MAX_w-MIN_w)+MIN_w;
    }
}
void renewAssign(int body_num, Body Nbody[],double vx[], double vy[]){
    for(int i=0;i<body_num;i++) {
            Nbody[i].x = Nbody[i].x + vx[i] * 0.01;
            Nbody[i].y = Nbody[i].y + vy[i] * 0.01;
            Nbody[i].vx = vx[i];
            Nbody[i].vy = vy[i];
    }
}

pthread_mutex_t mutex;
int startx[2];
struct Body Nbody[1000];
double vx[1000];
double vy[1000];

void *forcecal(void* para)
{
    int *startx = (int *)para;
    int startX = startx[0];

    double deltaX, deltaY;
    double distance;
    double F;
    while(startX < body_num - 8){
        for(int j=0;j<body_num;j++)
        {
        if (j==startX) continue;
        deltaX = Nbody[j].x - Nbody[startX].x;
        deltaY = Nbody[j].y - Nbody[startX].y;
        distance = sqrt((deltaX * deltaX) + (deltaY * deltaY));
        if(distance == 0) continue;
        F = G * Nbody[j].w / (distance*distance);
        if(distance >= 25)
            {
            vx[startX] = vx[startX] + 0.01 * F * deltaX/distance;
            vy[startX] = vy[startX] + 0.01 * F * deltaY/distance;
            }
        }
    pthread_mutex_lock(&mutex);
    startX++;
    pthread_mutex_unlock(&mutex);
    }
}

int main (int argc,char *argv[])
{
        printf("Name: Chen Yanyu\nStudent ID: 118010029\nAssignment 3, N-Body Simulation, Pthread Implementation\n");
        int nthread;
        body_num = atoi(argv[1]);
        iteration_num = atoi(argv[2]);
        nthread = atoi(argv[3]);

        XInitThreads();

        unsigned int x = 0, y = 0;
        char *display_name = NULL;
        unsigned long valuemask = 0;
        XGCValues values;
        XSizeHints      size_hints;

        XSetWindowAttributes attr[1];
        Display *display = XOpenDisplay (display_name);
        if (display == NULL) {
           fprintf (stderr, "Something wrong with the X server %s\n",
                                XDisplayName (display_name) );
        }
        int screen = DefaultScreen (display);
        unsigned int display_width = DisplayWidth (display, screen);
        unsigned int display_height = DisplayHeight (display, screen);
        unsigned int border_width = 4;
        Window win = XCreateSimpleWindow (display, RootWindow (display, screen),
                                x, y, WIDTH, HEIGHT, border_width,
                                BlackPixel (display, screen), WhitePixel (display, screen));

        size_hints.flags = USPosition|USSize;
        size_hints.x = x;
        size_hints.y = y;
        size_hints.width = WIDTH;
        size_hints.height = HEIGHT;
        size_hints.min_width = 300;
        size_hints.min_height = 300;

        XSetNormalHints (display, win, &size_hints);
        GC gc = XCreateGC (display, win, valuemask, &values);

        XSetBackground (display, gc, WhitePixel (display, screen));
        XSetForeground (display, gc, BlackPixel (display, screen));
        XSetLineAttributes (display, gc, 1, LineSolid, CapRound, JoinRound);

        attr[0].backing_store = Always;
        attr[0].backing_planes = 1;
        attr[0].backing_pixel = BlackPixel(display, screen);

        XChangeWindowAttributes(display, win, CWBackingStore | CWBackingPlanes | CWBackingPixel, attr);

        XMapWindow (display, win);
        XSync(display, 0);

        int scr = DefaultScreen(display);
        int pm = XCreatePixmap(display,win,WIDTH,HEIGHT,DefaultDepth(display,scr));
        srand(time(NULL));
        randAssign(Nbody,body_num);

        struct timespec start, end;
        clock_gettime(CLOCK_MONOTONIC, &start);

        pthread_t tid[nthread];
        pthread_mutex_init(&mutex, NULL);

        for(int k = 0; k < iteration_num; k++)
        {
            XSetForeground(display,gc,0);
            XFillRectangle(display,pm,gc,0,0,WIDTH,HEIGHT);

            for (int i = 0; i < nthread; i++)
            {
                startx[0] = i;
                pthread_create(&tid[i], NULL, &forcecal, &startx);
            }

            for (int i = 0; i < nthread; i++)
            {
                pthread_join(tid[i], NULL);
            }
            renewAssign(body_num,Nbody,vx,vy);

            XSetForeground(display, gc, WhitePixel(display,scr));

            for(int i = 0; i < body_num - 8; i++)
            {
                if(Nbody[i].y <= HEIGHT && Nbody[i].x <= WIDTH)
                    XDrawPoint(display, pm, gc, (int)Nbody[i].y, (int)Nbody[i].x);
            }
            XCopyArea(display,pm,win,gc,0,0,WIDTH,HEIGHT,0,0);
            XFlush(display);
        }
    clock_gettime(CLOCK_MONOTONIC, &end);
    double time_count = end.tv_sec-start.tv_sec + (double)(end.tv_nsec - start.tv_nsec)/1000000000.0;
    printf("Execution time is: %f seconds\n", time_count);

    XFreePixmap(display,pm);
    XCloseDisplay(display);
    return 0;
}
