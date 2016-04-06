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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "renderer.h"
#include "mandelbox.h"
#include "camera.h"

#define BUF_SIZE 1024

char buf[BUF_SIZE];

void getParameters(char *filename, CameraParams *camP, RenderParams *renP, MandelBoxParams *boxP)
{
  FILE *fp;
  int ret;
  double *d;

  renP->fractalType = 0;
  renP->maxRaySteps = 8000;
  renP->maxDistance = 1000;

  fp = fopen(filename,"r");

  if( !fp )
    {
      printf(" *** File %s does not exist\n", filename);
      exit(1);
    }

  int count = 0;

  while (1)
    {
      memset(buf, 0, BUF_SIZE);

      ret = fscanf(fp, "%1023[^\n]\n", buf);
      if (ret == EOF) break;

      if(buf[0] == '#') // comment line
	continue;

      switch(count)
	{
	  // CAMERA
	  //camera position
	case 0:
	  d = camP->camPos;
	  sscanf(buf, "%lf %lf %lf", d, d+1, d+2);
	  break;
	case 1:
	  //camera target
	  d = camP->camTarget;
	  sscanf(buf, "%lf %lf %lf", d, d+1, d+2);
	  break;
	  //camera up
	case 2:
	  d = camP->camUp;
	  sscanf(buf, "%lf %lf %lf", d, d+1, d+2);
	  break;
	  //field of view
	case 3:
	  sscanf(buf, "%lf", &camP->fov);
	  break;

	  //IMAGE
	  //width, height
	case 4:
	  sscanf(buf, "%d %d", &renP->width, &renP->height);
	  break;
	  //detail
	case 5:
	  sscanf(buf, "%f", &renP->detail);
	  break;

	  //FRACTAL
	case 6:
	  sscanf(buf, "%f %f %f", &boxP->scale, &boxP->rMin, &boxP->rFixed);
 	  break;

	case 7:
	  sscanf(buf, "%d %f ", &boxP->num_iter, &boxP->escape_time);
	  break;

	  //COLORING
	case 8:
	  sscanf(buf, "%d", &renP->colourType);
	  break;
      //Brightness
	case 9:
	  sscanf(buf, "%f ", &renP->brightness);
	  break;
	case 10:
	  sscanf(buf, "%d ", &renP->super_sampling);
	  break;

	  //FILENAME
	case 11:
	  strcpy(renP->file_name, buf);
	  break;
	}
      count++;
    }
  fclose(fp);
}



void fillParams(char *filename, CameraParams *camP, RenderParams *renP, MandelBoxParams *boxP)
{
  //initiat mandel box params
  //   float rMin, rFixed;
  //   float scale;
  //   float escape_time;
  //   int num_iter;
  // } MandelBoxParams;
  boxP->scale = 2.00001;
  boxP->rMin = 0.5;
  boxP->rFixed = 1.0;
  boxP->num_iter = 13;
  boxP->escape_time = 1000.0;

  // initiate renderer
  //   int fractalType;
  //   int colourType;
  //   int super_sampling;
  //   float brightness;
  //   int width;
  //   int height;
  //   float detail;
  //   int maxRaySteps;
  //   float maxDistance;
  //   char file_name[80];
  // } RenderParams;
  renP->fractalType = 0;
  renP->maxRaySteps = 8000;
  renP->maxDistance = 1000.0;
  renP->width = 1920;
  renP->height = 1080;
  renP->detail = -3;
  renP->colourType = 1;
  renP->brightness = 1.2;
  renP->super_sampling = 0;
  snprintf(renP->file_name,80,"test%d.bmp",1);

  //initiate camera
  // double camPos[3];
  // double camTarget[3];
  // double camUp[3];
  // double fov;
  // double matModelView[16];
  // double matProjection[16];
  // double matInvProjModel[16];
  // int    viewport[4];
  // } CameraParams;

  camP->camPos[0] = 20;
  camP->camPos[1] = 20;
  camP->camPos[2] = 7;

  camP->camTarget[0] = 0;
  camP->camTarget[1] = 0;
  camP->camTarget[2] = -1;

  camP->camUp[0] = 0;
  camP->camUp[1] = 0;
  camP->camUp[2] = 1;
  camP->fov = 1;


}



void print_renderer(RenderParams *renP)
{
  // initiate renderer
  //   int fractalType;
  //   int colourType;
  //   int super_sampling;
  //   float brightness;
  //   int width;
  //   int height;
  //   float detail;
  //   int maxRaySteps;
  //   float maxDistance;
  //   char file_name[80];
  // } RenderParams;
  printf("1. fractalType: %d\n",renP->fractalType);
  printf("2. maxRaySteps: %d\n",renP->maxRaySteps);
  printf("3. maxDistance: %f\n",renP->maxDistance);
  printf("4. width: %d\n",renP->width);
  printf("5. height: %d\n",renP->height);
  printf("6. detail: %f\n",renP->detail);
  printf("7. colourType: %d\n",renP->colourType);
  printf("8. brightness: %f\n",renP->brightness);
  printf("9. super_sampling: %d\n",renP->super_sampling);
  printf("10. file_name: %s\n",renP->file_name);
}
