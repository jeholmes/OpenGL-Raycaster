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

extern float refraction_index;

extern int shadow_on;
extern int reflect_on;
extern int board_on;
extern int refract_on;
extern int diffuse_on;
extern int ss_on;

extern int step_max;

extern int res_scale;

/////////////////////////////////////////////////////////////////////

/*********************************************************************
 * Phong illumination - you need to implement this!
 *********************************************************************/
RGB_float phong(Point q, Spheres *sph) {
	RGB_float color;
  
  if (sph->radius > 0) {
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

    float att = 1/(a+(b*d)+(c*(d*d)));
    RGB_float ga_term = clr_scale(I_ga, k_ga);
    RGB_float a_term = clr_times(I_a, k_a);

    RGB_float d_term = clr_times(I_d, k_d);
    float d_scalar = 0.0;
    if (vec_dot(n, l) >= 0) d_scalar = vec_dot(n, l);
    RGB_float d_scaled = clr_scale(d_term, d_scalar);

    RGB_float s_term = clr_times(I_s, k_s);
    float s_scalar = 0.0;
    if (vec_dot(r, v) >= 0) s_scalar = pow(vec_dot(r, v), N);
    RGB_float s_scaled = clr_scale(s_term, s_scalar);

    color = clr_add(clr_add(ga_term, a_term), clr_scale(clr_add(d_scaled, s_scaled), (att)));

    if (shadow_on == 1) {
      if (vec_dot(n, l) < 0) color = clr_add(clr_scale(I_ga, k_ga), clr_times(I_a, k_a));
      else {
        //Test if sphere in way of light source
        Point hit; Spheres *sph_hit = intersect_scene(shift_point(q, n), l, scene, hit);
        if (sph_hit) color = clr_add(clr_scale(I_ga, k_ga), clr_times(I_a, k_a));
      }
    }
  }
  else {

    float x_val = floor(fmod((4*(q.x-(sph->center).x))/(sph->plane_scale), 2.0));
    float z_val = floor(fmod((4*(q.z-(sph->center).z))/(sph->plane_scale), 2.0));

    if ((x_val == 1.0 && z_val == 1.0) || (x_val == -1.0 && z_val == -1.0) || 
        (x_val == 1.0 && z_val == -1.0) || (x_val == -1.0 && z_val == 1.0) ||
        (x_val == 2.0 && z_val == 2.0) || (x_val == -2.0 && z_val == -2.0) || 
        (x_val == 2.0 && z_val == -2.0) || (x_val == -2.0 && z_val == 2.0) ||
        (x_val == 0.0 && z_val == 2.0) || (x_val == 0.0 && z_val == -2.0) || 
        (x_val == 0.0 && z_val == -2.0) || (x_val == 0.0 && z_val == 2.0) ||
        (x_val == 2.0 && z_val == 0.0) || (x_val == -2.0 && z_val == 0.0) || 
        (x_val == 2.0 && z_val == 0.0) || (x_val == -2.0 && z_val == 0.0) ||
        (x_val == 0.0 && z_val == 0.0))  {
      color.r = 0.5;
      color.g = 0.5;
      color.b = 0.5;
    }
    else {
      color.r = 0.0;
      color.g = 0.0;
      color.b = 0.0;
    }
    if (shadow_on == 1) {
      Vector n = sph->plane_normal; normalize(&n);
      Vector l = get_vec(q, light1); normalize(&l);
      Point hit; Spheres *sph_hit = intersect_scene(shift_point(q, n), l, scene, hit);
      if (sph_hit) color = clr_scale(color, 0.5);
    }
  }

	return color;
}

//Functions for transmitting vector for refraction

Vector transmit_vector_in(Point o, Vector u, Spheres *sph) {
  Vector rc;
  Vector n = vec_opposite(sphere_normal(o, sph));
  float refr = sph->refraction/refraction_index;
  normalize(&u); normalize(&n);
  rc.x = (u.x + ((refr-1)*n.x))/refr;
  rc.y = (u.y + ((refr-1)*n.y))/refr;
  rc.z = (u.z + ((refr-1)*n.z))/refr;
  normalize(&rc);
  return rc;
}

Vector transmit_vector_out(Point o, Vector u, Spheres *sph) {
  Vector rc;
  Vector n = sphere_normal(o, sph);
  float refr = sph->refraction/refraction_index;
  normalize(&u); normalize(&n);
  rc.x = (refr*u.x)-((refr-1)*n.x);
  rc.y = (refr*u.y)-((refr-1)*n.y);
  rc.z = (refr*u.z)-((refr-1)*n.z);
  normalize(&rc);
  return rc;
}

Vector transmit(Point o, Vector u, Spheres *sph, Point &q) {
  Vector u_in = transmit_vector_in(o, u, sph);
  intersect_sphere(shift_point(o, u), u_in, sph, q);
  Vector u_out = transmit_vector_out(q, u_in, sph);
  q = shift_point(q, u_out);
  return u_out;
}

/************************************************************************
 * This is the recursive ray tracer - you need to implement this!
 * You should decide what arguments to use.
 ************************************************************************/
RGB_float recursive_ray_trace(Point p, Vector d, int step) {
  if (step > step_max) return background_clr;

  Point q;
  Spheres *intersection = intersect_scene(p, d, scene, q);

  if (equal_points(q, light1)) {
    RGB_float output = {light1_specular[0], light1_specular[1], light1_specular[2]};
    return output;
  }
  if (!intersection) return background_clr;
  if (intersection->plane_scale > 0 && board_on == 0) return background_clr;

  RGB_float output;

  Vector r;
  if (intersection->radius > 0) r = vec_reflect(vec_opposite(d), sphere_normal(q, intersection));
  else if (intersection->plane_scale > 0) r = vec_reflect(vec_opposite(d), intersection->plane_normal);

  RGB_float local = phong(q, intersection);
  RGB_float reflected = clr_scale(recursive_ray_trace(shift_point(q, r), r, step+1), intersection->reflectance);

  if (diffuse_on == 1) {
    Vector n;
    if (intersection->radius > 0) n = sphere_normal(q, intersection);
    else if (intersection->plane_scale > 0) n = intersection->plane_normal;
    normalize(&n);

    for (int i = 0; i < DIFFUSE; i++) {
      Vector temp = vec_random(n);
      Point hit;
      Spheres *sph_hit;
      RGB_float to_add;
      sph_hit = intersect_scene(p, temp, scene, hit);

      if (sph_hit) to_add = clr_scale(phong(hit, sph_hit), intersection->reflectance);
      else to_add = clr_scale(background_clr, intersection->reflectance);

      clr_add(reflected, to_add);
    }
    clr_scale(reflected, (float)1/(DIFFUSE+1));
  }

  output = clr_add(local, reflected);

  if (refract_on == 1 && intersection->radius > 0) {
    Point t_q;
    Vector t = transmit(q, d, intersection, t_q);
    RGB_float transmitted = clr_scale(recursive_ray_trace(t_q, t, step+1), intersection->transparency);
    output = clr_add(output, transmitted);
  }
  
  return output;
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

  RGB_float ret_color_nw;
  RGB_float ret_color_ne;
  RGB_float ret_color_sw;
  RGB_float ret_color_se;
  Point pixel_nw;
  Point pixel_ne;
  Point pixel_sw;
  Point pixel_se; 
  Vector ray_nw;
  Vector ray_ne;
  Vector ray_sw;
  Vector ray_se;

  // ray is cast through center of pixel
  cur_pixel_pos.x = x_start + 0.5 * x_grid_size;
  cur_pixel_pos.y = y_start + 0.5 * y_grid_size;
  cur_pixel_pos.z = image_plane;

  // Super sampling pixel positions
  pixel_nw.x = x_start + 0.25 * x_grid_size;
  pixel_nw.y = y_start + 0.75 * y_grid_size;
  pixel_nw.z = image_plane;
  pixel_ne.x = x_start + 0.75 * x_grid_size;
  pixel_ne.y = y_start + 0.75 * y_grid_size;
  pixel_ne.z = image_plane;
  pixel_sw.x = x_start + 0.25 * x_grid_size;
  pixel_sw.y = y_start + 0.25 * y_grid_size;
  pixel_sw.z = image_plane;
  pixel_se.x = x_start + 0.75 * x_grid_size;
  pixel_se.y = y_start + 0.25 * y_grid_size;
  pixel_se.z = image_plane;

  for (i=0; i<win_height; i++) {
    for (j=0; j<win_width; j++) {
      ray = get_vec(eye_pos, cur_pixel_pos);

      //Super sampling rays
      ray_nw = get_vec(eye_pos, pixel_nw);
      ray_ne = get_vec(eye_pos, pixel_ne);
      ray_sw = get_vec(eye_pos, pixel_sw);
      ray_se = get_vec(eye_pos, pixel_se);

      ret_color = background_clr; // just background for now

      if (reflect_on == 1) {
        ret_color = recursive_ray_trace(eye_pos, ray, 0); //Reflection
        
        if (ss_on == 1) {
          ret_color_nw = recursive_ray_trace(eye_pos, ray_nw, 0);
          ret_color_ne = recursive_ray_trace(eye_pos, ray_ne, 0);
          ret_color_sw = recursive_ray_trace(eye_pos, ray_sw, 0);
          ret_color_se = recursive_ray_trace(eye_pos, ray_se, 0);
          RGB_float n_col = clr_add(ret_color_nw, ret_color_ne);
          RGB_float s_col = clr_add(ret_color_sw, ret_color_se);
          RGB_float diff_col = clr_add(n_col, s_col);
          ret_color = clr_add(ret_color, diff_col);
          ret_color = clr_scale(ret_color, 1/5.0);
        }
      }
      else {
        Point hit; Spheres *sph_hit = intersect_scene(eye_pos, ray, scene, hit);
        if ((sph_hit && sph_hit->radius > 0) || ((sph_hit && sph_hit->plane_scale > 0) && board_on == 1)) ret_color = phong(hit, sph_hit); //No reflection
        
        if (ss_on == 1) {
          Point hit_nw; Spheres *sph_hit_nw = intersect_scene(eye_pos, ray_nw, scene, hit_nw);
          if ((sph_hit_nw && sph_hit_nw->radius > 0) || ((sph_hit_nw && sph_hit_nw->plane_scale > 0) && board_on == 1)) ret_color_nw = phong(hit_nw, sph_hit_nw);
          Point hit_ne; Spheres *sph_hit_ne = intersect_scene(eye_pos, ray_ne, scene, hit_ne);
          if ((sph_hit_ne && sph_hit_ne->radius > 0) || ((sph_hit_ne && sph_hit_ne->plane_scale > 0) && board_on == 1)) ret_color_ne = phong(hit_ne, sph_hit_ne);
          Point hit_sw; Spheres *sph_hit_sw = intersect_scene(eye_pos, ray_sw, scene, hit_sw);
          if ((sph_hit_sw && sph_hit_sw->radius > 0) || ((sph_hit_sw && sph_hit_sw->plane_scale > 0) && board_on == 1)) ret_color_sw = phong(hit_sw, sph_hit_sw);
          Point hit_se; Spheres *sph_hit_se = intersect_scene(eye_pos, ray_se, scene, hit_se);
          if ((sph_hit_se && sph_hit_se->radius > 0) || ((sph_hit_se && sph_hit_se->plane_scale > 0) && board_on == 1)) ret_color_se = phong(hit_se, sph_hit_se);
          RGB_float n_col = clr_add(ret_color_nw, ret_color_ne);
          RGB_float s_col = clr_add(ret_color_sw, ret_color_se);
          RGB_float diff_col = clr_add(n_col, s_col);
          ret_color = clr_add(ret_color, diff_col);
          ret_color = clr_scale(ret_color, 1/5.0);        
        }
      }

      frame[i][j][0] = GLfloat(ret_color.r);
      frame[i][j][1] = GLfloat(ret_color.g);
      frame[i][j][2] = GLfloat(ret_color.b);

      cur_pixel_pos.x += x_grid_size;
    }

    cur_pixel_pos.y += y_grid_size;
    cur_pixel_pos.x = x_start;
  }
}
