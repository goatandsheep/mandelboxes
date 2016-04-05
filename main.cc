/*
   This file is part of the Mandelbox program developed for the course
    CS/SE  Distributed Computer Systems taught by N. Nedialkov in the
    Winter of 2015-2016 at McMaster University.

    Copyright (C) 2015-2016 T. Gwosdz and N. Nedialkov

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
    // for makedir
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
    // end makedir
#include "camera.h"
#include "renderer.h"
#include "mandelbox.h"
#include <math.h>
#ifdef _OPENMP
#include <omp.h>
#endif
#ifdef _OPENACC
#include <openacc.h>
#endif

void getParameters(char *filename, CameraParams *camera_params, RenderParams *renderer_params,
		   MandelBoxParams *mandelBox_paramsP);
void init3D       (CameraParams *camera_params, const RenderParams *renderer_params);
void renderFractal(const CameraParams &camera_params, const RenderParams &renderer_params, unsigned char* image);
void saveBMP      (const char* filename, const unsigned char* image, int width, int height);
void fillParams(char *filename, CameraParams *camP, RenderParams *renP, MandelBoxParams *boxP);
void print_renderer(RenderParams *renP);


MandelBoxParams mandelBox_params;
struct stat st = {0};
#define PI 3.14159265

int main(int argc, char** argv)
{
  #if defined(_OPENMP)
    omp_set_num_threads(2);
  #endif
  CameraParams    camera_params;
  RenderParams    renderer_params;





  // getParameters(argv[1], &camera_params, &renderer_params, &mandelBox_params); //this will use the file
  fillParams(argv[1], &camera_params, &renderer_params, &mandelBox_params); // this will fill with data from getparams.c


  int image_size = renderer_params.width * renderer_params.height;
  unsigned char *image = (unsigned char*)malloc(3*image_size*sizeof(unsigned char));

  if (stat("images", &st) == -1) {
    mkdir("images", 0700);
}
  int i;
  int imax = 2;
  for (i=1; i<=imax;i++){
    snprintf(renderer_params.file_name,80,"images/image%010d.bmp",i);
    printf("iteration %d of %d\n",i,imax);
    camera_params.camPos[0] = 15*cos(PI/180*i);
    camera_params.camPos[1] = 15*sin(PI/180*i);
    init3D(&camera_params, &renderer_params);
    renderFractal(camera_params, renderer_params, image);
    printf("saving: %s\n",renderer_params.file_name);
    saveBMP(renderer_params.file_name, image, renderer_params.width, renderer_params.height);
  }

  free(image);

  return 0;
}
