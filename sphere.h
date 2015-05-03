/**********************************************************************
 * Some stuff to handle spheres
 **********************************************************************/
#include "vector.h"

typedef struct sphere {
  int index;               // identifies a sphere; must be greater than 0

  Point center;
  float radius;

  float mat_ambient[3];    // material property used in Phong model
  float mat_diffuse[3];
  float mat_specular[3];
  float mat_shineness;

  float reflectance;       // this number [0,1] determines how much 
                           // reflected light contributes to the color
                           // of a pixel
  float transparency;
  float refraction;

  //Plane values
  Vector plane_normal;
  float plane_scale;

  struct sphere *next;
} Spheres;   // a list of spheres

// intersect ray with sphere
float intersect_sphere(Point, Vector, Spheres *, Point &);
// intersect ray with scene
Spheres *intersect_scene(Point, Vector, Spheres *, Point &);
// return the unit normal at a point on sphere
Vector sphere_normal(Point, Spheres *);
// add a sphere to the sphere list
Spheres *add_sphere(Spheres *, Point, float, float [], float [], float [], float, float, float, float, int, Vector, float);

