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
#include <syslog.h>

/*  Global variables  */

Display *     display;
static char * appname;

void logmsg(int level, char * msg) {
	openlog(appname, LOG_ODELAY, LOG_USER);
	syslog(level, msg);
	closelog();
}

/*  main() function  */

int main( int argc, char * argv[] ) {

    /*  Display variables  */

    char *       display_name = NULL;
    char * 	 msg = NULL; 

    msg = malloc(sizeof(char) * 100);
 
    appname = argv[0];

    snprintf(msg, 100, "Starting PiTftDpms display backlight utility.\n");
    logmsg(LOG_INFO,msg);

    /*  Connect to X server  */

    if ((display = XOpenDisplay(display_name)) == NULL ) {
        snprintf(msg, 100, "Couldn't connect to X server %s\n", display_name);
        logmsg(LOG_ERR,msg);
        exit(EXIT_FAILURE);
    }

int dpmsEvent;
int dpmsError;

    /* Make sure we can talk to the DPMS X extension */
    if (!DPMSQueryExtension(display, &dpmsEvent, &dpmsError)) {
        snprintf(msg, 100, "X claims it doesn't support the DPMS extension!\n", display_name);
        logmsg(LOG_ERR,msg);
        exit(EXIT_FAILURE);
    }
    
    /*  Enter event loop  */

    CARD16 power_level;
    CARD16 old_power_level;
    BOOL state;
    
    /* Initialize our current understanding of the display power state. */
    if (!DPMSInfo(display, &power_level, &state)) {
      snprintf(msg, 100, "Unable to read the display power state, exiting.\n");
      logmsg(LOG_ERR,msg);
      exit(EXIT_FAILURE);
    }
    old_power_level = power_level;
        
    while ( 1 ) {
        
        /*  Only update power state if X11 DPMS extension indicates that there has been a change in state. */
        if (power_level != old_power_level) {
            old_power_level = power_level;
            int fd = open("/sys/class/backlight/soc:backlight/brightness", O_WRONLY);
            if (fd == -1) {
                snprintf(msg, 100, "Couldn't open backlight device file. Need sudo?\n");
                logmsg(LOG_ERR,msg);
                exit(EXIT_FAILURE);
            }
            if (power_level != DPMSModeOn)
                write(fd, "0\n", 2);
            else
                write(fd, "1\n", 2);
            close(fd);
        }
    
        usleep(250000); /* Check every 0.25 seconds. */
        
        /* Update the power state after sleeping. */
        if (!DPMSInfo(display, &power_level, &state)) {
            snprintf(msg, 100, "Unable to read the display power state, exiting.\n");
            logmsg(LOG_ERR,msg);
            exit(EXIT_FAILURE);
        }
    } /* End While */
    return EXIT_SUCCESS;   /*  We shouldn't ever get here  */
}
