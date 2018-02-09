#include <stdlib.h>
#include <stdio.h>
#include <allegro.h>

#define WIDTH 680       
#define HEIGHT 480

int main(int argc, char const *argv[])
{
	int sound_return = 4;
    SAMPLE * sample;

// Allegro's initialization
	allegro_init();
    install_keyboard();
    set_color_depth(32);                                         
    set_gfx_mode(GFX_AUTODETECT_WINDOWED,WIDTH,HEIGHT,0,0);
    clear_to_color(screen, makecol(0, 0, 255));  
    set_window_title("Sound");

// Sound's installation
	sound_return = install_sound(DIGI_AUTODETECT, MIDI_NONE, 0);
	printf("Sound_return = %d\n", sound_return); // check if it returns 0 => sound successfully installed

//Loading sample
	sample = load_sample("DO.wav");
	if(sample == NULL){
		printf("ERROR ON LOADING WAVE FILE\n");
		exit(1);
	}
	else{
		printf("Sample load\n");
	}
//PLaying sample
	play_sample(sample, 255, 128, 1000, FALSE);
	printf("Playing sound\n");

	while(!key[KEY_ESC]);

//Stop playing sample and exit allegro
	stop_sample(sample);
	printf("Stop sound\n");

    allegro_exit();
    printf("Exit of Allegro\n");

	return 0;
}