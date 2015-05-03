#include <stdio.h>
#include <GL/glut.h>
#include <math.h>
#include "global.h"
#include "sphere.h"

//
// Global variables
//
extern int win_width;
extern int win_height;

extern GLfloat frame[WIN_HEIGHT][WIN_WIDTH][3];  

extern float image_width;
extern float image_height;

extern Point eye_pos;
extern float image_plane;
extern RGB_float background_clr;
extern RGB_float null_clr;

extern Spheres *scene;

// light 1 position and color
extern Point light1;
extern float light1_ambient[3];
extern float light1_diffuse[3];
extern float light1_specular[3];

// global ambient term
extern float global_ambient[3];

// light decay parameters
extern float decay_a;
extern float decay_b;
extern float decay_c;

extern int shadow_on;
extern int reflect_on;
extern int step_max;

/////////////////////////////////////////////////////////////////////

/*********************************************************************
 * Phong illumination - you need to implement this!
 *********************************************************************/
RGB_float phong(Point q, Spheres *sph) {
	RGB_float color;
  
  RGB_float I_ga = {global_ambient[0], global_ambient[1], global_ambient[2]};
  RGB_float I_a = {light1_ambient[0], light1_ambient[1], light1_ambient[2]};
  RGB_float I_d = {light1_diffuse[0], light1_diffuse[1], light1_diffuse[2]};
  RGB_float I_s = {light1_specular[0], light1_specular[1], light1_specular[2]};

  float k_ga = sph->reflectance;
  RGB_float k_a = {(sph->mat_ambient)[0], (sph->mat_ambient)[1], (sph->mat_ambient)[2]};
  RGB_float k_d = {(sph->mat_diffuse)[0], (sph->mat_diffuse)[1], (sph->mat_diffuse)[2]};
  RGB_float k_s = {(sph->mat_specular)[0], (sph->mat_specular)[1], (sph->mat_specular)[2]};

  Vector n = sphere_normal(q, sph); normalize(&n);
  Vector l = get_vec(q, light1); normalize(&l);
  Vector r = vec_reflect(l, n); normalize(&r);
  Vector v = get_vec(q, eye_pos); normalize(&v);
  float N = sph->mat_shineness;
  float d = sqrt(pow(light1.x - q.x, 2)+pow(light1.y - q.y, 2)+pow(light1.y - q.y, 2));
  
  float a = decay_a;
  float b = decay_b;
  float c = decay_c;

  color = clr_add(clr_add(clr_scale(I_ga, k_ga), clr_times(I_a, k_a)), clr_scale(clr_add(clr_scale(clr_times(I_d, k_d), vec_dot(n, l)), clr_scale(clr_times(I_s, k_s), pow(vec_dot(r, v), N))), (1/(a+(b*d)+(c*(d*d))))));

  if (shadow_on == 1) {
    if (vec_dot(n, l) < 0) color = clr_scale(color, 0.5);
    else {
      //Test if sphere in way of light source
      Point hit; Spheres *sph_hit = intersect_scene(q, l, scene, hit);
      if (sph_hit) color = clr_scale(color, 0.5);
    }
  }

	return color;
}

/************************************************************************
 * This is the recursive ray tracer - you need to implement this!
 * You should decide what arguments to use.
 ************************************************************************/
RGB_float recursive_ray_trace(Point o, Vector v, int limit) {
	RGB_float color;

  Point hit; //next o
  Spheres *sph_hit = intersect_scene(o, v, scene, hit); //sph

  if (sph_hit) {
    Vector u = vec_reflect(vec_opposite(v), sphere_normal(hit, sph_hit)); //next v

    Point next_hit; Spheres *next_sph = intersect_scene(hit, u, scene, next_hit); //next sph
    
    if (limit == 0 || !next_sph) {
      color = phong(hit, sph_hit);
    }
    else {
      color = clr_add(
                phong(hit, sph_hit),
                clr_scale(
                  recursive_ray_trace(hit, u, limit--),
                  sph_hit->mat_shineness
                )
              );
      printf("%f %f %f \n", color.r, color.g, color.b );
    }
  }
  else color = background_clr;

	return color;
}

/*********************************************************************
 * This function traverses all the pixels and cast rays. It calls the
 * recursive ray tracer and assign return color to frame
 *
 * You should not need to change it except for the call to the recursive
 * ray tracer. Feel free to change other parts of the function however,
 * if you must.
 *********************************************************************/
void ray_trace() {
  int i, j;
  float x_grid_size = image_width / float(win_width);
  float y_grid_size = image_height / float(win_height);
  float x_start = -0.5 * image_width;
  float y_start = -0.5 * image_height;
  RGB_float ret_color;
  Point cur_pixel_pos;
  Vector ray;

  // ray is cast through center of pixel
  cur_pixel_pos.x = x_start + 0.5 * x_grid_size;
  cur_pixel_pos.y = y_start + 0.5 * y_grid_size;
  cur_pixel_pos.z = image_plane+2; //Why +2?

  for (i=0; i<win_height; i++) {
    for (j=0; j<win_width; j++) {
      ray = get_vec(eye_pos, cur_pixel_pos);

      //
      // You need to change this!!!
      //
      // ret_color = recursive_ray_trace();
      ret_color = background_clr; // just background for now

      //RGB_float clr = {float(i/(32*RES_SCALE)), 0, float(j/(32*RES_SCALE))}; //Checkerboard
      //ret_color = clr;

      // Parallel rays can be cast instead using below
      //
      // ray.x = ray.y = 0;
      // ray.z = -1.0;
      // ret_color = recursive_ray_trace(cur_pixel_pos, ray, 1);

      ray.x = ray.y = 0;
      ray.z = -1.0;

      
      if (reflect_on == 1) ret_color = recursive_ray_trace(cur_pixel_pos, ray, step_max); //Reflection
      else {
        Point hit; Spheres *sph_hit = intersect_scene(cur_pixel_pos, ray, scene, hit);
        if (sph_hit) ret_color = phong(hit, sph_hit); //No reflection
      }

      ret_color = clr_scale(ret_color, CLR_EXPOSURE);

      frame[i][j][0] = GLfloat(ret_color.r);
      frame[i][j][1] = GLfloat(ret_color.g);
      frame[i][j][2] = GLfloat(ret_color.b);

      cur_pixel_pos.x += x_grid_size;
    }

    cur_pixel_pos.y += y_grid_size;
    cur_pixel_pos.x = x_start;
  }
}
