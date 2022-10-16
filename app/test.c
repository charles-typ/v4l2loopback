/*
 * Open the output.rgb with:
 * display -size 640x480 -depth 8 -colorspace RGB output.rgb
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/videodev2.h>

#define CLIP(color) (unsigned char) (((color) > 0xFF) ? 0xff : (((color) < 0) ? 0 : (color)))
void convert_yuyv_to_rgb24(const uint8_t *src, uint8_t *dest, int width, int height)
{
        /* From: Hans de Goede <j.w.r.degoede@hhs.nl> */
        int j;

        while(--height >= 0) {
                for(j = 0; j < width; j += 2) {
                        int u = src[1];
                        int v = src[3];
                        int u1 = (((u - 128) << 7) +  (u - 128)) >> 6;
                        int rg = (((u - 128) << 1) +  (u - 128) +
                                  ((v - 128) << 2) + ((v - 128)  << 1)) >> 3;
                        int v1 = (((v - 128) << 1) +  (v - 128)) >> 1;

                        *dest++ = CLIP(src[0] + v1);
                        *dest++ = CLIP(src[0] - rg);
                        *dest++ = CLIP(src[0] + u1);
                        *dest++ = CLIP(src[2] + v1);
                        *dest++ = CLIP(src[2] - rg);
                        *dest++ = CLIP(src[2] + u1);
                        src += 4;
                }
        }
}

int main() {
	char* deviceName = "/dev/video0";
	//int width = 640;
	//int height = 480;
	int width = 1280;
	int height =720;

	int fd = open(deviceName, O_RDWR);
	if (fd == -1) {
		printf("Couldn't open video device!\n");
		return -1;
	}

	struct v4l2_cropcap cropcap;
	struct v4l2_crop crop;
	memset(&cropcap, 0, sizeof(cropcap));
	cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (ioctl(fd, VIDIOC_CROPCAP, &cropcap) == 0) {
		memset(&crop, 0, sizeof(crop));
		crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		crop.c    = cropcap.defrect;
		if (ioctl(fd, VIDIOC_S_CROP, &crop) < 0) {
			switch(errno) {
				case EINVAL:
					/* Cropping not supported. */
				default:
					break;
			}
		}
	}

	struct v4l2_format fmt;
	memset(&fmt, 0, sizeof(fmt));
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width       = width;
	fmt.fmt.pix.height      = height;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
	fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;
	if (ioctl(fd, VIDIOC_S_FMT, &fmt) < 0) {
		printf("Couldn't set v4l format!\n");
		return -1;
	}
	if (ioctl(fd, VIDIOC_G_FMT, &fmt) < 0) {
		printf("Couldn't get v4l format!\n");
		return -1;
	}

	//struct v4l2_control control;
	//memset(&control, 0, sizeof(control));
	//control.id = V4L2_CID_AUTO_WHITE_BALANCE;
	//control.value = 1;
	//if (ioctl(fd, VIDIOC_S_CTRL, &control) < 0) {
	//	printf("Couldn't set auto white balance!\n");
	//	//return -1;
	//}
	//memset(&control, 0, sizeof(control));
	//control.id = V4L2_CID_EXPOSURE_AUTO;
	//control.value = 1;
	//if (ioctl(fd, VIDIOC_S_CTRL, &control) < 0) {
	//	printf("Couldn't set auto exposure!\n");
	//	//return -1;
	//}
	//memset(&control, 0, sizeof(control));
	//control.id = V4L2_CID_HFLIP;
	//control.value = 1;
	//if (ioctl(fd, VIDIOC_S_CTRL, &control) < 0) {
	//	printf("Couldn't set h flip!\n");
	//	//return -1;
	//}
	//memset(&control, 0, sizeof(control));
	//control.id = V4L2_CID_VFLIP;
	//control.value = 1;
	//if (ioctl(fd, VIDIOC_S_CTRL, &control) < 0) {
	//	printf("Couldn't set v flip!\n");
	//	//return -1;
	//}

	struct v4l2_streamparm fps;
	memset(&fps, 0, sizeof(fps));
	fps.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (ioctl(fd, VIDIOC_G_PARM, &fps) < 0) {
		printf("Couldn't query v4l fps!\n");
		return -1;
	}
	fps.parm.capture.timeperframe.numerator = 1;
	fps.parm.capture.timeperframe.denominator = 30;
	if (ioctl(fd, VIDIOC_S_PARM, &fps) < 0) {
		printf("Couldn't set v4l fps!\n");
		return -1;
	}
    printf("Checkpoint 1\n");

	int yuyv_size = fmt.fmt.pix.sizeimage;
    printf("YUYV size is set to: %d\n", fmt.fmt.pix.sizeimage);
	int rgb_size = 3*width*height;
	uint8_t *yuyv_buffer = (uint8_t *) malloc(yuyv_size);
	uint8_t *rgb_buffer = (uint8_t *) malloc(rgb_size);

	int ret = read(fd, yuyv_buffer, yuyv_size);
	printf("reading %d bytes from video device.\n", ret);
	if (ret == -1) {
		printf("read error!\n");
	} else if (ret != yuyv_size) {
		printf("couldn't fill the buffer completely!\n");
	} else {
        printf("Checkpoint 2\n");
		//int fdout = open("output.rgb", O_WRONLY|O_CREAT|O_TRUNC, 0x1FF);
		int fdout = open("output.yuv", O_WRONLY|O_CREAT|O_TRUNC, 0x1FF);
		if (fdout != -1) {
        	printf("Checkpoint 3\n");
			//convert_yuyv_to_rgb24(yuyv_buffer, rgb_buffer, width, height);
        	//printf("Checkpoint 4\n");
			//uint8_t *buffer = rgb_buffer;
			//int size = rgb_size;
        	//printf("Checkpoint 5\n");
			//ret = write(fdout, buffer, size);
			ret = write(fdout, yuyv_buffer, yuyv_size);
        	printf("Checkpoint 6\n");
			if (ret == -1) {
				printf("write error!\n");
			//} else if (ret < size) {
			} else if (ret < yuyv_size) {
				printf("couldn't write the buffer completely!\n");
			}
			printf("writing %d bytes into file.\n", ret);
			close(fdout);
		} else {
			printf("Couldn't open the output file!\n");
		}
	}
        printf("Checkpoint 7\n");
	free(yuyv_buffer);
	free(rgb_buffer);

	if (fd >= 0) {
		close(fd);
	}

	return 0;
}
