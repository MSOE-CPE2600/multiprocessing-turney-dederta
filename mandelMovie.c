/*
 * File: mandelMovie.c
 * Author: Ashlyn Dedert
 * Date: 11/11/2025
 * Description: Generates a series of mandelbrot set images by repeatedly running the
 * mandel executable with varying changes.
 * 
 * Command line options:
 * -p numChildren    : number of child processes to run simultaneously (default 1)
 * -f numFrames      : number of frames to generate (default 10)
 * -x xcenter        : x coordinate of center point (default 0.0)
 * -y ycenter        : y coordinate of center point (default 0.0)
 * -s scale          : initial scale (default 4.0)
 * -z zoomFactor     : zoom factor per frame (default 0.9)
 * -o outFilePrefix  : output file prefix (default "mandel")
 * 
 * Example usage:
 *   ./mandelMovie -p 4 -f 20 -x -0.7 -y 0.0 -s 3.0 -z 0.8 -o mandelZoom
 * 
 * Each frame is saved as a .jpeg file which can be combined using
 *   ffmpeg -framerate 24 -i mandel%d.jpg -pix_fmt yuv420p mandel.mp4
 * 
 * Compilation:
 *   gcc mandelMovie.c -o mandelMovie -lm
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <math.h>

int main(int argc, char *argv[])
{
    int numChildren = 1;        // number of processes
    int numFrames = 10;         // number of frames to generate
    double xcenter = 0.0;       // x image center
    double ycenter = 0.0;       // y image center
    double scale = 4.0;         // starting scale
    double zoomFactor = 0.9;    // zoom multiplier

    char *outFilePrefix = "mandel";

    int opt;

    while ((opt = getopt(argc, argv, "p:f:x:y:s:z:o:h")) != -1)
    {
        switch (opt)
        {
            case 'p':
                numChildren = atoi(optarg);
                break;
            case 'f':
                numFrames = atoi(optarg);
                break;
            case 'x':
                xcenter = atof(optarg);
                break;
            case 'y':
                ycenter = atof(optarg);
                break;
            case 's':
                scale = atof(optarg);
                break;
            case 'z':
                zoomFactor = atof(optarg);
                break;
            case 'o':
                outFilePrefix = optarg;
                break;
            case 'h':
                printf("Usage: %s [options]\n", argv[0]);
                printf("Options:\n");
                printf("  -p <numChildren>   Number of simultaneous child processes (default 1)\n");
                printf("  -f <numFrames>     Total number of frames to generate (default 10)\n");
                printf("  -x <xcenter>       X coordinate of image center (default 0.0)\n");
                printf("  -y <ycenter>       Y coordinate of image center (default 0.0)\n");
                printf("  -s <scale>         Starting scale or zoom width (default 4.0)\n");
                printf("  -z <zoomFactor>    Zoom multiplier per frame (default 0.9)\n");
                printf("  -o <prefix>        Output filename prefix (default \"mandel\")\n");
                exit(0);
            default:
                fprintf(stderr, "Use -h for help\n");
                exit(1);
        }
    }
    printf("Running mandelMovie with %d children and %d frames\n", numChildren, numFrames);

    int active = 0;

    for (int i = 0; i < numFrames; i++)
    {
        // wait if too many children are running
        while (active >= numChildren)
        {
            wait(NULL);
            active--;
        }

        double frameScale = scale * pow(zoomFactor, i);
        char filename[64];
        snprintf(filename, sizeof(filename), "%s%d.jpg", outFilePrefix, i);

        pid_t pid = fork();
        if (pid == 0)
        {
            // Child process
            char xstr[32], ystr[32], sstr[32], ostr[64];
            snprintf(xstr, sizeof(xstr), "%lf", xcenter);
            snprintf(ystr, sizeof(ystr), "%lf", ycenter);
            snprintf(sstr, sizeof(sstr), "%lf", frameScale);
            snprintf(ostr, sizeof(ostr), "%s", filename);

            execlp("./mandel", "mandel", "-x", xstr, "-y", ystr, "-s", sstr, "-o", ostr, (char *)NULL);
            perror("execlp failed");
            exit(1);
        }
        else if (pid > 0)
        {
            // Parent process
            active++;
        }
        else
        {
            perror("fork failed");
            exit(1);
        }


        printf("Frame %d: scale=%f -> %s\n", i, frameScale, filename);
    }

    while (active > 0)
    {
        wait(NULL);
        active--;
    }

    printf("All frames generated.\n");
}