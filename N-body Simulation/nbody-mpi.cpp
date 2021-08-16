
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <mpi.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

const int WIDTH = 800, HEIGHT = 800;
const double G = 6.67259;
const int MAX_x = 500, MIN_x  = 300, MAX_y = 500, MIN_y  = 300, MAX_w = 100, MIN_w  = 50;

struct Body{
    double x,y,vx,vy,w;
};

void randAssign(Body Nbody[], int body_num){
    for(int i=0;i<body_num;i++){
          Nbody[i].x = rand() % (MAX_x-MIN_x) + MIN_x;
          Nbody[i].y = rand() % (MAX_y-MIN_y) + MIN_y;
          Nbody[i].vx = 0;
          Nbody[i].vy = 0;
          Nbody[i].w = rand()% (MAX_w-MIN_w) + MIN_w;
    }
}

void renewAssign(int startPoint, int job, Body Nbody[], double vx[], double vy[]){
    for(int i=startPoint;i<job + startPoint;i++){
        Nbody[i].x = Nbody[i].x + vx[i] * 0.01;
        Nbody[i].y = Nbody[i].y + vy[i] * 0.01;
        Nbody[i].vx = vx[i];
        Nbody[i].vy = vy[i];
    }
}
void localToNbody(Body local[],Body Nbody[],int startPoint,int job){
    for(int i=0;i<job;i++){
        local[i].x = Nbody[startPoint+i].x;
        local[i].y = Nbody[startPoint+i].y;
        local[i].vy = Nbody[startPoint+i].vy;
        local[i].vx = Nbody[startPoint+i].vx;
        local[i].w = Nbody[startPoint+i].w;
    }
}
int main (int argc, char *argv[]){
    printf("Name: Chen Yanyu\nStudent ID: 118010029\nAssignment 3, N-Body Simulation, MPI Implementation\n");
    int body_num = atoi(argv[1]);
    int iteration_num = atoi(argv[2]);

    int size, rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    double start, end;
    start = MPI_Wtime();

    MPI_Datatype MPIBody;
    MPI_Type_contiguous(5, MPI_DOUBLE, &MPIBody);
    MPI_Type_commit(&MPIBody);

    MPI_Status status;
    int job = body_num /size;

    double vx[body_num], vy[body_num], deltaX, deltaY, distance, F;

    struct Body *local_Nbody;
    local_Nbody = (struct Body*)malloc(job * sizeof(struct Body));
    struct Body* Nbody = (struct Body*)malloc(body_num * sizeof(struct Body));

    if (rank == 0)
    {
        Window          win;
        unsigned int x = 0, y = 0;

        char *display_name = NULL;
        GC gc;
        unsigned long valuemask = 0;
        XGCValues values;
        Display *display = XOpenDisplay (display_name);
        XSizeHints size_hints;

        XInitThreads();
        XSetWindowAttributes attr[1];

        if (display == NULL){
            fprintf (stderr, "Errors occur with the X server %s\n",XDisplayName (display_name) );
        }

        unsigned int screen = DefaultScreen (display);
        unsigned int display_width = DisplayWidth (display, screen);
        unsigned int display_height = DisplayHeight (display, screen);

        unsigned int border_width = 4;

        win = XCreateSimpleWindow (display, RootWindow (display, screen),x, y, WIDTH, HEIGHT, border_width, BlackPixel (display, screen), WhitePixel (display, screen));

        size_hints.flags = USPosition|USSize;
        size_hints.x = x;
        size_hints.y = y;
        size_hints.width = WIDTH;
        size_hints.height = HEIGHT;
        size_hints.min_width = 300;
        size_hints.min_height = 300;

        XSetNormalHints (display, win, &size_hints);
        gc = XCreateGC (display, win, valuemask, &values);
        XSetBackground (display, gc, WhitePixel (display, screen));
        XSetForeground (display, gc, BlackPixel (display, screen));
        XSetLineAttributes (display, gc, 1, LineSolid, CapRound, JoinRound);

        attr[0].backing_store = Always;
        attr[0].backing_planes = 1;
        attr[0].backing_pixel = BlackPixel(display, screen);

        XChangeWindowAttributes(display, win, CWBackingStore | CWBackingPlanes | CWBackingPixel, attr);

        XMapWindow (display, win);
        XSync(display, 0);

        int pm = XCreatePixmap(display,win,WIDTH,HEIGHT,DefaultDepth(display,screen));

        srand(time(NULL));
        randAssign(Nbody,body_num);

        for (int i = 1; i < size; i++){
            MPI_Send(Nbody, body_num, MPIBody, i, i, MPI_COMM_WORLD);
        }

        for(int k=0;k<iteration_num;k++){
            int startPoint = job * rank;

            XSetForeground(display,gc,0);
            XFillRectangle(display,pm,gc,0,0,WIDTH,HEIGHT);

            for(int i=startPoint;i<job + startPoint;i++){
                for(int j=0;j<body_num;j++){
                    if (j==i) continue;
                    deltaX = Nbody[j].x - Nbody[i].x;
                    deltaY = Nbody[j].y - Nbody[i].y;
                    distance = sqrt((deltaX * deltaX) + (deltaY * deltaY));
                    if(distance == 0) continue;
                    if(distance >= 15){
                        F = G * Nbody[j].w / (distance*distance);
                        vx[i] = vx[i] + 0.01 * F * (deltaX/distance) ;
                        vy[i] = vy[i] + 0.01 * F * (deltaY/distance) ;
                    }
                }
            }
            renewAssign(startPoint,job,Nbody,vx,vy);

            localToNbody(local_Nbody,Nbody,startPoint,job);

            MPI_Gather(local_Nbody, job, MPIBody, Nbody, job, MPIBody, 0, MPI_COMM_WORLD);

            XSetForeground(display, gc, WhitePixel(display,screen));
            for(int i=0;i<body_num;i++) {
                if(Nbody[i].y <= HEIGHT && Nbody[i].x <= WIDTH)
                    XDrawPoint(display, pm, gc, Nbody[i].y, Nbody[i].x);
            }
            XCopyArea(display,pm,win,gc,0,0,WIDTH,HEIGHT,0,0);

            for (int j = 1; j < size; j++)
                MPI_Send(Nbody, body_num, MPIBody, j, j, MPI_COMM_WORLD);

        }
        XFreePixmap(display,pm);
        XCloseDisplay(display);
    }
    else{
        MPI_Recv(Nbody, body_num, MPIBody, 0, rank, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        for(int k=0;k<iteration_num;k++){
            int startPoint = job * rank;
            for(int i=startPoint;i<job + startPoint;i++){
                for(int j=0;j<body_num;j++){
                    if (j==i) continue;
                    deltaX = Nbody[j].x - Nbody[i].x;
                    deltaY = Nbody[j].y - Nbody[i].y;
                    distance = sqrt((deltaX * deltaX) + (deltaY * deltaY));
                    if(distance == 0) continue;
                    if(distance >= 15){
                        F = G * Nbody[j].w / (distance*distance);
                        vx[i] = vx[i] + 0.01 * F * (deltaX/distance);
                        vy[i] = vy[i] + 0.01 * F * (deltaY/distance);
                    }
                }
            }
            renewAssign(startPoint,job,Nbody,vx,vy);

            localToNbody(local_Nbody,Nbody,startPoint,job);

            MPI_Gather(local_Nbody, job, MPIBody, Nbody, job, MPIBody, 0, MPI_COMM_WORLD);
            MPI_Recv(Nbody, body_num, MPIBody, 0, rank, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    }
    if(rank == 0)
    {
        end = MPI_Wtime();
        double time_count = end - start;
        printf("Execution Time is: %f seconds\n", time_count);
        printf("Finish Drawing...\n");
    }
    MPI_Finalize();
    return 0;
}
