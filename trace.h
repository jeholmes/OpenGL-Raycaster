void ray_trace();
RGB_float phong(Point, Spheres *);
Vector transmit_vector_in(Point, Vector, Spheres *);
Vector transmit_vector_out(Point, Vector, Spheres *);
Vector transmit(Point, Vector, Spheres *, Point &);