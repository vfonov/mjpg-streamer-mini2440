/*******************************************************************************
# s3c2410 streaming input-plugin for MJPG-streamer                             #
#                                                                              #
# This plugin is intended to work with s3c2410 embeded device                  #
#                                                                              #
# Copyright (C) 2007 Tom Stöveken                                              #
#                                                                              #
# This program is free software; you can redistribute it and/or modify         #
# it under the terms of the GNU General Public License as published by         #
# the Free Software Foundation; either version 2 of the License, or            #
# (at your option) any later version.                                          #
#                                                                              #
# This program is distributed in the hope that it will be useful,              #
# but WITHOUT ANY WARRANTY; without even the implied warranty of               #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                #
# GNU General Public License for more details.                                 #
#                                                                              #
# You should have received a copy of the GNU General Public License            #
# along with this program; if not, write to the Free Software                  #
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA    #
#                                                                              #
*******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <getopt.h>
#include <pthread.h>

#include "s3c2410.h"
#include "utils.h"

#include "../../utils.h"
#include "../../mjpg_streamer.h"

#define INPUT_PLUGIN_NAME "S3C2410 embedded camera"
#define MAX_ARGUMENTS 32


/* private functions and variables to this plugin */
pthread_t cam;
struct vdIn *videoIn;

static globals *pglobal;

void *cam_thread( void *);
void cam_cleanup(void *);
void help(void);

/*** plugin interface functions ***/
/******************************************************************************
Description.: This function initializes the plugin. It parses the commandline-
              parameter and stores the default and parsed values in the
              appropriate variables.
Input Value.: param contains among others the command-line string
Return Value: 0 if everything is fine
              1 if "--help" was triggered, in this case the calling programm
              should stop running and leave.
******************************************************************************/
int input_init(input_parameter *param) {
  char  *dev = "/dev/camera", *s;
  int width=640, height=512,  i;

  /* keep a pointer to the global variables */
  pglobal = param->global;

  /* allocate webcam datastructure */
  videoIn = malloc(sizeof(struct vdIn));
  if ( videoIn == NULL ) 
  {
    IPRINT("not enough memory for videoIn\n");
    exit(EXIT_FAILURE);
  }
  
  memset(videoIn, 0, sizeof(struct vdIn));
  DBG("initializing s3c2410 device\n");
  
  /* open video device and prepare data structure */
  if (init_s3c2410 (videoIn, dev, width, height) != 0) 
  {
    IPRINT("init_s3c2410 failed\n");
    closelog();
    exit(EXIT_FAILURE);
  }
  return 0;
}

/******************************************************************************
Description.: Stops the execution of worker thread
Input Value.: -
Return Value: always 0
******************************************************************************/
int input_stop(void) {
  DBG("will cancel input thread\n");
  pthread_cancel(cam);

  return 0;
}

/******************************************************************************
Description.: spins of a worker thread
Input Value.: -
Return Value: always 0
******************************************************************************/
int input_run(void) 
{
  pglobal->buf = malloc(videoIn->framesizeIn);

  if (pglobal->buf == NULL) 
  {
    fprintf(stderr, "could not allocate memory\n");
    exit(EXIT_FAILURE);
  }

  pthread_create(&cam, 0, cam_thread, NULL);
  pthread_detach(cam);

  return 0;
}

/******************************************************************************
Description.: process commands, allows to set certain runtime configurations
              and settings like pan/tilt, colors, saturation etc.
Input Value.: * cmd specifies the command, a complete list is maintained in
                the file "input.h"
              * value is used for commands that make use of a parameter.
Return Value: depends in the command, for most cases 0 means no errors and
              -1 signals an error. This is just rule of thumb, not more!
******************************************************************************/
int input_cmd(in_cmd_type cmd, int value) {
  int res=0;

  return res;
}

/*** private functions for this plugin below ***/
/******************************************************************************
Description.: print a help message to stderr
Input Value.: -
Return Value: -
******************************************************************************/
void help(void) {
  int i;

  fprintf(stderr, " ---------------------------------------------------------------\n" \
                  " Help for input plugin..: "INPUT_PLUGIN_NAME"\n" \
                  " ---------------------------------------------------------------\n" \
                  "                          ");
  fprintf(stderr, " ---------------------------------------------------------------\n\n");
}

/******************************************************************************
Description.: this thread worker grabs a frame and copies it to the global buffer
Input Value.: unused
Return Value: unused, always NULL
******************************************************************************/
void *cam_thread( void *arg ) 
{
  int iframe = 0;
  unsigned char *pictureData = NULL;
  struct frame_t *headerframe;

  /* set cleanup handler to cleanup allocated ressources */
  pthread_cleanup_push(cam_cleanup, NULL);

  while( !pglobal->stop ) 
  {
    
    /* grab a frame */
    
    if( s3c2410_Grab( videoIn ) < 0 ) {
      IPRINT("Error grabbing frames\n");
      exit(EXIT_FAILURE);
    }

    iframe=(videoIn->frame_cour +(OUTFRMNUMB-1))% OUTFRMNUMB;
    videoIn->framelock[iframe]++;
    
    headerframe=(struct frame_t*)videoIn->ptframe[iframe];
    pictureData = videoIn->ptframe[iframe]+sizeof(struct frame_t);
    
    videoIn->framelock[iframe]--;

    /* copy JPG picture to global buffer */
    pthread_mutex_lock( &pglobal->db );

    pglobal->size = get_jpegsize(pictureData, headerframe->size);
    memcpy(pglobal->buf, pictureData, pglobal->size);

    /* signal fresh_frame */
    pthread_cond_broadcast(&pglobal->db_update);
    pthread_mutex_unlock( &pglobal->db );
  }

  DBG("leaving input thread, calling cleanup function now\n");
  pthread_cleanup_pop(1);

  return NULL;
}

/******************************************************************************
Description.: 
Input Value.: 
Return Value: 
******************************************************************************/
void cam_cleanup(void *arg) 
{
  static unsigned char first_run=1;

  if ( !first_run ) {
    DBG("already cleaned up ressources\n");
    return;
  }

  first_run = 0;
  IPRINT("cleaning up resources allocated by input thread\n");

  close_s3c2410(videoIn);
  //if (videoIn->tmpbuffer != NULL) free(videoIn->tmpbuffer);
  if (videoIn != NULL) free(videoIn);
  if (pglobal->buf != NULL) free(pglobal->buf);
}

