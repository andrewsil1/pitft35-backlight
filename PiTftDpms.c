/*

  PiTftDPMS.c
  ========
  (c) Copyright Andrew Silverman 2018
  Email: andrewsi@outlook.com

  DPMS backlight synchronizer for PiTFT 3.5" resistive displays

*/


#include <X11/Xlib.h>
#include <X11/extensions/dpms.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>

/*  Global variables  */

Display *     display;
static char * appname;

/*  main() function  */

int main( int argc, char * argv[] ) {

    /*  Display variables  */

    char *       display_name = NULL;
 
    appname = argv[0];

    /*  Connect to X server  */

    if ( (display = XOpenDisplay(display_name)) == NULL ) {
	fprintf(stderr, "%s: couldn't connect to X server %s\n",
		appname, display_name);
	exit(EXIT_FAILURE);
    }

int dpmsEvent;
int dpmsError;

    /* Make sure we can talk to the DPMS X extension */
    if (!DPMSQueryExtension(display, &dpmsEvent, &dpmsError)) {
        fprintf(stderr,"%s: X claims it's doesn't support the DPMS extension!", appname);
        exit(EXIT_FAILURE);
    }
    
    /*  Enter event loop  */

    CARD16 power_level;
    CARD16 old_power_level;
    BOOL state;
    
    /* Initialize our current understanding of the display power state. */
    if (!DPMSInfo(display, &power_level, &state)) {
      exit(EXIT_FAILURE);
    }
    old_power_level = power_level;
        
    while ( 1 ) {
        
        if (!DPMSInfo(display, &power_level, &state)) {
          exit(EXIT_FAILURE);
        } else {
            /*  Only update power state if X11 DPMS extension indicates that there has been a change in state. */
            if (power_level != old_power_level) {
                old_power_level = power_level;
                int fd = open("/sys/class/backlight/soc:backlight/brightness", O_WRONLY);
                if (fd == -1) {
                    fprintf(stderr, "Couldn't open backlight device file\n", appname);
                    exit(EXIT_FAILURE);
                }
                if (power_level != DPMSModeOn)
                    write(fd, "0\n", 2);
                else
                    write(fd, "1\n", 2);
                close(fd);
            }
        }
        usleep(250000); /* Check every 0.25 seconds. */
    }
    return EXIT_SUCCESS;   /*  We shouldn't get here  */
}
