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

void getParameters(char *filename, CameraParams *camera_params, RenderParams *renderer_params,
		   MandelBoxParams *mandelBox_paramsP);
void init3D       (CameraParams *camera_params, const RenderParams *renderer_params);

void renderFractal(const CameraParams &camera_params, const RenderParams &renderer_params, unsigned char* image, pixelData& max_pix_data,pixelData *return_distances);

void saveBMP(const char* filename, const unsigned char* image, int width, int height);
void fillDefaultParams(char *filename, CameraParams *camP, RenderParams *renP, MandelBoxParams *boxP);
void print_renderer(RenderParams *renP);
void print_camera(CameraParams *camP);


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

  if (stat("images", &st) == -1)
    {
    mkdir("images", 0700);
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
  int frames = 100;
  // CameraParams camera_params_array[frames];
  CameraParams *camera_params_array = (CameraParams *)malloc(sizeof(CameraParams)*frames);//forward (0), left (45), right (45), top (45), bottom (45)


  // MAKE SINGLE IMAGE
  if (rank ==0) {
    renderer_params.width = 100;
    renderer_params.height = 100;
    int image_size = renderer_params.width * renderer_params.height;
    unsigned char *image = (unsigned char*)malloc(3*image_size*sizeof(unsigned char));
    camera_params.camTarget[0] = 7;
    camera_params.camTarget[1] = 2;
    camera_params.camTarget[2] = 0;
    camera_params.camPos[0] = 10;
    camera_params.camPos[1] = 0;
    camera_params.camPos[2] = 0;
    camera_params.fov = 0.4;

    vec3 direction,direction2,bump;
    direction2.x = 0;
    direction2.y = 0;
    direction2.z = 0;


    pixelData max_pix_data;

    // pixelData *return_distances = (pixelData *)malloc(sizeof(pixelData));//forward (0), left (45), right (45), top (45), bottom (45)
    pixelData return_distances[5];
    double time = getTime();
    double new_dir_weight;
    double move_rate, cam_dist;
    double dir_error,dir_error2;
    int last_direction_change_reset = 2;
    int last_direction_change = 0;

    int j;
    for (j=0;j<frames;j++){
      init3D(&camera_params, &renderer_params);

      renderFractal(camera_params, renderer_params, image,max_pix_data,return_distances);
      // print_camera(&camera_params);

      // printf("%f\n",return_distances[2].distance);




      // snprintf(renderer_params.file_name,80,"images/image%010d.bmp",j+1);

      // saveBMP(renderer_params.file_name, image, renderer_params.width, renderer_params.height);

      move_rate = 0.01*max_pix_data.distance; //0.001
      cam_dist = move_rate+1;


      direction.x = max_pix_data.hit.x-camera_params.camPos[0];
      direction.y = max_pix_data.hit.y-camera_params.camPos[1];
      direction.z = max_pix_data.hit.z-camera_params.camPos[2];
      last_direction_change = last_direction_change_reset;

      direction.Normalize();

      if (max_pix_data.distance < 0.01){ //0.001
        new_dir_weight = 1- 10*max_pix_data.distance; //- max_pix_data.distance
      }
      else{
        new_dir_weight = 0.5; // camera aggression
      }
      dir_error = new_dir_weight*(direction2-direction).Magnitude();
      dir_error2 = 1-dir_error;
      // printf("dir error %f\n",dir_error);
      direction.x = dir_error2*direction2.x+dir_error* direction.x;
      direction.y = dir_error2*direction2.y+dir_error* direction.y;
      direction.z = dir_error2*direction2.z+dir_error* direction.z;
      direction.Normalize();
      direction2 = direction;
      int i;
      double bump_factor = move_rate/20; //30
      if (return_distances[1].escaped == false && return_distances[2].escaped == false){ // left and right
        int lr;
        if (return_distances[1].distance > return_distances[2].distance){ // then move left
          lr = 1;
        }
        else{
          lr = 2;
        }
        bump.x = return_distances[lr].hit.x- camera_params.camPos[0];
        bump.y = return_distances[lr].hit.y- camera_params.camPos[1];
        bump.z = return_distances[lr].hit.z- camera_params.camPos[2];
        bump.Normalize();
        camera_params.camPos[0]+=bump.x*bump_factor;
        camera_params.camPos[1]+=bump.y*bump_factor;
        camera_params.camPos[2]+=bump.z*bump_factor;
      }
      if (return_distances[4].escaped == false && return_distances[5].escaped == false){ // up and down
        int ud;
        if (return_distances[4].distance > return_distances[5].distance){ // then move left
          ud = 4;
        }
        else{
          ud = 5;
        }
        bump.x = return_distances[ud].hit.x- camera_params.camPos[0];
        bump.y = return_distances[ud].hit.y- camera_params.camPos[1];
        bump.z = return_distances[ud].hit.z- camera_params.camPos[2];
        bump.Normalize();
        camera_params.camPos[0]+=bump.x*bump_factor;
        camera_params.camPos[1]+=bump.y*bump_factor;
        camera_params.camPos[2]+=bump.z*bump_factor;
      }


      camera_params.camPos[0] += direction.x*move_rate;
      camera_params.camPos[1] += direction.y*move_rate;
      camera_params.camPos[2] += direction.z*move_rate;

      camera_params.camTarget[0] = camera_params.camPos[0]+ direction.x*cam_dist;
      camera_params.camTarget[1] = camera_params.camPos[1]+ direction.y*cam_dist;
      camera_params.camTarget[2] = camera_params.camPos[2]+ direction.z*cam_dist;
      camera_params_array[j] = camera_params;

      if (rank==0) printProgress((j+1)/(double)frames,getTime()-time);
    }
    free(image);
  }

  #ifdef USE_MPI
    if (size >1){
    printf("brodcasting\n");
    MPI_Bcast(camera_params_array, sizeof(CameraParams)*frames,MPI_CHAR, 0, MPI_COMM_WORLD);
    }
  #endif

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! BEGIN HD MPI !!!!!!!!!!!!!!!!!!!!!!!!!
      // BEGIN PATHING TESTING
    renderer_params.width = 200;
    renderer_params.height = 200;
    pixelData max_pix_data;
    pixelData return_distances[5];
    int image_size = renderer_params.width * renderer_params.height;
    unsigned char *image = (unsigned char*)malloc(3*image_size*sizeof(unsigned char));
    init3D(&camera_params, &renderer_params);

    camera_params.camTarget[0] = 7;
    camera_params.camTarget[1] = 2;
    camera_params.camTarget[2] = 0;
    camera_params.camPos[0] = 10;
    camera_params.camPos[1] = 0;
    camera_params.camPos[2] = 0;
    camera_params.fov = 1;
    double time = getTime(); // was before pragma loop
    int i;
    for (i=rank; i<frames;i+=size){
      snprintf(renderer_params.file_name,80,"images/image%010d.bmp",i+1);
      camera_params= camera_params_array[i];
      camera_params.fov = 1;
      init3D(&camera_params, &renderer_params);
      renderFractal(camera_params, renderer_params, image,max_pix_data,return_distances);

      if (rank==0) printProgress((i+1)/(double)frames,getTime()-time);
      saveBMP(renderer_params.file_name, image, renderer_params.width, renderer_params.height);
    }
    if (rank==0) printProgress((double)1,getTime()-time);


    free(image);
    // free(return_distances);


  if (rank==0) printf("\n");
  #ifdef USE_MPI
   MPI_Finalize();
  #endif
  return 0;
}
