#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "allegro.h"


#define PI 3.14159265   
#define RADIUS 5        // Radius of the circles (masses)
#define N 1000          // Number of values from the text file
#define WIDTH 680       
#define HEIGHT 480

int main()
{
    /* Variables */
    float x0, y0;                               // Fixed center coordinates 
    float x1[N], y1[N], x2[N], y2[N];           // Mass 1 & 2 coordinates 
    float l1, l2;                               // Bar 1 & 2 lenghts            
    float th1[N], th2[N];                       // Mass 1 & 2 angles
    float w1[N], w2[N];                         // Mass 1 & 2 omegas
    float t[N];                                 // Time t in sec
    float map[WIDTH][HEIGHT];                   // Saturation map => map[x][y] = 0 to 1             


    /* Variables' initialization */
    x0 = WIDTH/2;
    y0 = HEIGHT/3;

    l1 = l2 = 100;

    for(int w = 0; w < WIDTH; w++){
        for(int h = 0; h < HEIGHT; h++){
            map[w][h] = 0;
        }
    }


    /* Processing results' file */
    FILE* fichier = NULL;
    fichier = fopen("outfile.txt", "r");

    if (fichier != NULL)
    { 
        int i = 0;

        while(i < N){ 

            fscanf(fichier, "%f %f %f %f %f ", &t[i], &th1[i], &w1[i], &th2[i], &w2[i]);
            printf("On a t = %f, th1 = %f, w1 = %f, th2 = %f, w2 = %f\n", t[i], th1[i], w1[i], th2[i], w2[i]);
            
            x1[i] = x0 + l1*sin(th1[i]);
            y1[i] = y0 + l1*cos(th1[i]);
            
            x2[i] = x1[i] + l2*sin(th2[i]);
            y2[i] = y1[i] + l2*cos(th2[i]);

            i++;
        }

        fclose(fichier);
    }

    /* Allegro's intitialization */
    allegro_init();
    install_keyboard();
    set_color_depth(32);                                         // RGB mode (32 bits)  
    set_gfx_mode(GFX_AUTODETECT_WINDOWED,WIDTH,HEIGHT,0,0);
    clear_to_color(screen, makecol(0, 0, 0));                                  // Clear to Black background
    
    BITMAP *buf;                                                // Buffer used to avoid flickering on screen
    buf = create_bitmap(WIDTH, HEIGHT);


    int k = 0;

    while((k < N) && (!key[KEY_ESC])){                          // Running till ESC is pressed or N is reached

        int step = k;
        int x;
        int y;
        int r, g, b;        // RGB components
        float h, s, v;      // HSV components

        clear_to_color(buf, makecol(0, 0, 0));                                 // Clear to Black background

        /* Double pendulum graphics */
        for(int a = 0; a < k; a++){
            x = x2[a];
            y = y2[a];
            h = 120;
            s = 1;
            v = sqrt(pow(l1*w1[a]*cos(th1[a]) + l2*w2[a]*cos(th2[a]),2) + pow(l1*w1[a]*sin(th1[a]) + l2*w2[a]*sin(th2[a]),2));   
            hsv_to_rgb(h, s, v, &r, &g, &b);

            circlefill(buf, x, y, RADIUS, makecol(r, g, b));
        }
        /*
        int color = getpixel(buf, x, y);
        r = getr(color);
        g = getg(color);
        b = getb(color);
        rgb_to_hsv(r, g, b, &h, &s, &v);

        if(v < 1){
            map[x][y] = v + 0.1;
        }
        else {
            map[x][y] = 1;
        }

        hsv_to_rgb(h, s, map[x][y], &r, &g, &b);
        circlefill(buf, x, y, RADIUS, makecol(r, g, b));
        */
        line(buf, x0, y0, x1[step], y1[step], makecol(255, 0, 0)); 
        line(buf, x1[step], y1[step], x2[step], y2[step], makecol(255, 0, 0));
        circlefill(buf, x0, y0, RADIUS+2, makecol(0, 0, 0));
        circlefill(buf, x0, y0, RADIUS, makecol(255, 255, 255));
        circlefill(buf, x1[step], y1[step], RADIUS+2, makecol(0, 0, 0));
        circlefill(buf, x1[step], y1[step], RADIUS, makecol(255, 255, 255));
        circlefill(buf, x2[step], y2[step], RADIUS+2, makecol(0, 0, 0));
        circlefill(buf, x2[step], y2[step], RADIUS, makecol(255, 255, 255));
        
        for(int j=0;j<5000000;j++);                             // Waiting loop

        blit(buf, screen, 0, 0, 0, 0, buf->w, buf->h);           // Copy buffer on screen

        k++;

    }

    allegro_exit();
    
    return 0;
}

