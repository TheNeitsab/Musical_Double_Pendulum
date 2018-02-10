#ifndef FUNCTIONS_H
#define FUNCTIONS_H

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
// description : initializes the double pendulum structure
// parameters  : struct double_pendulum *dp 
//               double pendulum structure 
void  init_buffer(struct buffer *b);
// description : initializes the buffer structure storing  
//               the trail of the second mass
// parameters  : struct buffer *b
//               graphical buffer structure       
void  init_graphics(void);
// description : initializes everything related to the 
//               graphical management
// parameters  : none
void  init_sound(struct sound *s);
// description : initializes the sound structure
// parameters  : struct sound *s
//               sound structure
void  init_global(void);
// description : makes the global initialization thanks
//               to the previous functions
// parameters  : none

void  draw_trail(void);
// description : draws the trail of the second mass
// parameters  : none
void  store_trail(struct buffer *b, int x2, int y2);
// description : stores the trail of the second mass
//               in the dedicated buffer
// parameters  : struct buffer *b 
//               graphical buffer structure

//               int x2
//               x coordinate of the second mass

//               int y2
//               y coordinate of the second mass

void  runge_kutta(float in[], float out[], float h);
// description : executes the runge kutta algorithm to
//               update the double pendulum parameters
// parameters  : float in[] 
//               array of the old angles and angular
//               velocities values 

//               float out[] 
//               array of the new angles and angular
//               velocities values 

//               float h
//               integration interval
void  derivs(float in[], float din[]);
// description : computes the derivative of the input
//               parameters
// parameters  : float in[] 
//               array of values to derivate

//               float din[]
//               array of derivated values
#endif