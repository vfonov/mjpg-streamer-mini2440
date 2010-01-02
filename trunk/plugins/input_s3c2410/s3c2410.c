/*******************************************************************************
# mini2410 SOC  streaming input-plugin for MJPG-streamer                       #
#                                                                              #
# This package work with the CAM130 connected directly to mini2410 board       #
#                                                                              #
#   Orginally Copyright (C) 2005 2006 Laurent Pinchart &&  Michel Xhaard       #
#   Modifications Copyright (C) 2006  Gabriel A. Devenyi                       #
#   Modifications Copyright (C) 2007  Tom Stöveken                             #
#   Modifications Copyright (C) 2009  Vladimir S. Fonov                        #
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

#include "../../utils.h"
#include "../../mjpg_streamer.h"


#include "s3c2410.h"
#include "utils.h"
#include "jdatatype.h"
#include "encoder.h"

/****************************************************************************
*			Public
****************************************************************************/
int init_s3c2410 (struct vdIn *vd, char *device, 
                  int width, int height)
{
  int err = -1;
  int f;
  int i;

  if (vd == NULL || device == NULL)
    return -1;

  if (width == 0 || height == 0)
    return -1;

  vd->videodevice=strdup(device);
  vd->framesizeIn=(width*height*BPPIN)>>3; 
  vd->hdrwidth=width;
  vd->hdrheight=height;
  
  vd->pFramebuffer=(unsigned char *) malloc ((size_t) vd->framesizeIn );
  
  vd->formatIn=0;

  DBG("Opening device\n");
  
  if ((vd->fd = open( vd->videodevice, O_RDWR)) == -1)
    exit_fatal ("ERROR opening V4L interface");

  DBG("Allocating input buffers\n");
  
  /* allocate the 4 frames output buffer */
  for (i = 0; i < OUTFRMNUMB; i++)
  {
      vd->ptframe[i] = NULL;
      vd->ptframe[i] = (unsigned char *) malloc ((size_t) vd->framesizeIn );
      vd->framelock[i] = 0;
  }

  vd->frame_cour = 0;
  
  pthread_mutex_init (&vd->grabmutex, NULL);
  
  return 0;
}

int close_s3c2410 (struct vdIn *vd)
{
  int i;
  free(vd->pFramebuffer);
  vd->pFramebuffer = NULL;

  DBG("close video_device\n");  
  close (vd->fd);
  /* dealloc the whole buffers */
  if (vd->videodevice)
  {
      free (vd->videodevice);
      vd->videodevice = NULL;
  }

  for (i = 0; i < OUTFRMNUMB; i++)
  {
      if (vd->ptframe[i])
	{
	  free (vd->ptframe[i]);
	  vd->ptframe[i] = NULL;
	  vd->framelock[i] = 0;
	  DBG("freeing output buffer %d\n",i);
	}
   }

   pthread_mutex_destroy (&vd->grabmutex);
   return 0;
}

int convertframe(unsigned char *dst,unsigned char *src, 
		 int width,int height, int formatIn, int qualite)
{ 
   return  encode_image(src,dst,qualite,RGB565to420,width,height);	
}

int s3c2410_Grab (struct vdIn *vd )
{
  static int frame = 0;

  int len;
  int size;
  int err = 0;
  int jpegsize = 0;
  int qualite = 1024;
  
  struct frame_t *headerframe;
  double timecourant =0;
  double temps = 0;
  
  timecourant = ms_time();

/* read method */
  size = vd->framesizeIn;
  len = read (vd->fd, vd->pFramebuffer, size);
  
  if (len <= 0 )
  {
      printf ("2410 read error\n");
      printf ("len %d asked %d \n", len, size);
      return 0;
  }
  /* Is there someone using the frame */
    while((vd->framelock[vd->frame_cour] != 0)&& vd->signalquit)
      usleep(1000);

  pthread_mutex_lock (&vd->grabmutex);
  /*
    memcpy (vd->ptframe[vd->frame_cour]+ sizeof(struct frame_t), vd->pFramebuffer, vd->framesizeIn);
    jpegsize =jpeg_compress(vd->ptframe[vd->frame_cour]+ sizeof(struct frame_t),len,
    vd->pFramebuffer, vd->hdrwidth, vd->hdrheight, qualite); 
    */
    temps = ms_time();
    
    jpegsize= convertframe(vd->ptframe[vd->frame_cour]+ sizeof(struct frame_t),
		  vd->pFramebuffer,
		  vd->hdrwidth, vd->hdrheight,
	          vd->formatIn,  qualite); 
		  
    headerframe=(struct frame_t*)vd->ptframe[vd->frame_cour];
    
    snprintf(headerframe->header,5,"%s","2410"); 
    
    headerframe->seqtimes = ms_time();
    headerframe->deltatimes=(int)(headerframe->seqtimes-timecourant); 
    headerframe->w = vd->hdrwidth;
    headerframe->h = vd->hdrheight;
    headerframe->size = (( jpegsize < 0)?0:jpegsize);; 
    headerframe->format = vd->formatIn; 
    headerframe->nbframe = frame++; 
    
  DBG("compress frame %d times %f\n",frame, headerframe->seqtimes-temps);
  vd->frame_cour = (vd->frame_cour +1) % OUTFRMNUMB;  
  pthread_mutex_unlock (&vd->grabmutex); 
  /************************************/
     
  return err;
}
