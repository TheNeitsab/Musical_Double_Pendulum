//-------------------------------------------------------------
// To compile : gcc -Wall -Wextra -L./lib -I./inc src/MUSICAL_DOUBLE_PENDULUM_PEIGNE_GABRIELLI.c -o MUSICAL_DOUBLE_PENDULUM_PEIGNE_GABRIELLI -lptask -lpthread -lm `allegro-config --libs`
//-------------------------------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <sched.h>
#include <allegro.h>
#include <time.h>

#include "ptask.h"
#include "pmutex.h"
#include "tstat.h"


//-------------------------------------------------------------
// DOUBLE PENDULUM CONSTANTS
//-------------------------------------------------------------

#define PI 3.14159265
#define G 9.81          // acc'n due to gravity (in m/s^2) 

#define L1 100.0          // length of pendulum 1 (in m) 
#define L2 100.0          // length of pendulum 2 (in m) 
#define M1 10.0          // mass of pendulum 1 (in kg) 
#define M2 10.0          // mass of pendulum 2 (in kg)
#define TH1 180
#define TH2 179
#define W1 0
#define W2 0

#define N 4             // number of equations for RK4 algorithm   
#define TLEN 100         // trail length
#define TSCALE 10       // time scale factor

//-------------------------------------------------------------
// TASKS CONSTANTS
//-------------------------------------------------------------

#define PERIOD 20          // task period in ms
#define DREL 20             // relative deadline in ms
#define PRIORITY 80         // task priority
#define ACT 1   // activation flag

//-------------------------------------------------------------
// GRAPHICS CONSTANTS
//-------------------------------------------------------------

#define WIDTH 680   // window's width        
#define HEIGHT 480  // window's height 
#define BKG 0               // background color
#define TCOL 15             // trail color

//-------------------------------------------------------------
// GLOBAL VARIABLES
//-------------------------------------------------------------

pthread_mutex_t mxdp; // mutual esclusion semaphore for double pendulum
pthread_mutex_t mxb; // mutual esclusion semaphore for buffer

float t = 0;

//-------------------------------------------------------------
// GLOBAL DATA STRUCTURES
//-------------------------------------------------------------

struct double_pendulum {
    
    int x0, y0;             // coordinates of the fixed point  
    float x1, y1, x2, y2;   // coordinates of pendulums 1 & 2           
    float th1, th2;         // angles of pendulums 1 & 2
    float w1, w2;           // angular velocities of pendulums 1 & 2

}DOUBLE_PENDULUM;


struct buffer{

    int top;         // index of the current element
    int x2[TLEN];    // array of x2 coordinates
    int y2[TLEN];    // array of y2 coordinates

}BUFFER;

//-------------------------------------------------------------
// PROTOTYPES
//-------------------------------------------------------------

void init_double_pendulum(struct double_pendulum *dp);
void init_buffer(struct buffer *b);
void init_graphics(void);

void draw_trail(int past_values);
void store_trail(struct buffer *b);

void runge_kutta(float in[], float out[], float h);
void derivs(float in[], float din[]);

void init_global(void);

//-------------------------------------------------------------
// TASKS
//-------------------------------------------------------------

ptask double_pendulum_task(void){

    printf("Enterring the task !\n");
  
    float dt;                                 // integration interval
    dt = TSCALE*(float) PERIOD/1000;
    
    float in[N], out[N];
    int i;
    int j;

    // computing new parameters
    while (1) {

        pthread_mutex_lock(&mxdp);

        // filling first array with old values 
        in[0] = DOUBLE_PENDULUM.th1;
        in[1] = DOUBLE_PENDULUM.w1;
        in[2] = DOUBLE_PENDULUM.th2;
        in[3] = DOUBLE_PENDULUM.w2;
        /*
        printf("Old values => | th1 | w1 | th2 | w2 |\n|");
        for(j = 0; j < 4; j++){
          printf(" %f |",in[j]);
        }
        */
        //printf("OLD = x1 : %f | y1 : %f | x2 : %f | y2 : %f\n", DOUBLE_PENDULUM.x1, DOUBLE_PENDULUM.y1, DOUBLE_PENDULUM.x2, DOUBLE_PENDULUM.y2);
        runge_kutta(in, out, dt);

        // updating the old values
        DOUBLE_PENDULUM.th1 = out[0];
        DOUBLE_PENDULUM.w1 = out[1];
        DOUBLE_PENDULUM.th2 = out[2];
        DOUBLE_PENDULUM.w2 = out[3];
        DOUBLE_PENDULUM.x1 = DOUBLE_PENDULUM.x0 + L1*sin(DOUBLE_PENDULUM.th1);
        DOUBLE_PENDULUM.y1 = DOUBLE_PENDULUM.y0 + L1*cos(DOUBLE_PENDULUM.th1);
        DOUBLE_PENDULUM.x2 = DOUBLE_PENDULUM.x1 + L2*sin(DOUBLE_PENDULUM.th2);
        DOUBLE_PENDULUM.y2 = DOUBLE_PENDULUM.y1 + L2*cos(DOUBLE_PENDULUM.th2);

        t = dt*j;
        j++;
        printf("At t = %f => x1 : %f | y1 : %f | x2 : %f | y2 : %f\n", t, DOUBLE_PENDULUM.x1, DOUBLE_PENDULUM.y1, DOUBLE_PENDULUM.x2, DOUBLE_PENDULUM.y2);
        
        printf("New values => | th1 | w1 | th2 | w2 |\n|");
        for(i = 0; i < 4; i++){
          printf(" %f |",out[i]);
        }
        printf("\n");
        
        pthread_mutex_unlock(&mxdp);

        pthread_mutex_lock(&mxb);
        store_trail(&BUFFER);                   // storing the values in the trail
        pthread_mutex_unlock(&mxb);

        ptask_wait_for_period();
    }
}

ptask graphic_task(void){

    int k;
    //ptask_get_argument();

    while(1){
        //printf("Dans while du graphic_task\n");
        pthread_mutex_lock(&mxdp);
        clear_to_color(screen, makecol(0, 0, 0));

        circlefill(screen, DOUBLE_PENDULUM.x0, DOUBLE_PENDULUM.y0, 2, makecol(255, 255, 255));       
        circlefill(screen, DOUBLE_PENDULUM.x1, DOUBLE_PENDULUM.y1, 2, makecol(255, 255, 255));  
        circlefill(screen, DOUBLE_PENDULUM.x2, DOUBLE_PENDULUM.y2, 2, makecol(255, 255, 255));                       
        line(screen, DOUBLE_PENDULUM.x0, DOUBLE_PENDULUM.y0, DOUBLE_PENDULUM.x1, DOUBLE_PENDULUM.y1, makecol(255, 0, 0));                       
        line(screen, DOUBLE_PENDULUM.x1, DOUBLE_PENDULUM.y1, DOUBLE_PENDULUM.x2, DOUBLE_PENDULUM.y2, makecol(255, 0, 0));
        
        pthread_mutex_unlock(&mxdp);

        pthread_mutex_lock(&mxb);
        for(k = 0; k < TLEN; k++){
            draw_trail(k);
            //printf("Draw chemin\n");
        }
        pthread_mutex_unlock(&mxb);
        //blit(b, screen, 0, 0, 0, 0, b->w, b->h); // copy buffer on screen

        ptask_wait_for_period();
        //for(int j=0;j<5000000;j++);                // waiting loop        
    }
}


int main(void)
{
  printf("Starting main\n");

  init_global();

  while(!key[KEY_ESC]);

  allegro_exit();

  return 0;
}


//-------------------------------------------------------------
// FUNCTIONS
//-------------------------------------------------------------

void init_global(void){
  
  int err;

  printf("Starting global init\n");

  init_graphics();
  init_double_pendulum(&DOUBLE_PENDULUM);
  init_buffer(&BUFFER);

  printf("Doing mutex dp init\n");
  pmux_create_pi(&mxdp);
  printf("Doing mutex buffer init\n");
  pmux_create_pi(&mxb);

  printf("Doing ptask init\n");
  ptask_init(SCHED_FIFO, GLOBAL, NO_PROTOCOL);
  printf("Doing ptask create for dp\n");
  err = ptask_create_prio(double_pendulum_task, PERIOD, PRIORITY, ACT);
  printf("We've done ptask create with err = %d \n", err);
  printf("Doing ptask create for graphic_task\n");
  err = ptask_create_prio(graphic_task, PERIOD, PRIORITY, ACT);
  printf("We've done ptask create with err = %d \n", err);
}

void derivs(float in[], float din[]){
  // function to fill array of derivatives din at t

  float den1, den2, del;

  din[0] = in[1]; 
  
  del = in[2]-in[0];
  den1 = (M1+M2)*L1 - M2*L1*cos(del)*cos(del);
  din[1] = (M2*L1*in[1]*in[1]*sin(del)*cos(del)
    + M2*G*sin(in[2])*cos(del) + M2*L2*in[3]*in[3]*sin(del)
    - (M1+M2)*G*sin(in[0]))/den1;

  din[2] = in[3];

  den2 = (L2/L1)*den1;
  din[3] = (-M2*L2*in[3]*in[3]*sin(del)*cos(del)
    + (M1+M2)*G*sin(in[0])*cos(del) 
    - (M1+M2)*L1*in[1]*in[1]*sin(del)
    - (M1+M2)*G*sin(in[2]))/den2;

  return;

}

void runge_kutta(float in[], float out[], float h){
  // fourth order Runge-Kutta
 
  printf("Inside RK4 algorithm\n");
  int i;
  float din[N], dint[N], yt[N], k1[N], k2[N], k3[N], k4[N];
    
  
  derivs(in, din); // first step
  for (i = 0; i < N; i++) 
  {
    k1[i] = h*din[i];
    yt[i] = in[i] + 0.5*k1[i];
  }

  derivs(yt, dint); // second step 
  for (i = 0; i < N; i++)
  {
    k2[i] = h*dint[i];
    yt[i] = in[i] + 0.5*k2[i];
  }   

  derivs(yt, dint); // third step
  for (i = 0; i < N; i++)
  {
    k3[i] = h*dint[i];
    yt[i] = in[i] + k3[i];
  }

  derivs(yt, dint); // fourth step
  for (i = 0; i < N; i++)
  {
    k4[i] = h*dint[i];
    out[i] = in[i] + k1[i]/6. + k2[i]/3. + k3[i]/3. + k4[i]/6.;
  }
  
  return;
}

void store_trail(struct buffer *b){

    int k;

    k = b->top;

    //printf("Previous Values for k = %d : | x1 = %d | y1 = %d | x2 = %d | y2 = %d |\n", k, b->x1[k], b->y1[k], b->x2[k], b->y2[k]);

    k = (k + 1) % TLEN;

    //printf("Storing the trail in the buffer\n");
    b->x2[k] = (int) DOUBLE_PENDULUM.x2;
    b->y2[k] = (int) DOUBLE_PENDULUM.y2;

    //printf("New Values for k = %d : | x1 = %d | y1 = %d | x2 = %d | y2 = %d |\n", k, b->x1[k], b->y1[k], b->x2[k], b->y2[k]);

    b->top = k;
}

void draw_trail(int past_values){

    int i, j;                             // trail indexes
    int x2, y2;                   // x, y coordinates
    for(i = 0; i < past_values; i++){
        j = (BUFFER.top + TLEN - i)%TLEN;
        x2 = BUFFER.x2[j];
        y2 = BUFFER.y2[j];
        circlefill(screen, x2, y2, 2, makecol(255,0,255));     //voir pour changement de couleur du tracé
    }
}

void init_graphics(void){

    allegro_init();
    install_keyboard();
    set_color_depth(32);        // RGB mode 32 bufs
    set_gfx_mode(GFX_AUTODETECT_WINDOWED,WIDTH,HEIGHT,0,0);
    clear_to_color(screen, BKG);
    //printf("Init graphique\n");
}

void init_buffer(struct buffer *b){

    printf("Initializing buffer\n");
    b->top = 0;

    int k;
    for(k = 0; k < TLEN; k++){
      b->x2[k] = 0;
      b->y2[k] = 0;
    }

    b->x2[0] = (int) DOUBLE_PENDULUM.x2;
    b->y2[0] = (int) DOUBLE_PENDULUM.y2;
}

void init_double_pendulum(struct double_pendulum *dp){

  printf("Initializing double pendulum\n");
  dp->x0 = WIDTH/2;
  dp->y0 = HEIGHT/2;

  dp->th1 = TH1*PI/180.0;
  dp->th2 = TH2*PI/180.0;

  dp->w1 = W1*PI/180.0;
  dp->w2 = W2*PI/180.0;

  dp->x1 = dp->x0 + L1*sin(dp->th1);
  dp->y1 = dp->y0 + L1*cos(dp->th1);

  dp->x2 = dp->x1 + L2*sin(dp->th2);
  dp->y2 = dp->y1 + L2*cos(dp->th2);
}










