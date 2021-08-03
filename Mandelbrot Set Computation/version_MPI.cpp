#include <mpi.h>
#include<X11/Xlib.h>
#include<unistd.h>
#include<cstdio>
#include<cstdlib>
#include<cstring>


#define ROOT 0
using namespace std;

GC gc;
Display *display;
Window window;
int screen;

int MAX_ITERATION, width, height;
double x1 = 2, x2 = -2, y1 = 2, y2 = -2;
int enable = 0;
double begin, end, time;

void draw(int i, int j, int repeats){
    XSetForeground(display, gc, 200 * 200 *(repeats % 256));
    XDrawPoint(display,window,gc,i,j);
}
void initGraph(int x, int y, int width,int height){
    display = XOpenDisplay(NULL);
    if(display == NULL){
        exit(0);
    }
    screen = DefaultScreen(display);
    int border_wid = 0;
    window - XCreateSimpleWindow(display, RootWindow(display, screen), x, y, width, height,border_wid, BlackPixel(display,screen), WhitePixel(display, screen));
    XGCValues val;

    long val_mask=0;

    gc = XCreateGC(display,window,val_mask, &val);
    XSetLineAttributes (display, gc, 1, LineSolid, CapRound, JoinRound);
    XSetBackground(display, gc, 0X0000FF00);
    XSetForeground(display, gc, BlackPixel (display, screen));
    XMapWindow(display,window);
    XSync(display,0);
    XSetForeground(display,gc,BlackPixel(display,screen));
    XFillRectangle(display,window,gc,0,0,width,height);
    XFlush(display);
}



struct Compl{
	double real, imag;
};


int main(int argc, char* argv[]){
    int rank, size;
    int *reps, *result;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    width = atoi(argv[1]);
    height = atoi(argv[2]);
    MAX_ITERATION = atoi(argv[3]);
    if(strcmp("enable", argv[4])==0) enable = 1;

    if(rank==ROOT){
        printf("Name: Chen Yanyu\nStudent ID: 118010029\nAssignment 2, Mandelbrot Set, MPI Implementation\n");
        begin = MPI_Wtime();
        if(enable)
            initGraph((int)(x1+x2)/2, (int)(y1+y2)/2, width, height);
    }

    double tmp;
    double sq_len;

    int chunk;
    int count=0;

    Compl z;
    Compl c;

    int repeat;

    if(width%size==0){
        chunk=width/size;
    }
    else {
        chunk=(width+(size-width%size))/size;
    }
    reps = new int[height*chunk];

    if(rank==ROOT){
        result = new int[chunk*size*height];
    }
    for(int i=rank*chunk; i<rank*chunk+chunk && i<width; i++){

        for(int j=0; j<height; j++){
            z.real = 0.0;
            z.imag = 0.0;

            c.real = x1+i*(x2-x1)/width;
            c.imag = y1+j*(y2-y1)/height;

            repeat = 0;
            sq_len = 0.0;

            while(i<width && repeat < MAX_ITERATION && sq_len < 4.0) {
                tmp = z.real*z.real - z.imag*z.imag + c.real;
                z.imag = 2*z.real*z.imag + c.imag;
                z.real = tmp;
                sq_len= z.real*z.real + z.imag*z.imag;
                repeat++;
            }
            reps[count++] = repeat;
        }

    }


    MPI_Gather(reps, height*chunk, MPI_INT, result, height*chunk, MPI_INT, ROOT, MPI_COMM_WORLD);
    if(rank==ROOT){
        end = MPI_Wtime();
        time = (end - begin);
        printf("The runtime is: %f seconds.\n", time);
    }

    if(enable && rank==ROOT){
        count = 0;
        for(int i=0;i<width;i++)
            for(int j=0;j<height;j++)
                draw(i, j, result[count++]);
        XFlush(display);
        printf("Display the image for 5 seconds...\n");
        sleep(5);
        printf("Finish Drawing.\n");
    }
    MPI_Finalize();
    return 0;
}
