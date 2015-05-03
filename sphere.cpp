//#include "include/Angel.h"
#include "sphere.h"
#include <stdlib.h>
#include <math.h>

/**********************************************************************
 * This function intersects a ray with a given sphere 'sph'. You should
 * use the parametric representation of a line and do the intersection.
 * The function should return the parameter value for the intersection, 
 * which will be compared with others to determine which intersection
 * is closest. The value -1.0 is returned if there is no intersection
 *
 * If there is an intersection, the point of intersection should be
 * stored in the "hit" variable
 **********************************************************************/
float intersect_sphere(Point o, Vector u, Spheres *sph, Point &hit) {
  float final_dist = -1.0; 
  Vector oc = get_vec(o, sph->center);  
  Point p = get_point(o, vec_scale(u, vec_dot(oc,u)/vec_len(u))); 
  float dist = sqrt(pow(sph->radius, 2) - pow(vec_len(get_vec(p, sph->center)), 2));  
  if (vec_dot(oc, u) < 0) {   
    if (vec_len(oc) < sph->radius) {    
      final_dist = dist - vec_len(get_vec(p, o));
      hit = get_point(o, vec_scale(u, final_dist)); 
    }
    else if (vec_len(oc) == sph->radius) hit = o; 
  }
  else {  
    if (vec_len(get_vec(p, sph->center)) < sph->radius) { 
      if (vec_len(oc) > sph->radius) final_dist = vec_len(get_vec(p, o)) - dist; 
      else final_dist = vec_len(get_vec(p, o)) + dist;
      hit = get_point(o, vec_scale(u, final_dist));
    }
  }
  return final_dist;
}

float intersect_plane(Point o, Vector u, Spheres *sph, Point &hit) {
  float final_dist = -1.0;
  Vector oc = get_vec(o, sph->center);
  if (vec_dot(sph->plane_normal, u) < 0) {
    float distc = vec_dot(oc, sph->plane_normal)/vec_dot(sph->plane_normal, u);
    Point hitc = get_point(o, vec_scale(u, distc));
    if (hitc.x >= (sph->center).x-(sph->plane_scale) &&
        hitc.x <= (sph->center).x+(sph->plane_scale) &&
        hitc.z >= (sph->center).z-(sph->plane_scale) &&
        hitc.z <= (sph->center).z+(sph->plane_scale)) {
      final_dist = distc;
      hit = hitc;
    }
  }
  return final_dist;
}

/*********************************************************************
 * This function returns a pointer to the sphere object that the
 * ray intersects first; NULL if no intersection. You should decide
 * which arguments to use for the function. For exmaple, note that you
 * should return the point of intersection to the calling function.
 **********************************************************************/
Spheres *intersect_scene(Point o, Vector u, Spheres *sph, Point &hit) {
  Spheres *output = NULL;
  float min_dist = -1.0;
  float dist = -1.0;
  normalize(&u);
  while (sph) {
    if (sph->radius > 0) dist = intersect_sphere(o, u, sph, hit);
    else if (sph->plane_scale > 0) dist = intersect_plane(o, u, sph, hit);
    if (dist >= 0 && (min_dist < 0 || (min_dist >= 0 && dist < min_dist))) {
          min_dist = dist;
          output = sph;
    }
    sph = sph->next;
  }
	return output;
}

/*****************************************************
 * This function adds a sphere into the sphere list
 *
 * You need not change this.
 *****************************************************/
Spheres *add_sphere(Spheres *slist, Point ctr, float rad, float amb[],
		    float dif[], float spe[], float shine, 
		    float refl, float refr, float trans, int sindex, Vector norm, float scale) {
  Spheres *new_sphere;

  new_sphere = (Spheres *)malloc(sizeof(Spheres));
  new_sphere->index = sindex;
  new_sphere->center = ctr;
  new_sphere->radius = rad;
  (new_sphere->mat_ambient)[0] = amb[0];
  (new_sphere->mat_ambient)[1] = amb[1];
  (new_sphere->mat_ambient)[2] = amb[2];
  (new_sphere->mat_diffuse)[0] = dif[0];
  (new_sphere->mat_diffuse)[1] = dif[1];
  (new_sphere->mat_diffuse)[2] = dif[2];
  (new_sphere->mat_specular)[0] = spe[0];
  (new_sphere->mat_specular)[1] = spe[1];
  (new_sphere->mat_specular)[2] = spe[2];
  new_sphere->mat_shineness = shine;
  new_sphere->reflectance = refl;
  new_sphere->refraction = refr;
  new_sphere->transparency = trans;
  new_sphere->next = NULL;

  new_sphere->plane_normal = norm;
  new_sphere->plane_scale = scale;

  if (slist == NULL) { // first object
    slist = new_sphere;
  } else { // insert at the beginning
    new_sphere->next = slist;
    slist = new_sphere;
  }

  return slist;
}

/******************************************
 * computes a sphere normal - done for you
 ******************************************/
Vector sphere_normal(Point q, Spheres *sph) {
  Vector rc;

  rc = get_vec(sph->center, q);
  normalize(&rc);
  return rc;
}