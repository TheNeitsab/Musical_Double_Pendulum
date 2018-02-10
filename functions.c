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
#include "inc/functions.h"

//-------------------------------------------------------------
// TASKS DEFINITIONS
//-------------------------------------------------------------

// task managing the double pendulum state
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

// task managing the graphical part
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

// task managing the sound part
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