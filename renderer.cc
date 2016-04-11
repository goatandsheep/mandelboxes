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

#define MAGNITUDE(m,p)  ({ m=sqrt( p.x*p.x + p.y*p.y + p.z*p.z ); })

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


  #pragma omp parallel default(shared) private(to, pix_data) shared(image, camera_params, renderer_params, from, farPoint)
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
void renderFractal_for_path(const CameraParams &camera_params, const RenderParams &renderer_params, pixelData& max_pix_data,pixelData& min_pix_data,pixelData *return_distances)
{


  const double eps = pow(10.0, renderer_params.detail);
  double farPoint[3];
  vec3 to, from;


  from.SetDoublePoint(camera_params.camPos);

  const int height = renderer_params.height;
  const int width  = renderer_params.width;

  pixelData pix_data;
  max_pix_data.distance=0;
  min_pix_data.distance=100;

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
        #pragma omp critical
        if  (pix_data.distance < min_pix_data.distance){
          min_pix_data = pix_data;
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
        #pragma omp critical
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
  printf("Generating Camera Path (Serial)\n");
  renderer_params.width = 25;
  renderer_params.height = 25;

  camera_params.fov = 1; // 0.4


  vec3 direction,bump,direction_new;
  direction.x = camera_params.camTarget[0];
  direction.y = camera_params.camTarget[1];
  direction.z = camera_params.camTarget[2];

  pixelData max_pix_data,min_pix_data;
  pixelData return_distances[5];
  double time = getTime();
  double smooth_direction,dir_error,max_turn_speed;
  max_turn_speed = 0.05; // higher values induce more jitter but allow for a more agile camera

  double move_rate, cam_dist;
  double smooth_speed,new_distance;
  smooth_speed = 1;

  double bump_factor;
  int j;
  for (j=0;j<frames;j++){
    init3D(&camera_params, &renderer_params);

    renderFractal_for_path(camera_params, renderer_params,max_pix_data,min_pix_data,return_distances);

    // MOVEMENT SPEED
    new_distance = return_distances[0].distance;//min_pix_data.distance;//
    if (new_distance > 0.1) smooth_speed *= 1.1;
    else smooth_speed *= 0.5;
    if (smooth_speed > 1) smooth_speed = 1;
    else if (smooth_speed < 0.01) smooth_speed = 0.01;

    move_rate = camera_speed*smooth_speed;
    cam_dist = move_rate+1;


    // CALCULATE DIRECTION
    direction_new.x = max_pix_data.hit.x-camera_params.camPos[0];
    direction_new.y = max_pix_data.hit.y-camera_params.camPos[1];
    direction_new.z = max_pix_data.hit.z-camera_params.camPos[2];
    direction_new.Normalize();

    MAGNITUDE(dir_error,(direction-direction_new));
    dir_error = 10*(dir_error/max_pix_data.distance);

    if (dir_error > 1) dir_error = 1;
    else if (dir_error < 0.01) dir_error = 0; //0.01
    smooth_direction = max_turn_speed*dir_error;
    direction = direction*(1-smooth_direction) + direction_new*smooth_direction;
    direction.Normalize();

    // FLIP CAMERA
    if (max_pix_data.distance < 0.0001){ // just in case, do not need in final version
      direction.x *= -1;
      direction.y *= -1;
      direction.z *= -1;
    }

    if (min_pix_data.distance < 0.001) bump_factor = min_pix_data.distance/10; //move_rate/10; //30
    else bump_factor = 0;

    bump.x = camera_params.camPos[0] - min_pix_data.hit.x;
    bump.y = camera_params.camPos[1] - min_pix_data.hit.y;
    bump.z = camera_params.camPos[2] - min_pix_data.hit.z;
    bump.Normalize();

    camera_params.camPos[0] += direction.x*move_rate + bump.x*bump_factor;
    camera_params.camPos[1] += direction.y*move_rate + bump.y*bump_factor;
    camera_params.camPos[2] += direction.z*move_rate + bump.z*bump_factor;

    camera_params.camTarget[0] = camera_params.camPos[0]+ direction.x*cam_dist;
    camera_params.camTarget[1] = camera_params.camPos[1]+ direction.y*cam_dist;
    camera_params.camTarget[2] = camera_params.camPos[2]+ direction.z*cam_dist;
    camera_params_array[j] = camera_params;

    printProgress((j+1)/(double)frames,getTime()-time);
  }
  printf("\n");
}
