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
#include "color.h"
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

extern void getParameters(char *filename, CameraParams *camera_params, RenderParams *renderer_params, MandelBoxParams *mandelBox_paramsP);
extern void init3D       (CameraParams *camera_params, const RenderParams *renderer_params);
extern void renderFractal(const CameraParams &camera_params, const RenderParams &renderer_params, unsigned char* image);
extern void saveBMP(const char* filename, const unsigned char* image, int width, int height);
extern void fillDefaultParams(char *filename, CameraParams *camP, RenderParams *renP, MandelBoxParams *boxP);
// extern void print_renderer(RenderParams *renP);
// extern void print_camera(CameraParams *camP);
extern void generateCameraPath(CameraParams &camera_params, RenderParams &renderer_params, CameraParams *camera_params_array, int frames, double camera_speed);


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
    printf("Nodes (tot): %d, Name (Host): %s, Rank: %d\n",size,processor_name,rank);
  #else
    int rank = 0;
    int size = 1;
  #endif


  #if defined(_OPENMP)
    //int nProcessors=omp_get_max_threads();
    int nProcessors=omp_get_num_procs();
    omp_set_num_threads(nProcessors);
  #endif

  // begin normal program
  CameraParams    camera_params;
  RenderParams    renderer_params;
  fillDefaultParams(argv[1], &camera_params, &renderer_params, &mandelBox_params); // this will fill with data from getparams.c

  // Modify Default Params
  camera_params.camTarget[0] = 7;
  camera_params.camTarget[1] = 2;
  camera_params.camTarget[2] = 0;
  camera_params.camPos[0] = 10;
  camera_params.camPos[1] = 0;
  camera_params.camPos[2] = 0;

  // Output Video Params
  int frames = 400;
  int out_width = 100; //1920
  int out_height = 100; //1080
  double camera_speed = 0.05;


  // make sure output directory exists
  if (stat("_images", &st) == -1)
    {
    mkdir("_images", 0700);
  }
  if (stat("_videos", &st) == -1)
    {
    mkdir("_videos", 0700);
  }





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


  // Generate Path locally and then distribute to MPI cluster for HD rendering
  CameraParams *camera_params_array = (CameraParams *)malloc(sizeof(CameraParams)*frames);//forward (0), left (45), right (45), top (45), bottom (45)
  if (rank ==0) generateCameraPath(camera_params, renderer_params, camera_params_array, frames,camera_speed);

  #ifdef USE_MPI
    if (size >1){
    MPI_Bcast(camera_params_array, sizeof(CameraParams)*frames,MPI_CHAR, 0, MPI_COMM_WORLD);
    printf("Broadcasting Path\n");
    }
  #endif

  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! BEGIN HD MPI !!!!!!!!!!!!!!!!!!!!!!!!!
  printf("Rendering HD Images\n");
  renderer_params.width = out_width;
  renderer_params.height = out_height;

  int image_size = renderer_params.width * renderer_params.height;
  unsigned char *image = (unsigned char*)malloc(3*image_size*sizeof(unsigned char));
  init3D(&camera_params, &renderer_params);


  camera_params.fov = 1;
  double time;
  if (rank == 0) time = getTime(); // was before pragma loop

  int i;
  for (i=rank; i<frames;i+=size){
    snprintf(renderer_params.file_name,80,"_images/image%010d.bmp",i+1);
    camera_params= camera_params_array[i];
    camera_params.fov = 1;
    init3D(&camera_params, &renderer_params);

    renderFractal(camera_params, renderer_params, image);


    if (rank==0) printProgress((i+1)/(double)frames,getTime()-time);
    saveBMP(renderer_params.file_name, image, renderer_params.width, renderer_params.height);
  }
  if (rank==0) printProgress((double)1,getTime()-time);

  free(image);


  if (rank==0) printf("\n");
  #ifdef USE_MPI
   MPI_Finalize();
  #endif
  return 0;
}
