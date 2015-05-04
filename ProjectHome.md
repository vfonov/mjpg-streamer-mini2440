**NOTICE**: Project is now located in https://github.com/vfonov/mjpg-streamer

Version of MJPG-STREAMER project: https://sourceforge.net/projects/mjpg-streamer/
Compiled for mini2440 SOC with support for embedded camera CAM130.

Current version is based on [r63](https://code.google.com/p/mjpg-streamer-mini2440/source/detail?r=63) version of mjpg-streamer, binary package is compiled with
arm-none-linux-gnueabi-gcc (Sourcery G++ Lite 2008q3-72) 4.3.2

### Installing ###
Binary package based on [r6](https://code.google.com/p/mjpg-streamer-mini2440/source/detail?r=6):

To use:

  1. download binary package: http://mjpg-streamer-mini2440.googlecode.com/files/mjpg-streamer-mini2440-bin-r6.tar.gz or source code and compile
  1. copy to mini2440 using NFS, FTP or SD card
  1. create a directory, for example
> > ` mkdir /mjpg-streamer `
  1. untar
> > ` cd /mjpg-streamer;tar zxf mjpg-streamer-mini2440-bin.tar.gz `
  1. to try using CAM130 - run
> > ` start_s3c2410.sh `
  1. to try using USB UVC based camera - connect camera to usb socket and run
> > ` start_uvc.sh `
  1. to try using USB camera without MJPG support - connect camera to usb socket and run
> > ` start_uvc_yuv.sh `

### Compiling ###
  1. install your platform version of gcc ( http://www.friendlyarm.net/dl.php?file=arm-linux-gcc-4.3.2.tgz in case of mini2440)
  1. run make CC=arm-linux-gcc package to create a tar.gz package


### Experimental S3C2440 camera interface linux kernel driver ###
Now project includes a minimal V4L2 driver for internal camera, able to capture images at full resolution (1280x1024) and scale down using hardware scaler in YUV422P format.

To try, build a custom kernel without built-in support form S3C2440 camera ( comment out ` CONFIG_S3C2440_CAMERA=y ` ) in configuration file. And build s3c2440camera\_driver:
` make KERNELDIR=<whenever you have your kernel> `, then install module using command ` insmod s3c2440camera.ko ` on mini2440
If you are compiling kernel version 2.6.29.4-FriendlyARM, you can use precompiled binary driver : [s3c2440camera.ko](http://s3c2440camera.googlecode.com/files/s3c2440camera.ko)
Also, [mjpg-streamer-mini2440-bin-r9.tar.gz](http://mjpg-streamer-mini2440.googlecode.com/files/mjpg-streamer-mini2440-bin-r9.tar.gz) precompiled binary package of the mjpg streamer able to use this driver.

Below is a screenshot of capture running at full resolition ( 1280x1024)

![http://mjpg-streamer-mini2440.googlecode.com/files/screenshot.jpg](http://mjpg-streamer-mini2440.googlecode.com/files/screenshot.jpg)

