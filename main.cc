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
#include "vector3d.h"
#include <math.h>
#ifdef _OPENMP
#include <omp.h>
#endif
#ifdef _OPENACC
#include <openacc.h>
#include <curand.h>
#endif
#ifdef USE_MPI
#include <mpi.h>
#endif

void getParameters(char *filename, CameraParams *camera_params, RenderParams *renderer_params,
		   MandelBoxParams *mandelBox_paramsP);
void init3D       (CameraParams *camera_params, const RenderParams *renderer_params);
void renderFractal(const CameraParams &camera_params, const RenderParams &renderer_params, unsigned char* image);
void saveBMP      (const char* filename, const unsigned char* image, int width, int height);
void fillDefaultParams(char *filename, CameraParams *camP, RenderParams *renP, MandelBoxParams *boxP);
void print_renderer(RenderParams *renP);

// extern double MandelBulbDistanceEstimator(const vec3 &p0, const MandelBoxParams &params);


// used for percentage
extern void   printProgress( double perc, double time );
extern double getTime();

MandelBoxParams mandelBox_params;
struct stat st = {0};
#define PI 3.14159265

int main(int argc, char** argv)
{
  #ifdef USE_MPI
  int rank, size,namelen,provided;
    //MPI_Init(&argc, &argv);
    MPI_Init_thread(0, 0, MPI_THREAD_MULTIPLE, &provided );
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    MPI_Get_processor_name(processor_name, &namelen);
    printf("Nodes: %d, Name: %s, Rank: %d\n",size,processor_name,rank);
  #else
    int rank = 0;
    int size = 1;
  #endif


  #if defined(_OPENMP)
    //int nProcessors=omp_get_max_threads();
    int nProcessors=omp_get_num_procs();
    omp_set_num_threads(nProcessors);
  #endif
  CameraParams    camera_params;
  RenderParams    renderer_params;




  // getParameters(argv[1], &camera_params, &renderer_params, &mandelBox_params); //this will use the file
  fillDefaultParams(argv[1], &camera_params, &renderer_params, &mandelBox_params); // this will fill with data from getparams.c



  if (stat("images", &st) == -1) {
    mkdir("images", 0700);
}
  renderer_params.width = 1920;
  renderer_params.height = 1080;
  int image_size = renderer_params.width * renderer_params.height;
  unsigned char *image = (unsigned char*)malloc(3*image_size*sizeof(unsigned char));

  init3D(&camera_params, &renderer_params);


  // BEGIN PATHING TESTING



  // camera_params.camPos[0] = 20;
  // camera_params.camPos[1] = 20;
  // camera_params.camPos[2] = 7;

  // camP->camTarget[0] = 0;
  // camP->camTarget[1] = 0;
  // camP->camTarget[2] = 0;

  // camP->camUp[0] = 0;
  // camP->camUp[1] = 0;
  // camP->camUp[2] = 1;
  double time = getTime(); // was before pragma loop
  int i;
  int imax = 200;
  for (i=rank; i<imax;i+=size){
    snprintf(renderer_params.file_name,80,"images/image%010d.bmp",i+1);
    camera_params.camPos[0] = (200-i)/10.0;
    camera_params.camPos[1] = (200-i)/10.0;

    init3D(&camera_params, &renderer_params);

    double return_distances[5]; //forward (max), left (45), right (45), top (45), bottom (45)
    double dist;
    // dist =  MandelBulbDistanceEstimator(camera_params.camPos, MandelBoxParams);
    // printf("distance = %d\n",dist);
    renderFractal(camera_params, renderer_params, image);
    // printf("CPU %d: Saving: %s\n",rank,renderer_params.file_name);
    if (rank==0) printProgress((i+1)/(double)imax,getTime()-time);
    saveBMP(renderer_params.file_name, image, renderer_params.width, renderer_params.height);
  }
  if (rank==0) printProgress((double)1,getTime()-time);

  free(image);




  // MAKE 360 SPIN IN IMAGES
  // double time = getTime(); // was before pragma loop
  // int i;
  // int imax = 360;
  // for (i=rank; i<imax;i+=size){
  //   snprintf(renderer_params.file_name,80,"images/image%010d.bmp",i+1);
  //   // printf("iteration %d of %d\n",i,imax);
  //   camera_params.camPos[0] = 15*cos(PI/180*i);
  //   camera_params.camPos[1] = 15*sin(PI/180*i);
  //   init3D(&camera_params, &renderer_params);
  //   renderFractal(camera_params, renderer_params, image);
  //   // printf("CPU %d: Saving: %s\n",rank,renderer_params.file_name);
  //   if (rank==0) printProgress((i+1)/(double)imax,getTime()-time);
  //   saveBMP(renderer_params.file_name, image, renderer_params.width, renderer_params.height);
  // }
  // free(image);


  // MAKE SINGLE IMAGE
  // if (rank ==0) {
  // int image_size = renderer_params.width * renderer_params.height;
  // unsigned char *image = (unsigned char*)malloc(3*image_size*sizeof(unsigned char));

  // init3D(&camera_params, &renderer_params);
  // renderFractal(camera_params, renderer_params, image);
  // printf("CPU %d: Saving: %s\n",rank,renderer_params.file_name);
  // saveBMP(renderer_params.file_name, image, renderer_params.width, renderer_params.height);
  // free(image);
  // }

  if (rank==0) printf("\n\n");
  #ifdef USE_MPI
   MPI_Finalize();
  #endif
  return 0;
}
