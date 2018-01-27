#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "allegro.h"

#define GRAVITY 9.8
#define PI 3.14159265
#define RADIUS 5                                               // radius


int main()
{
    /* variables */
    float x0, y0, x1, y1, x2, y2;                              // coordinates 
    float l1, l2;                                              // lenghts            
    float th1, th2;                                            // angles
    float w1, w2;                                              // omegas
    float t;                                                   // time

    x0 = y0 = x1 = y1 = x2 = y2 = l1 = l2 = th1 = th2 = w1 = w2 = t = 0;

    /* openning results' file */
    FILE* fichier = NULL;
    fichier = fopen("outfile.txt", "r");                    
    
    /* initialization */
    if (fichier != NULL)
    {
        fscanf(fichier, "%f %f %f %f %f\n", &t, &th1, &w1, &th2, &w2);
        printf("We get : t = %f, th1 = %f, w1 = %f, th2 = %f, w2 = %f\n", t, th1, w1, th2, w2);
        fclose(fichier);
    }
    else{ printf("ERROR while trying to read the file\n"); }

    x0 = 319;
    y0 = 100;

    l1 = l2 = 100;

    x1 = x0 + l1*sin(th1);
    y1 = y0 + l1*cos(th1);

    x2 = x1 + l2*sin(th2);
    y2 = y1 + l2*cos(th2);
    
    /* Graphic's part */
    allegro_init();
    install_keyboard();
    set_color_depth(8);                                     // VGA mode (8 bits)
    set_gfx_mode(GFX_AUTODETECT_WINDOWED,640,480,0,0);  

    clear_to_color(screen, 0);                              // black background

    line(screen, x0, y0, x1, y1, 4);   
    line(screen, x1, y1, x2, y2, 4);
    circlefill(screen, x0, y0, RADIUS, 7);
    circlefill(screen, x1, y1, RADIUS, 7);
    circlefill(screen, x2, y2, RADIUS, 7);

    readkey();                                              // wait for any key

    allegro_exit();
    
    return 0;
}

