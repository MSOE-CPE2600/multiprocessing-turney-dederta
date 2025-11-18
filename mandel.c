/// 
//  mandel.c
//  Based on example code found here:
//  https://users.cs.fiu.edu/~cpoellab/teaching/cop4610_fall22/project3.html
//
//  Converted to use jpg instead of BMP and other minor changes
//  Modified by: Ashlyn DedertSS
///
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "jpegrw.h"
#include <pthread.h>

// struct for passing arguments to threads
typedef struct {
	imgRawImage* img;
	double xmin;
	double xmax;
	double ymin;
	double ymax;
	int max;
	int start_row;
	int end_row;
} thread_args_t;

// local routines
static int iteration_to_color( int i, int max );
static int iterations_at_point( double x, double y, int max );
static void compute_image( imgRawImage *img, double xmin, double xmax,
									double ymin, double ymax, int max, int num_threads );
static void show_help();
void *computeRegion(void *arg);

int main( int argc, char *argv[] )
{
	char c;

	// These are the default configuration values used
	// if no command line arguments are given.
	const char *outfile = "mandel.jpg";
	double xcenter = 0;
	double ycenter = 0;
	double xscale = 4;
	double yscale = 0; // calc later
	int    image_width = 1000;
	int    image_height = 1000;
	int    max = 1000;
	int num_threads = 1; // default

	// For each command line argument given,
	// override the appropriate configuration value.

	while((c = getopt(argc,argv,"x:y:s:W:H:m:o:t:h"))!=-1) {
		switch(c) 
		{
			case 'x':
				xcenter = atof(optarg);
				break;
			case 'y':
				ycenter = atof(optarg);
				break;
			case 's':
				xscale = atof(optarg);
				break;
			case 'W':
				image_width = atoi(optarg);
				break;
			case 'H':
				image_height = atoi(optarg);
				break;
			case 'm':
				max = atoi(optarg);
				break;
			case 'o':
				outfile = optarg;
				break;
			case 'h':
				show_help();
				exit(1);
				break;
			case 't':
				num_threads = atoi(optarg);
				if (num_threads < 1){
					num_threads = 1;
				}
				if (num_threads > 20){
					num_threads = 20;
				}
				break;
		}
	}

	// Calculate y scale based on x scale (settable) and image sizes in X and Y (settable)
	yscale = xscale / image_width * image_height;

	// Display the configuration of the image.
	printf("mandel: x=%lf y=%lf xscale=%lf yscale=%1f max=%d outfile=%s\n",xcenter,ycenter,xscale,yscale,max,outfile);

	// Create a raw image of the appropriate size.
	imgRawImage* img = initRawImage(image_width,image_height);

	// Fill it with a black
	setImageCOLOR(img,0);

	// Compute the Mandelbrot image
	compute_image(
		img,
		xcenter-xscale/2,
		xcenter+xscale/2,
		ycenter-yscale/2,
		ycenter+yscale/2,
		max, 
		num_threads);

	// Save the image in the stated file.
	storeJpegImageFile(img,outfile);

	// free the mallocs
	freeRawImage(img);

	return 0;
}


/*
Return the number of iterations at point x, y
in the Mandelbrot space, up to a maximum of max.
*/

int iterations_at_point( double x, double y, int max )
{
	double x0 = x;
	double y0 = y;

	int iter = 0;

	while( (x*x + y*y <= 4) && iter < max ) {

		double xt = x*x - y*y + x0;
		double yt = 2*x*y + y0;

		x = xt;
		y = yt;

		iter++;
	}

	return iter;
}

/*
Compute an entire Mandelbrot image, writing each point to the given bitmap.
Scale the image to the range (xmin-xmax,ymin-ymax), limiting iterations to "max"
*/

void compute_image(imgRawImage* img, double xmin, double xmax, double ymin, double ymax, int max, int num_threads)
{
	pthread_t threads[num_threads];
	thread_args_t args[num_threads];
	int height = img->height;
	int rows_per_thread = height / num_threads;

	for (int i = 0; i < num_threads; i++){
		args[i].img = img;
		args[i].xmin = xmin;
		args[i].xmax = xmax;
		args[i].ymin = ymin;
		args[i].ymax = ymax;
		args[i].max = max;
		args[i].start_row = i * rows_per_thread;
		args[i].end_row = (i == num_threads - 1) ? height : (i + 1) * rows_per_thread;

		pthread_create(&threads[i], NULL, computeRegion, &args[i]);
	}
	for (int i = 0; i < num_threads; i++){
		pthread_join(threads[i], NULL);
	}
}


/*
Convert a iteration number to a color.
Here, we just scale to gray with a maximum of imax.
Modify this function to make more interesting colors.
*/
int iteration_to_color( int iters, int max )
{
	int color = 0xFFFFFF*iters/(double)max;
	return color;
}


// Show help message
void show_help()
{
	printf("Use: mandel [options]\n");
	printf("Where options are:\n");
	printf("-m <max>    The maximum number of iterations per point. (default=1000)\n");
	printf("-x <coord>  X coordinate of image center point. (default=0)\n");
	printf("-y <coord>  Y coordinate of image center point. (default=0)\n");
	printf("-s <scale>  Scale of the image in Mandlebrot coordinates (X-axis). (default=4)\n");
	printf("-W <pixels> Width of the image in pixels. (default=1000)\n");
	printf("-H <pixels> Height of the image in pixels. (default=1000)\n");
	printf("-o <file>   Set output file. (default=mandel.jpg)\n");
	printf("-t <threads> Number of threads to use. (default=1)\n");
	printf("-h          Show this help text.\n");
	printf("\nSome examples are:\n");
	printf("mandel -x -0.5 -y -0.5 -s 0.2\n");
	printf("mandel -x -.38 -y -.665 -s .05 -m 100\n");
	printf("mandel -x 0.286932 -y 0.014287 -s .0005 -m 1000\n\n");
}

void *computeRegion(void *arg){
	thread_args_t *args = (thread_args_t*) arg;
	int width = args->img->width;
	int height = args->img->height;

	for (int j = args->start_row; j < args-> end_row; j++){
		for (int i = 0; i < width; i++){
			double x = args->xmin + i*(args->xmax - args->xmin)/width;
			double y = args->ymin + j*(args->ymax - args->ymin)/height;
			int iters = iterations_at_point(x, y, args->max);
			setPixelCOLOR(args->img, i, j, iteration_to_color(iters,args->max));
		}
	}
	return NULL;
}
