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

#include "color.h"
#include "mandelbox.h"
#include "camera.h"
#include "vector3d.h"
#include "3d.h"
#ifdef _OPENMP
#include <omp.h>
#endif
#ifdef _OPENACC
#include <openacc.h>
#include <curand.h>
#endif

extern double getTime();
extern void   printProgress( double perc, double time );
extern void init3D       (CameraParams *camera_params, const RenderParams *renderer_params);

extern void rayMarch (const RenderParams &render_params, const vec3 &from, const vec3  &to, double eps, pixelData &pix_data);

extern vec3 getColour(const pixelData &pixData, const RenderParams &render_params,
		      const vec3 &from, const vec3  &direction);

void renderFractal(const CameraParams &camera_params, const RenderParams &renderer_params, unsigned char* image)
{


  const double eps = pow(10.0, renderer_params.detail);
  double farPoint[3];
  vec3 to, from;


  from.SetDoublePoint(camera_params.camPos);

  const int height = renderer_params.height;
  const int width  = renderer_params.width;

  pixelData pix_data;


  #pragma omp parallel\
  default(shared)\
  private(to, pix_data)\
  shared(image,camera_params, renderer_params, from, farPoint)
  {
  vec3 color;
  int i,j,k;
  #pragma omp for schedule (guided)
  for(j = 0; j < height; j++)
  {

      //for each column pixel in the row

    for(i = 0; i <width; i++)
  	{
  	  // get point on the 'far' plane
  	  // since we render one frame only, we can use the more specialized method
  	  UnProject(i, j, camera_params, farPoint);

  	  // to = farPoint - camera_params.camPos
  	  to = SubtractDoubleDouble(farPoint,camera_params.camPos);
  	  to.Normalize();

  	  //render the pixel
  	  rayMarch(renderer_params, from, to, eps, pix_data);


  	  //get the colour at this pixel
  	  color = getColour(pix_data, renderer_params, from, to);

  	  //save colour into texture
  	  k = (j * width + i)*3;
  	  image[k+2] = (unsigned char)(color.x * 255);
  	  image[k+1] = (unsigned char)(color.y * 255);
  	  image[k]   = (unsigned char)(color.z * 255);
  	}
    //if (ID==0) printProgress((j+1)/(double)height,getTime()-time);
  }
  //if (ID==0) printf("\n rendering done:\n");
}//end parallel

}

// in this function we do not calculate the color of each pixel and we also return the max distance pix data as well as the distances in a cross
void renderFractal_for_path(const CameraParams &camera_params, const RenderParams &renderer_params, pixelData& max_pix_data,pixelData *return_distances)
{


  const double eps = pow(10.0, renderer_params.detail);
  double farPoint[3];
  vec3 to, from;


  from.SetDoublePoint(camera_params.camPos);

  const int height = renderer_params.height;
  const int width  = renderer_params.width;

  pixelData pix_data;
  max_pix_data.distance=0;

  #pragma omp parallel\
  default(shared)\
  private(to, pix_data)\
  shared(camera_params, renderer_params, from, farPoint,max_pix_data,return_distances)
  {
    int i,j;
    #pragma omp for schedule (guided)
    for(j = 0; j < height; j++)
    {
        //for each column pixel in the row
      for(i = 0; i <width; i++)
      {
        // get point on the 'far' plane
        // since we render one frame only, we can use the more specialized method
        UnProject(i, j, camera_params, farPoint);

        // to = farPoint - camera_params.camPos
        to = SubtractDoubleDouble(farPoint,camera_params.camPos);
        to.Normalize();

        //render the pixel
        rayMarch(renderer_params, from, to, eps, pix_data);

        #pragma omp critical
        if  (pix_data.distance > max_pix_data.distance){
          max_pix_data = pix_data;
        }

      }
    }

    int divisor = 2;
    const int denom =2*divisor;
    const int h_width = width/2; // half
    const int h_height = height/2; // half
    const int q_width = width/denom; // half
    const int q_height = height/denom; // half
    const int left = divisor-1;
    const int right = divisor+1;
    int camera_pos_width[5]  = {h_width,left*q_width,right*q_width,h_width,h_width};
    int camera_pos_height[5] = {h_height,h_height,h_height, right*q_height,left*q_height};
      #pragma omp for schedule (guided)
      for(j = 0; j < 5; j++)
      {
        // get point on the 'far' plane
        // since we render one frame only, we can use the more specialized method
        UnProject(camera_pos_width[j], camera_pos_height[j], camera_params, farPoint);

        // to = farPoint - camera_params.camPos
        to = SubtractDoubleDouble(farPoint,camera_params.camPos);
        to.Normalize();

        //render the pixel
        rayMarch(renderer_params, from, to, eps, pix_data);
        // #pragma omp critical
        {
        return_distances[j].distance = pix_data.distance;
        return_distances[j].hit = pix_data.hit;
        return_distances[j].escaped = pix_data.escaped;
        }
      }
    }//end parallel
}

void generateCameraPath(CameraParams &camera_params, RenderParams &renderer_params, CameraParams *camera_params_array, int frames, double camera_speed)
{
  printf("Generating Camera Path\n");
  renderer_params.width = 25;
  renderer_params.height = 25;

  camera_params.fov = 0.4; // 0.4

  vec3 direction,direction2,bump;
  direction2.x = 0;
  direction2.y = 0;
  direction2.z = 0;


  pixelData max_pix_data;

  pixelData return_distances[5];
  double time = getTime();
  double new_dir_weight;
  double move_rate, cam_dist;
  double dir_error,dir_error2;
  // int last_direction_change_reset = 2;
  // int last_direction_change = 0;
  double smooth_speed,new_distance,old_distance;
  old_distance = 0;
  int j;
  for (j=0;j<frames;j++){
    init3D(&camera_params, &renderer_params);

    renderFractal_for_path(camera_params, renderer_params,max_pix_data,return_distances);
    new_distance = return_distances[0].distance;
    smooth_speed = 0.5* old_distance + 0.5*new_distance;
    // if (new_distance > old_distance) smooth_speed = 0.5* old_distance + 0.5*new_distance;
    // else smooth_speed = 0.3* old_distance + 0.8*new_distance;
    old_distance = new_distance;

    move_rate = camera_speed*smooth_speed; //0.001
    cam_dist = move_rate+1;


    direction.x = max_pix_data.hit.x-camera_params.camPos[0];
    direction.y = max_pix_data.hit.y-camera_params.camPos[1];
    direction.z = max_pix_data.hit.z-camera_params.camPos[2];
    // last_direction_change = last_direction_change_reset;

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

    printProgress((j+1)/(double)frames,getTime()-time);
  }
  printf("\n");
}
