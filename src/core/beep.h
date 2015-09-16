// This file is part of CNCSVision, a computer vision related library
// This software is developed under the grant of Italian Institute of Technology
//
// Copyright (C) 2010-2014 Carlo Nicolini <carlo.nicolini@iit.it>
//
//
// CNCSVision is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.
//
// Alternatively, you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// CNCSVision is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License or the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License and a copy of the GNU General Public License along with
// CNCSVision. If not, see <http://www.gnu.org/licenses/>.

/*  beep - just what it sounds like, makes the console beep - but with
 * precision control.  See the man page for details.
 *
 * Try beep -h for command line args
 *
 * This code is copyright (C) Johnathan Nightingale, 2000.
 *
 * This code may distributed only under the terms of the GNU Public License 
 * which can be found at http://www.gnu.org/copyleft or in the file COPYING 
 * supplied with this code.
 *
 * This code is not distributed with warranties of any kind, including implied
 * warranties of merchantability or fitness for a particular use or ability to 
 * breed pandas in captivity, it just can't be done.
 *
 * Bug me, I like it:  http://johnath.com/  or johnath@johnath.com
 */
#ifndef __BEEP_H_
#define __BEEP_H_

#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <linux/kd.h>
#include <linux/input.h>

/* I don't know where this number comes from, I admit that freely.  A 
   wonderful human named Raine M. Ekman used it in a program that played
   a tune at the console, and apparently, it's how the kernel likes its
   sound requests to be phrased.  If you see Raine, thank him for me.  
   
   June 28, email from Peter Tirsek (peter at tirsek dot com):
   
   This number represents the fixed frequency of the original PC XT's
   timer chip (the 8254 AFAIR), which is approximately 1.193 MHz. This
   number is divided with the desired frequency to obtain a counter value,
   that is subsequently fed into the timer chip, tied to the PC speaker.
   The chip decreases this counter at every tick (1.193 MHz) and when it
   reaches zero, it toggles the state of the speaker (on/off, or in/out),
   resets the counter to the original value, and starts over. The end
   result of this is a tone at approximately the desired frequency. :)
*/
#ifndef CLOCK_TICK_RATE
#define CLOCK_TICK_RATE 1193180
#endif

#define VERSION_STRING "beep-1.3"
char *copyright = 
"Copyright (C) Johnathan Nightingale, 2002.  "
"Use and Distribution subject to GPL.  "
"For information: http://www.gnu.org/copyleft/.";

/* Meaningful Defaults */
#define DEFAULT_FREQ       440.0 /* Middle A */
#define DEFAULT_LENGTH     200   /* milliseconds */
#define DEFAULT_REPS       1
#define DEFAULT_DELAY      100   /* milliseconds */
#define DEFAULT_END_DELAY  NO_END_DELAY
#define DEFAULT_STDIN_BEEP NO_STDIN_BEEP

/* Other Constants */
#define NO_END_DELAY    0
#define YES_END_DELAY   1

#define NO_STDIN_BEEP   0
#define LINE_STDIN_BEEP 1
#define CHAR_STDIN_BEEP 2

typedef struct beep_parms_t {
  float freq;     /* tone frequency (Hz)      */
  int length;     /* tone length    (ms)      */
  int reps;       /* # of repetitions         */
  int delay;      /* delay between reps  (ms) */
  int end_delay;  /* do we delay after last rep? */
  int stdin_beep; /* are we using stdin triggers?  We have three options:
		     - just beep and terminate (default)
		     - beep after a line of input
		     - beep after a character of input
		     In the latter two cases, pass the text back out again,
		     so that beep can be tucked appropriately into a text-
		     processing pipe.
		  */
  int verbose;    /* verbose output?          */
  struct beep_parms_t *next;  /* in case -n/--new is used. */
} beep_parms_t;

enum { BEEP_TYPE_CONSOLE, BEEP_TYPE_EVDEV };

/* Momma taught me never to use globals, but we need something the signal 
   handlers can get at.*/
int console_fd = -1;
int console_type = BEEP_TYPE_CONSOLE;
char *console_device = NULL;


void do_beep(int freq) {
  if (console_type == BEEP_TYPE_CONSOLE) {
    if(ioctl(console_fd, KIOCSOUND, freq != 0
      ? (int)(CLOCK_TICK_RATE/freq)
      : freq) < 0) {
      printf("\a");  /* Output the only beep we can, in an effort to fall back on usefulness */
      perror("ioctl");
    }
  } else {
     /* BEEP_TYPE_EVDEV */
     struct input_event e;

     e.type = EV_SND;
     e.code = SND_TONE;
     e.value = freq;

     write(console_fd, &e, sizeof(struct input_event));
  }
}


/* If we get interrupted, it would be nice to not leave the speaker beeping in
   perpetuity. */
void handle_signal(int signum) {

  if(console_device)
    free(console_device);

  switch(signum) {
  case SIGINT:
    if(console_fd >= 0) {
      /* Kill the sound, quit gracefully */
      do_beep(0);
      close(console_fd);
      exit(signum);
    } else {
      /* Just quit gracefully */
      exit(signum);
    }
  }
}

/* print usage and exit */
void usage_bail(const char *executable_name) {
  printf("Usage:\n%s [-f freq] [-l length] [-r reps] [-d delay] "
	 "[-D delay] [-s] [-c] [--verbose | --debug] [-e device]\n",
	 executable_name);
  printf("%s [Options...] [-n] [--new] [Options...] ... \n", executable_name);
  printf("%s [-h] [--help]\n", executable_name);
  printf("%s [-v] [-V] [--version]\n", executable_name);
  exit(1);
}


/* Parse the command line.  argv should be untampered, as passed to main.
 * Beep parameters returned in result, subsequent parameters in argv will over-
 * ride previous ones.
 * 
 * Currently valid parameters:
 *  "-f <frequency in Hz>"
 *  "-l <tone length in ms>"
 *  "-r <repetitions>"
 *  "-d <delay in ms>"
 *  "-D <delay in ms>" (similar to -d, but delay after last repetition as well)
 *  "-s" (beep after each line of input from stdin, echo line to stdout)
 *  "-c" (beep after each char of input from stdin, echo char to stdout)
 *  "--verbose/--debug"
 *  "-h/--help"
 *  "-v/-V/--version"
 *  "-n/--new"
 *
 * March 29, 2002 - Daniel Eisenbud points out that c should be int, not char,
 * for correctness on platforms with unsigned chars.
 */
void parse_command_line(int argc, char **argv, beep_parms_t *result) {
  int c;

  struct option opt_list[7] = {{"help", 0, NULL, 'h'},
			       {"version", 0, NULL, 'V'},
			       {"new", 0, NULL, 'n'},
			       {"verbose", 0, NULL, 'X'},
			       {"debug", 0, NULL, 'X'},
			       {"device", 1, NULL, 'e'},
			       {0,0,0,0}};
  while((c = getopt_long(argc, argv, "f:l:r:d:D:schvVne:", opt_list, NULL))
	!= EOF) {
    int argval = -1;    /* handle parsed numbers for various arguments */
    float argfreq = -1; 
    switch(c) {      
    case 'f':  /* freq */
      if(!sscanf(optarg, "%f", &argfreq) || (argfreq >= 20000 /* ack! */) || 
	 (argfreq <= 0))
	usage_bail(argv[0]);
      else
	if (result->freq != 0)
	  fprintf(stderr, "WARNING: multiple -f values given, only last "
	    "one is used.\n");
	result->freq = argfreq;    
      break;
    case 'l' : /* length */
      if(!sscanf(optarg, "%d", &argval) || (argval < 0))
	usage_bail(argv[0]);
      else
	result->length = argval;
      break;
    case 'r' : /* repetitions */
      if(!sscanf(optarg, "%d", &argval) || (argval < 0))
	usage_bail(argv[0]);
      else
	result->reps = argval;
      break;
    case 'd' : /* delay between reps - WITHOUT delay after last beep*/
      if(!sscanf(optarg, "%d", &argval) || (argval < 0))
	usage_bail(argv[0]);
      else {
	result->delay = argval;
	result->end_delay = NO_END_DELAY;
      }
      break;
    case 'D' : /* delay between reps - WITH delay after last beep */
      if(!sscanf(optarg, "%d", &argval) || (argval < 0))
	usage_bail(argv[0]);
      else {
	result->delay = argval;
	result->end_delay = YES_END_DELAY;
      }
      break;
    case 's' :
      result->stdin_beep = LINE_STDIN_BEEP;
      break;
    case 'c' :
      result->stdin_beep = CHAR_STDIN_BEEP;
      break;
    case 'v' :
    case 'V' : /* also --version */
      printf("%s\n",VERSION_STRING);
      exit(0);
      break;
    case 'n' : /* also --new - create another beep */
      if (result->freq == 0)
	result->freq = DEFAULT_FREQ;
      result->next = (beep_parms_t *)malloc(sizeof(beep_parms_t));
      result->next->freq       = 0;
      result->next->length     = DEFAULT_LENGTH;
      result->next->reps       = DEFAULT_REPS;
      result->next->delay      = DEFAULT_DELAY;
      result->next->end_delay  = DEFAULT_END_DELAY;
      result->next->stdin_beep = DEFAULT_STDIN_BEEP;
      result->next->verbose    = result->verbose;
      result->next->next       = NULL;
      result = result->next; /* yes, I meant to do that. */
      break;
    case 'X' : /* --debug / --verbose */
      result->verbose = 1;
      break;
    case 'e' : /* also --device */
      console_device = strdup(optarg);
      break;
    case 'h' : /* notice that this is also --help */
    default :
      usage_bail(argv[0]);
    }
  }
  if (result->freq == 0)
    result->freq = DEFAULT_FREQ;
}  

void play_beep(beep_parms_t parms) {
  int i; /* loop counter */

  if(parms.verbose == 1)
      fprintf(stderr, "[DEBUG] %d times %d ms beeps (%d delay between, "
	"%d delay after) @ %.2f Hz\n",
	parms.reps, parms.length, parms.delay, parms.end_delay, parms.freq);

  /* try to snag the console */
  if(console_device)
    console_fd = open(console_device, O_WRONLY);
  else
    if((console_fd = open("/dev/tty0", O_WRONLY)) == -1)
      console_fd = open("/dev/vc/0", O_WRONLY);

  if(console_fd == -1) {
    fprintf(stderr, "Could not open %s for writing\n",
      console_device != NULL ? console_device : "/dev/tty0 or /dev/vc/0");
    printf("\a");  /* Output the only beep we can, in an effort to fall back on usefulness */
    perror("open");
    exit(1);
  }

  if (ioctl(console_fd, EVIOCGSND(0)) != -1)
    console_type = BEEP_TYPE_EVDEV;
  else
    console_type = BEEP_TYPE_CONSOLE;
  
  /* Beep */
  for (i = 0; i < parms.reps; i++) {                    /* start beep */
    do_beep(parms.freq);
    /* Look ma, I'm not ansi C compatible! */
    usleep(1000*parms.length);                          /* wait...    */
    do_beep(0);                                         /* stop beep  */
    if(parms.end_delay || (i+1 < parms.reps))
       usleep(1000*parms.delay);                        /* wait...    */
  }                                                     /* repeat.    */

  close(console_fd);
}

/*
    If it does not beep try the following:
    1) Open a terminal and install beep

    $> sudo apt-get install beep

    and then try the beep:

    $> beep

    2) if still it does not beep
    un-blacklist the pc speaker:

    $> sudo vim /etc/modprobe.d/blacklist.conf
    and then comment the line

    #blacklist pcspkr

    If it does not beep it is because of permissions to tty0, try run the program with sudo.
    sudo ./program_which_beeps
    Otherwise put yourself in audio group

    and

    sudo chmod 4755 /usr/bin/beep

*/
void Beep(int freq, int lenght_ms)
{
    beep_parms_t *parms = (beep_parms_t *)malloc(sizeof(beep_parms_t));
    parms->freq       = freq;
    parms->length     = lenght_ms;
    parms->reps       = DEFAULT_REPS;
    parms->delay      = DEFAULT_DELAY;
    parms->end_delay  = DEFAULT_END_DELAY;
    parms->stdin_beep = DEFAULT_STDIN_BEEP;
    parms->verbose    = 0;
    parms->next       = NULL;
    play_beep(*parms);
    free(parms);
}

#endif