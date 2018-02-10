//-------------------------------------------------------------------
//                      MUSICAL DOUBLE PENDULUM
//                  GABRIELLI Bastien & PEIGNE Aurore
//-------------------------------------------------------------------
// This program implements a musical double pendulum using real time 
// tasks. You  can  find  all  the  parameters related to the double 
// pendulum  in  the "functions.h". You can freely change the masses, 
// the  angles  and  the  angular  velocities defined in the file in  
// order to get antoher chaotic and random behavior and music.
// Nonetheless,  since  the  graphic scaling as not been implemented
// yet, you might not change too much the values of L1 and L2.

// First you need to have allegro4 and csfml librairies installed :
// sudo apt-get install libcsfml-dev
// sudo apt-get install liballegro4-dev

// Then, you can use the dedicated makefile to compile the project
// while being in the project general folder.

// Enjoy ! :-)
//-------------------------------------------------------------------

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

int   main(void){
    printf("Starting main\n");

    init_global();

    while(!key[KEY_ESC]);

    allegro_exit();

    return 0;
}