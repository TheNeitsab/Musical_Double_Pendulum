//-------------------------------------------------------------
// MUSICAL DOUBLE PENDULUM
// GABRIELLI Bastien & PEIGNE Aurore
// First wo need to have allegro4 and csfml librairies intsalled
// sudo apt-get install libcsfml-dev
// sudo apt-get install liballegro4-dev
// Then to compile the project you must open a terminal in the project folder and use the command :
// gcc -Wall -Wextra -L./lib -I./inc src/MUSICAL_DOUBLE_PENDULUM_PEIGNE_GABRIELLI.c -o MUSICAL_DOUBLE_PENDULUM_PEIGNE_GABRIELLI -lptask -lpthread -lcsfml-audio -lm `allegro-config --libs`
//-------------------------------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <sched.h>
#include <allegro.h>
#include <time.h>
#include <stdbool.h>

#include "ptask.h"
#include "pmutex.h"
#include "tstat.h"
#include "SFML/Audio.h"


//-------------------------------------------------------------
// DOUBLE PENDULUM CONSTANTS
//-------------------------------------------------------------
#define   PI        3.14159265
#define   G         9.81  // acceleration due to gravity (in m/s^2) 

#define   L1        100.0 // length of pendulum 1 (in m) 
#define   L2        100.0 // length of pendulum 2 (in m) 
#define   M1        1.0   // mass of pendulum 1 (in kg) 
#define   M2        1.0   // mass of pendulum 2 (in kg)
#define   TH1       90    // angle of pendulum 1 (in radian)
#define   TH2       180   // angle of pendulum 2 (in radian)
#define   W1        0     // velocity angular of pendulum 1 (in radian)
#define   W2        0     // velocity angular of pendulum 2 (in radian)

#define   N         4     // number of equations for RK4 algorithm   
#define   TLEN      80    // trail length
#define   TSCALE    10    // time scale factor

//-------------------------------------------------------------
// TASKS CONSTANTS
//-------------------------------------------------------------
#define   PERIOD    20    // task period (in ms)
#define   DREL      20    // relative deadline (in ms)
#define   PRIORITY  80    // task priority
#define   ACT       1     // activation flag

//-------------------------------------------------------------
// GRAPHICS CONSTANTS
//-------------------------------------------------------------
#define   WIDTH     680   // window's width        
#define   HEIGHT    480   // window's height 

//-------------------------------------------------------------
// GLOBAL VARIABLES
//-------------------------------------------------------------
pthread_mutex_t   mxdp;   // mutual esclusion semaphore for double pendulum
pthread_mutex_t   mxb;    // mutual esclusion semaphore for buffer
pthread_mutex_t   mxs;    // mutual esclusion semaphore for sound

BITMAP  *buf;             // buffer to avoid flickering on screen

//-------------------------------------------------------------
// GLOBAL DATA STRUCTURES
//-------------------------------------------------------------
struct  double_pendulum{          // double pendulum parameters
        int   x0, y0;             // coordinates of the fixed point  
        float   x1, y1, x2, y2;   // coordinates of pendulums 1 & 2       
        float   th1, th2;         // angles of pendulums 1 & 2
        float   w1, w2;           // angular velocities of pendulums 1 & 2
}DOUBLE_PENDULUM;

struct  buffer{                   // trail circular buffer
        int   top;                // index of the current element
        int   x2[TLEN];           // array of x2 coordinates
        int   y2[TLEN];           // array of y2 coordinates
}BUFFER;

struct  sound{                    // sound parameters
        sfSoundBuffer   *buffer;  // buffer storing the sound sample
        sfSound   *DO;            // sound in C
        sfSound   *FA;            // sound in F
        sfSound   *FAD;           // sound in F#
        sfSound   *MIb;           // sound in Eb
        sfSound   *SIb;           // sound in Bb
        sfSound   *SOL;           // sound in G
        int   state;              // zone index for sound mapping
}SOUND;

//-------------------------------------------------------------
// FUNCTIONS PROTOTYPES
//-------------------------------------------------------------
void  init_double_pendulum(struct double_pendulum *dp);
void  init_buffer(struct buffer *b);
void  init_graphics(void);
void  init_sound(struct sound *s);
void  init_global(void);

void  draw_trail(void);
void  store_trail(struct buffer *b, int x2, int y2);

void  runge_kutta(float in[], float out[], float h);
void  derivs(float in[], float din[]);

//-------------------------------------------------------------
// TASKS DEFINITIONS
//-------------------------------------------------------------
ptask   double_pendulum_task(void){
    float   dt;                // integration interval   
    float   in[N], out[N];     // arrays storing old and new thi and wi values
    int   x2, y2;              // storing dp values

    dt = TSCALE * (float)PERIOD / 1000;

    while (1) {
        pthread_mutex_lock(&mxdp);

        // filling first array with old parameters 
        in[0] = DOUBLE_PENDULUM.th1;
        in[1] = DOUBLE_PENDULUM.w1;
        in[2] = DOUBLE_PENDULUM.th2;
        in[3] = DOUBLE_PENDULUM.w2;
        
        runge_kutta(in, out, dt);

        // updating double pendulum parameters
        DOUBLE_PENDULUM.th1 = out[0];
        DOUBLE_PENDULUM.w1 = out[1];
        DOUBLE_PENDULUM.th2 = out[2];
        DOUBLE_PENDULUM.w2 = out[3];
        DOUBLE_PENDULUM.x1 = DOUBLE_PENDULUM.x0 + L1 * sin(DOUBLE_PENDULUM.th1);
        DOUBLE_PENDULUM.y1 = DOUBLE_PENDULUM.y0 + L1 * cos(DOUBLE_PENDULUM.th1);
        DOUBLE_PENDULUM.x2 = DOUBLE_PENDULUM.x1 + L2 * sin(DOUBLE_PENDULUM.th2);
        DOUBLE_PENDULUM.y2 = DOUBLE_PENDULUM.y1 + L2 * cos(DOUBLE_PENDULUM.th2);

        x2 = DOUBLE_PENDULUM.x2;
        y2 = DOUBLE_PENDULUM.y2;

        pthread_mutex_unlock(&mxdp);

        pthread_mutex_lock(&mxb);
        store_trail(&BUFFER, x2, y2);
        pthread_mutex_unlock(&mxb);

        ptask_wait_for_period();
    }
}

ptask   graphic_task(void){
    while (1) {
        pthread_mutex_lock(&mxb);

        clear_to_color(buf, makecol(0, 0, 0));

        draw_trail();

        pthread_mutex_unlock(&mxb);

        pthread_mutex_lock(&mxdp);

        // graphical design of the double pendulum
        line(buf, DOUBLE_PENDULUM.x0, DOUBLE_PENDULUM.y0, DOUBLE_PENDULUM.x1, DOUBLE_PENDULUM.y1, makecol(255,0,0));                       
        line(buf, DOUBLE_PENDULUM.x1, DOUBLE_PENDULUM.y1, DOUBLE_PENDULUM.x2, DOUBLE_PENDULUM.y2, makecol(255,0,0));
        circlefill(buf, DOUBLE_PENDULUM.x0, DOUBLE_PENDULUM.y0, 4, makecol(255,255,255));       
        circlefill(buf, DOUBLE_PENDULUM.x1, DOUBLE_PENDULUM.y1, 4, makecol(255,255,255));  
        circlefill(buf, DOUBLE_PENDULUM.x2, DOUBLE_PENDULUM.y2, 4, makecol(255,255,255));

        pthread_mutex_unlock(&mxdp);

        pthread_mutex_lock(&mxs);

        // graphical sound mapping
        if (SOUND.state == 1) {
        rect(buf, 0, HEIGHT / 2, WIDTH / 3, 0, makecol(100,100,100));
        }
        if (SOUND.state == 2) {
        rect(buf, WIDTH / 3, HEIGHT / 2, 2 * WIDTH /3, 0, makecol(100,100,100));
        } 
        if (SOUND.state == 3) {
        rect(buf, 2 * WIDTH / 3, HEIGHT / 2, WIDTH - 1, 0, makecol(100,100,100));
        } 
        if (SOUND.state == 4) {
        rect(buf, 0, HEIGHT - 1, WIDTH / 3, HEIGHT / 2, makecol(100,100,100));
        } 
        if (SOUND.state == 5) {
        rect(buf, WIDTH / 3, HEIGHT - 1, 2 * WIDTH / 3, HEIGHT / 2, makecol(100,100,100));
        } 
        if (SOUND.state == 6) {
        rect(buf, 2 * WIDTH / 3, HEIGHT - 1, WIDTH, HEIGHT / 2, makecol(100,100,100));
        }                     

        pthread_mutex_unlock(&mxs);     
        

        // copy graphic buffer to avoid flickering
        blit(buf, screen, 0, 0, 0, 0, buf->w, buf->h);

        ptask_wait_for_period();       
    }
}

ptask   sound_task(void){

    int   x2, y2;   // storing dp values

    while (1) {

        pthread_mutex_lock(&mxdp);
        x2 = DOUBLE_PENDULUM.x2;
        y2 = DOUBLE_PENDULUM.y2;
        pthread_mutex_unlock(&mxdp);

        pthread_mutex_lock(&mxs);

        // sound mapping according to the second mass position on the screen
        if ((x2 < (int) WIDTH/3) && (y2 < (int) HEIGHT / 2)) {
            if (SOUND.state != 1) {
            SOUND.buffer = sfSoundBuffer_createFromFile("sounds/SOL.wav");
            sfSound_setBuffer(SOUND.SOL, SOUND.buffer);
            sfSound_play(SOUND.SOL);

            SOUND.state = 1;
            }
        }

        if ((x2 > (int) WIDTH / 3) && (x2 < (int) 2 * WIDTH / 3) && (y2 < (int) HEIGHT / 2)) {
            if (SOUND.state != 2) {
            SOUND.buffer = sfSoundBuffer_createFromFile("sounds/SIb.wav");
            sfSound_setBuffer(SOUND.SIb, SOUND.buffer);
            sfSound_play(SOUND.SIb);

            SOUND.state = 2;
            }
        }

        if ((x2 > (int) 2 * WIDTH / 3) && (y2 < (int) HEIGHT / 2)) {
            if (SOUND.state != 3) {
            SOUND.buffer = sfSoundBuffer_createFromFile("sounds/MIb.wav");
            sfSound_setBuffer(SOUND.MIb, SOUND.buffer);
            sfSound_play(SOUND.MIb);

            SOUND.state = 3;
            }
        }

        if ((x2 < (int) WIDTH / 3) && (y2 > (int) HEIGHT / 2)) {
            if (SOUND.state != 4) {
            SOUND.buffer = sfSoundBuffer_createFromFile("sounds/DO.wav");
            sfSound_setBuffer(SOUND.DO, SOUND.buffer);
            sfSound_play(SOUND.DO);

            SOUND.state = 4;
            }
        }

        if ((x2 > (int) WIDTH / 3) && (x2 < (int) 2 * WIDTH / 3) && (y2 > (int) HEIGHT / 2)) {
            if (SOUND.state != 5) {
            SOUND.buffer = sfSoundBuffer_createFromFile("sounds/FA.wav");
            sfSound_setBuffer(SOUND.FA, SOUND.buffer);
            sfSound_play(SOUND.FA);

            SOUND.state = 5;
            }
        }

        if ((x2 > (int) 2 * WIDTH / 3) && (y2 > (int) HEIGHT / 2)) {
            if (SOUND.state != 6) {
            SOUND.buffer = sfSoundBuffer_createFromFile("sounds/FAD.wav");
            sfSound_setBuffer(SOUND.FAD, SOUND.buffer);
            sfSound_play(SOUND.FAD);

            SOUND.state = 6;
            }
        }

        pthread_mutex_unlock(&mxs);

        ptask_wait_for_period();       
    }

    sfSound_destroy(SOUND.DO);
    sfSound_destroy(SOUND.FA);
    sfSound_destroy(SOUND.FAD);
    sfSound_destroy(SOUND.MIb);
    sfSound_destroy(SOUND.SIb);
    sfSound_destroy(SOUND.SOL);
}

//-------------------------------------------------------------
// FUNCTIONS DEFINITIONS
//-------------------------------------------------------------
void  init_global(void){
    printf("Starting global initialization\n");

    int   err;    // return value from ptask functions

    init_graphics();
    init_double_pendulum(&DOUBLE_PENDULUM);
    init_buffer(&BUFFER);
    init_sound(&SOUND);

    printf("Doing mutex dp init\n");
    pmux_create_pi(&mxdp);
    printf("Doing mutex buffer init\n");
    pmux_create_pi(&mxb);
    printf("Doing mutex sound init\n");
    pmux_create_pi(&mxs);

    printf("Doing ptask init\n");
    ptask_init(SCHED_FIFO, GLOBAL, NO_PROTOCOL);

    printf("Doing ptask create for dp\n");
    err = ptask_create_prio(double_pendulum_task, PERIOD, PRIORITY, ACT);
    printf("We've done ptask create with err = %d \n", err);

    printf("Doing ptask create for graphic_task\n");
    err = ptask_create_prio(graphic_task, PERIOD, PRIORITY, ACT);
    printf("We've done ptask create with err = %d \n", err);

    printf("Doing ptask create for sound_task\n");
    err = ptask_create_prio(sound_task, PERIOD, PRIORITY, ACT);
    printf("We've done ptask create with err = %d \n", err);
}

void  derivs(float in[], float din[]){
    float   den1, den2, del;

    din[0] = in[1]; 

    del = in[2]-in[0];
    den1 = (M1 + M2) * L1 - M2 * L1 * cos(del) * cos(del);
    din[1] = (M2 * L1 * in[1] * in[1] * sin(del) * cos(del)
    + M2 * G * sin(in[2]) * cos(del) + M2 * L2 * in[3] * in[3] * sin(del)
    - (M1 + M2) * G * sin(in[0])) / den1;

    din[2] = in[3];

    den2 = (L2 / L1) * den1;
    din[3] = (-M2 * L2 * in[3] * in[3] * sin(del) * cos(del)
    + (M1 + M2) * G * sin(in[0]) * cos(del) 
    - (M1 + M2) * L1 * in[1] * in[1] * sin(del)
    - (M1 + M2) * G * sin(in[2])) / den2;
}

void  runge_kutta(float in[], float out[], float h){
    int   i;
    float   din[N], dint[N], yt[N], k1[N], k2[N], k3[N], k4[N];   // Runge Kutta parameters

    // first step  
    derivs(in, din);
    for (i = 0; i < N; i++) {
        k1[i] = h * din[i];
        yt[i] = in[i] + 0.5 * k1[i];
    }

    // second step 
    derivs(yt, dint);
    for (i = 0; i < N; i++) {
        k2[i] = h * dint[i];
        yt[i] = in[i] + 0.5 * k2[i];
    }   

    // third step
    derivs(yt, dint);
    for (i = 0; i < N; i++) {
        k3[i] = h * dint[i];
        yt[i] = in[i] + k3[i];
    }

    // fourth step
    derivs(yt, dint);
    for (i = 0; i < N; i++) {
        k4[i] = h * dint[i];
        out[i] = in[i] + k1[i] / 6. + k2[i] / 3. + k3[i] / 3. + k4[i] / 6.;
    }
}

void  store_trail(struct buffer *b, int x2, int y2){
    int  k;

    k = b->top;

    k = (k + 1) % TLEN;

    b->x2[k] = x2;
    b->y2[k] = y2;

    b->top = k;
}

void draw_trail(void){
    int   i, j;     // trail indexes
    int   x2, y2;   // graphical coordinates extracted from the buffer
    int   r, g, b;  // RGB components

    for (i = 0; i < TLEN; i++) {
        j = (BUFFER.top + TLEN - i) % TLEN;
        x2 = BUFFER.x2[j];
        y2 = BUFFER.y2[j];

        r = g = b = 255 - 255 * i / TLEN;
        circlefill(buf, x2, y2, 4, makecol(r,g,b));
    }
}

void  init_sound(struct sound *s){
    printf("Initializing sound structure\n");

    s->DO = sfSound_create();
    s->FA = sfSound_create();
    s->FAD = sfSound_create();
    s->MIb = sfSound_create();
    s->SIb = sfSound_create();
    s->SOL = sfSound_create();
    s->state = 0;
}

void  init_graphics(void){
    printf("Initializing graphic components\n");

    allegro_init();
    install_keyboard();
    set_color_depth(32);
    set_gfx_mode(GFX_AUTODETECT_WINDOWED,WIDTH,HEIGHT,0,0);
    clear_to_color(screen, makecol(0,0,0));

    buf = create_bitmap(WIDTH, HEIGHT);
}

void  init_buffer(struct buffer *b){
    printf("Initializing buffer structure\n");

    int  k;

    b->top = 0;

    for (k = 0; k < TLEN; k++)  {
        b->x2[k] = 0;
        b->y2[k] = 0;
    }

    b->x2[0] = DOUBLE_PENDULUM.x2;
    b->y2[0] = DOUBLE_PENDULUM.y2;
}

void  init_double_pendulum(struct double_pendulum *dp){
    printf("Initializing double pendulum structure\n");

    dp->x0 = WIDTH / 2;
    dp->y0 = HEIGHT / 2;

    dp->th1 = TH1 * PI / 180.0;
    dp->th2 = TH2 * PI / 180.0;

    dp->w1 = W1 * PI / 180.0;
    dp->w2 = W2 * PI / 180.0;

    dp->x1 = dp->x0 + L1 * sin(dp->th1);
    dp->y1 = dp->y0 + L1 * cos(dp->th1);

    dp->x2 = dp->x1 + L2 * sin(dp->th2);
    dp->y2 = dp->y1 + L2 * cos(dp->th2);
}

int   main(void){
    printf("Starting main\n");

    init_global();

    while(!key[KEY_ESC]);

    allegro_exit();

    return 0;
}