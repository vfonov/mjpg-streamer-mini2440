#include <asm/types.h>          /* for videodev2.h */
#include <linux/videodev2.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

int main(int argc,char **argv)
{
  const char *output_base;
	const char *video_device;
  int fcount=1;
  int dev_fd;
  int buffer_width=1280;
  int buffer_height=1024;
  int buffer_size;
  int Ysize,CbCrsize;
  unsigned char *buffer=NULL;
	struct v4l2_capability cap;
	struct v4l2_format fmt;
	char fourcc[5]={0,0,0,0,0};
	
  int i;
   
  if(argc<4)
  {
    fprintf(stderr,"Usage:%s <device> <output_base> [number of frames]\n",argv[0]);
    return 1;
  }
	video_device=argv[1];
  output_base=argv[2];
  
  if(argc>2) fcount=atoi(argv[3]);

  
  if ((dev_fd = open( video_device, O_RDWR)) == -1)
    perror("ERROR opening camera interface\n");  
  
	if (0 != ioctl(dev_fd, VIDIOC_QUERYCAP, &cap)) {
		if (EINVAL == errno) {
			fprintf (stderr, "%s is no V4L2 device\n",video_device);
			return 1;
		} else {
			perror ("VIDIOC_QUERYCAP");
			return 1;
		}
	}
	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
		fprintf (stderr, "%s is no video capture device\n",video_device);
		return 1;
	}	
	
	if (!(cap.capabilities & V4L2_CAP_READWRITE)) {
		fprintf (stderr, "%s does not support read i/o\n", video_device);
		return 1;
	}	
	
	memset(&fmt,sizeof(fmt),0);
	fmt.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
	
	if (0 != ioctl (dev_fd, VIDIOC_G_FMT, &fmt))
	{
		perror ("VIDIOC_G_FMT");
		return 1;
	}
	
	buffer_width= fmt.fmt.pix.width;
	buffer_height=fmt.fmt.pix.height;
	memmove(fourcc,&fmt.fmt.pix.pixelformat,4);
	
	printf("Goint to capture frames %d x %d format: %4s framesize: %d\n",
				 fmt.fmt.pix.width,
				 fmt.fmt.pix.height,
				 fourcc,
				 fmt.fmt.pix.sizeimage);
								
  printf("Goint to capture %d images to %s\n",fcount,output_base);
  
	
  buffer_size=fmt.fmt.pix.sizeimage; //YCbCr422
  Ysize=buffer_width*buffer_height;
  CbCrsize=buffer_width*buffer_height/2;
  
  if((buffer=malloc(buffer_size))==NULL)
    perror("Can't allocate buffer\n");
  
  for(i=0;i<fcount;i++)
  {
    FILE *img=NULL;
    char tmp[1024];
    int rd=0;
    
    if((rd=read(dev_fd, buffer, buffer_size))<buffer_size)
    {
      fprintf(stderr,"Expected %d got %d!\n",buffer_size,rd);
      continue;
    }
    
    sprintf(tmp,"%s_%03d.yuv422",output_base,i);
    
    if((img=fopen(tmp,"wb"))==NULL)
    {
      fprintf(stderr,"Can't open file %s for writing\n",tmp);
      break;
    } 
    
    //fprintf(img,"P5\n%d %d\n255\n",buffer_width,buffer_height);
    if(fwrite(buffer,1,buffer_size,img)<buffer_size)
    {
      fprintf(stderr,"Can't write to file %s\n",tmp);
      fclose(img);
      break;
    }
    fclose(img);
    
    /*
    sprintf(tmp,"%s_%03d_CbCr.ppm",output_base,i);
    
    if((img=fopen(tmp,"wb"))==NULL)
    {
      fprintf(stderr,"Can't open file %s for writing\n",tmp);
      break;
    } 
    
    fprintf(img,"P5\n%d %d\n255\n",buffer_width/2,buffer_height*2);
    if(fwrite(buffer+Ysize,1,CbCrsize*2,img)<CbCrsize*2)
    {
      fprintf(stderr,"Can't write to file %s\n",tmp);
      fclose(img);
      break;
    }
    fclose(img);*/
    
    printf("%d\t",i);
    fflush(stdout);
  }
  printf("\n");
  free(buffer);
  close(dev_fd);
}
