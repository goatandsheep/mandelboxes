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
#endif
#ifdef USE_MPI
#include <mpi.h>
#endif

extern void getParameters(char *filename, CameraParams *camera_params, RenderParams *renderer_params, MandelBoxParams *mandelBox_paramsP);
extern void init3D       (CameraParams *camera_params, const RenderParams *renderer_params);
extern void renderFractal(const CameraParams &camera_params, const RenderParams &renderer_params, unsigned char* image);
extern void saveBMP(const char* filename, const unsigned char* image, int width, int height);
extern void fillDefaultParams(CameraParams *camP, RenderParams *renP, MandelBoxParams *boxP);
// extern void print_renderer(RenderParams *renP);
// extern void print_camera(CameraParams *camP);
extern void generateCameraPath(CameraParams &camera_params, RenderParams &renderer_params, CameraParams *camera_params_array, int frames, double camera_speed);


// used for percentage
extern void   printProgress( double perc, double time );
extern double getTime();

MandelBoxParams mandelBox_params;
struct stat st = {0};
// #define PI 3.14159265 // used for camera 360

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
    //int nProcessors=omp_get_max_threads(); // not used due to the need for an external variable to be set
    int nProcessors=omp_get_num_procs();
    omp_set_num_threads(nProcessors);
  #endif

  // begin normal program

  // make sure output directory exists
  if (stat("_images", &st) == -1)
    {
    mkdir("_images", 0700);
  }
  if (stat("_videos", &st) == -1)
    {
    mkdir("_videos", 0700);
  }

  CameraParams    camera_params;
  RenderParams    renderer_params;

  if (argc > 1){ // original behavior
    getParameters(argv[1], &camera_params, &renderer_params, &mandelBox_params);
    printf("Reading File: %s\n",argv[1]);
    int image_size = renderer_params.width * renderer_params.height;
    unsigned char *image = (unsigned char*)malloc(3*image_size*sizeof(unsigned char));
    init3D(&camera_params, &renderer_params);
    renderFractal(camera_params, renderer_params, image);
    printf("Saving %s\n",renderer_params.file_name);
    saveBMP(renderer_params.file_name, image, renderer_params.width, renderer_params.height);
    free(image);
  }
  else{ // do auto path (run with no arguments just "./mandelbox")

  fillDefaultParams(&camera_params, &renderer_params, &mandelBox_params); // this will fill with data from getparams.c
  // getParameters(argv[1], &camera_params, &renderer_params, &mandelBox_params); can load defaults from file

  // Modify Default Params (starting params)
  camera_params.camPos[0] = 20;
  camera_params.camPos[1] = 20;
  camera_params.camPos[2] = 7;

  camera_params.camTarget[0] = -4;
  camera_params.camTarget[1] = -4;
  camera_params.camTarget[2] = -1;

  double fov = 1; //need to re-set this as the pathing defaults to 0.4


  // Output Video Params
  int frames = 10000;
  int out_width = 1920; //1920
  int out_height = 1080; //1080
  double camera_speed = 0.01; //0.01 for 30 fps


  // Generate Path locally and then distribute to MPI cluster for HD rendering
  CameraParams *camera_params_array = (CameraParams *)malloc(sizeof(CameraParams)*frames);//forward (0), left (45), right (45), top (45), bottom (45)
  if (rank ==0) generateCameraPath(camera_params, renderer_params, camera_params_array, frames,camera_speed);

  #ifdef USE_MPI
    if (size >1){
    MPI_Bcast(camera_params_array, sizeof(CameraParams)*frames,MPI_CHAR, 0, MPI_COMM_WORLD);
    if (rank==0)printf("Broadcasting Path\n");
    }
  #endif

  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! BEGIN HD MPI !!!!!!!!!!!!!!!!!!!!!!!!!
  if (rank==0 and size == 1) printf("Rendering HD Images\n");
  if (rank==0 and size > 1) printf("Rendering HD Images Across %d Nodes\n",size);
  renderer_params.width = out_width;
  renderer_params.height = out_height;

  int image_size = renderer_params.width * renderer_params.height;
  unsigned char *image = (unsigned char*)malloc(3*image_size*sizeof(unsigned char));
  init3D(&camera_params, &renderer_params);


  camera_params.fov = fov;
  double time;
  if (rank == 0) time = getTime(); // was before pragma loop

  int i;
  for (i=rank; i<frames;i+=size){
    snprintf(renderer_params.file_name,80,"_images/image%010d.bmp",i+1);
    camera_params= camera_params_array[i];
    camera_params.fov = fov;

    init3D(&camera_params, &renderer_params);
    renderFractal(camera_params, renderer_params, image);


    if (rank==0) printProgress((i+1)/(double)frames,getTime()-time);
    saveBMP(renderer_params.file_name, image, renderer_params.width, renderer_params.height);
  }
  if (rank==0) printProgress((double)1,getTime()-time); //make it end at 100%

  free(image);
  }



  if (rank==0) printf("\n");
  #ifdef USE_MPI
   MPI_Finalize();
  #endif
  return 0;
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

