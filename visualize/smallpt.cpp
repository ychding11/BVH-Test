#include <math.h>   // smallpt, a Path Tracer by Kevin Beason, 2008
#include <stdlib.h> // Make : g++ -O3 -fopenmp smallpt.cpp -o smallpt
#include <stdio.h>  //        Remove "-fopenmp" for g++ version < 4.2

#include "smallpt.h"

#ifndef M_PI
#define M_PI  3.1415926
#endif

struct Ray { Vec o, d; Ray(Vec o_, Vec d_) : o(o_), d(d_) {} };
enum Refl_t { DIFF, SPEC, REFR };  // material types, used in radiance()
struct Sphere {
  double rad;       // radius
  Vec p, e, c;      // position, emission, color
  Refl_t refl;      // reflection type (DIFFuse, SPECular, REFRactive)
  Sphere(double rad_, Vec p_, Vec e_, Vec c_, Refl_t refl_):
    rad(rad_), p(p_), e(e_), c(c_), refl(refl_) {}
  double intersect(const Ray &r) const { // returns distance, 0 if nohit
    Vec op = p-r.o; // Solve t^2*d.d + 2*t*(o-p).d + (o-p).(o-p)-R^2 = 0
    double t, eps=1e-4, b=op.dot(r.d), det=b*b-op.dot(op)+rad*rad;
    if (det<0) return 0; else det=sqrt(det);
    return (t=b-det)>eps ? t : ((t=b+det)>eps ? t : 0);
  }
};
Sphere spheres[] =
{
 //Scene: radius, position, emission, color, material
  Sphere(1e5, Vec( 1e5+1,40.8,81.6), Vec(),Vec(.75,.25,.25),DIFF),//Left
  Sphere(1e5, Vec(-1e5+99,40.8,81.6),Vec(),Vec(.25,.25,.75),DIFF),//Rght
  Sphere(1e5, Vec(50,40.8, 1e5),     Vec(),Vec(.75,.75,.75),DIFF),//Back
  Sphere(1e5, Vec(50,40.8,-1e5+170), Vec(),Vec(),           DIFF),//Frnt
  Sphere(1e5, Vec(50, 1e5, 81.6),    Vec(),Vec(.75,.75,.75),DIFF),//Botm
  Sphere(1e5, Vec(50,-1e5+81.6,81.6),Vec(),Vec(.75,.75,.75),DIFF),//Top
  Sphere(16.5,Vec(27,16.5,47),       Vec(),Vec(1,1,1)*.999, SPEC),//Mirr
  Sphere(16.5,Vec(73,16.5,78),       Vec(),Vec(1,1,1)*.999, REFR),//Glas
  Sphere(600, Vec(50,681.6-.27,81.6),Vec(12,12,12),  Vec(), DIFF) //Lite
};
inline double clamp(double x){ return x<0 ? 0 : x>1 ? 1 : x; }
inline int toInt(double x){ return int(pow(clamp(x),1/2.2)*255+.5); }
inline bool intersect(const Ray &r, double &t, int &id)
{
  double n=sizeof(spheres)/sizeof(Sphere), d, inf=t=1e20;
  for(int i=int(n);i--;) if((d=spheres[i].intersect(r))&&d<t){t=d;id=i;}
  return t<inf;
}

#include <random>
#include <iostream>
std::default_random_engine generator;
std::uniform_real_distribution<double> distr(0.0, 1.0);
double erand48(unsigned short* X)
{
    return distr(generator);
}
Vec radiance(const Ray &r, int depth, unsigned short *Xi)
{
  double t;                               // distance to intersection
  int id=0;                               // id of intersected object
  if (!intersect(r, t, id)) return Vec(); // if miss, return black
  const Sphere &obj = spheres[id];        // the hit object
  Vec x=r.o+r.d*t,
      n=(x-obj.p).norm(),
      nl=n.dot(r.d)<0 ? n : n*-1,
      f=obj.c;
  double p = f.x>f.y && f.x>f.z ? f.x : f.y>f.z ? f.y : f.z; // max refl

  if (++depth > 4 && depth <= 6)
  {
      if (erand48(Xi)<p) f=f*(1/p);
      else return obj.e;
  }
  else if (depth > 4)
  {
      return obj.e;
  }

	if (obj.refl == DIFF) // Ideal DIFFUSE reflection
	{                  
		double r1=2*M_PI*erand48(Xi),
			r2=erand48(Xi),
			r2s=sqrt(r2);
		Vec w=nl,
			u=((fabs(w.x)>.1? Vec(0,1) : Vec(1)) % w).norm(),
			v=w % u;
		Vec d = (u*cos(r1)*r2s + v*sin(r1)*r2s + w*sqrt(1-r2)).norm();
		return obj.e + f.mult(radiance(Ray(x,d),depth,Xi));
	}
	else if (obj.refl == SPEC)            // Ideal SPECULAR reflection
	{
		return obj.e + f.mult(radiance(Ray(x,r.d-n*2*n.dot(r.d)),depth,Xi));
	}
	else
	{
		Ray reflRay(x, r.d - n*2*n.dot(r.d));     // Ideal dielectric REFRACTION
		bool into = n.dot(nl) > 0;                // Ray from outside going in?
		double nc = 1,       // Air
				nt = 1.5,   // IOR  Glass
				nnt=into ? nc/nt : nt/nc,
				ddn=r.d.dot(nl),
			    cos2t;
		if ((cos2t = 1 - nnt * nnt*(1 - ddn * ddn)) < 0)    // Total internal reflection
		{
			return obj.e + f.mult(radiance(reflRay,depth,Xi));
		}
		Vec tdir = (r.d*nnt - n*((into?1:-1) * (ddn*nnt+sqrt(cos2t)))).norm();
		double a=nt-nc,
			b=nt+nc,
			R0=a*a/(b*b),
			c = 1-(into?-ddn:tdir.dot(n));
		double Re=R0+(1-R0)*c*c*c*c*c,
			Tr=1-Re,
			P=.25+.5*Re,
			RP=Re/P,
			TP=Tr/(1-P);
		return obj.e + f.mult(depth>2 ? (erand48(Xi)<P ?   // Russian roulette
		radiance(reflRay,depth,Xi)*RP:radiance(Ray(x,tdir),depth,Xi)*TP) :
		radiance(reflRay,depth,Xi)*Re+radiance(Ray(x,tdir),depth,Xi)*Tr);
	}
}

#include <omp.h>

//// https://docs.microsoft.com/en-us/cpp/build/reference/openmp-enable-openmp-2-0-support?view=vs-2019
float* smallpt(std::string &log, int w = 1024, int h = 768, int samps = 1)
{
  Ray cam(Vec(50,52,295.6), Vec(0,-0.042612,-1).norm()); // cam pos, dir
  Vec cx = Vec(w*.5135 / h),
	  cy = (cx%cam.d).norm()*.5135,
	  r;

  static Vec  *c = new Vec[w * h];
  static float *data = new float[w * h * 3];
  static uint64_t count = 0;
  std::ostringstream ss;

  ss << " Sample Number: " << count << "\n";
  log = ss.str();

  if (count >= samps) // render done 
  {
	  return data;
  }
  
  #pragma omp parallel for schedule(static, 1) private(r) private(ss)       // OpenMP
  for (int y = 0; y < h; y++) // Loop over image rows
  {
#if 0 
	ss << " Thread Number: " << omp_get_num_threads()<<  "\t Thread ID: " << omp_get_thread_num() << "\n";
    #pragma omp critical
	{
	log += ss.str();
	}
#endif
    for (unsigned short x = 0, Xi[3]={0,0,y*y*y}; x < w; x++)   // Loop cols
      for (int sy=0, i=(y) * w + x; sy < 2; sy++)     // 2x2 subpixel rows
        for (int sx = 0; sx < 2; sx++, r=Vec())			  // 2x2 subpixel cols
        {        
          //for (int s = 0; s < samps; s++)
          {
            double r1=2*erand48(Xi), dx=r1<1 ? sqrt(r1)-1: 1-sqrt(2-r1);
            double r2=2*erand48(Xi), dy=r2<1 ? sqrt(r2)-1: 1-sqrt(2-r2);
            Vec d = cx*( ( (sx+.5 + dx)/2 + x) / w - .5) +
                    cy*( ( (sy+.5 + dy)/2 + y) / h - .5) + cam.d;
            //r = r + radiance(Ray(cam.o+d*140, d.norm()), 0, Xi) * (1./samps);
            r = r + radiance(Ray(cam.o+d*140, d.norm()), 0, Xi);
          } // Camera rays are pushed ^^^^^ forward to start in interior
          c[i] = c[i] + Vec(clamp(r.x),clamp(r.y),clamp(r.z)) * .25;
        }
  }

  ++count;

  // Convert to float
  for (int i = 0, j = 0; i < w * h * 3 && j < w * h; i += 3, j += 1)
  {
      data[i + 0] = c[j].x / count;
      data[i + 1] = c[j].y / count;
      data[i + 2] = c[j].z / count;
  }

  return data;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////
//// 
//// https://docs.microsoft.com/en-us/cpp/build/reference/openmp-enable-openmp-2-0-support?view=vs-2019
///////////////////////////////////////////////////////////////////////////////////////////////////////
float* smallptTest::smallpt(void)
{
  Ray cam(Vec(50,52,295.6), Vec(0,-0.042612,-1).norm()); // cam pos, dir
  Vec cx = Vec(w*.5135 / h),
	  cy = (cx%cam.d).norm()*.5135,
	  r;

  const int samps = 1;
  std::ostringstream ss;
  ss << " Total Number: " << this->samples * 4 * samps << "\n";
  ss << " Sample Number: " << this->iterates * 4 * samps << "\n";
  this->progress = ss.str();

  if (this->iterates >= this->samples) // render done 
  {
	  runTest = false; // Test Done
	  ss << " Render Done! samples : " << this->iterates * 4 * samps << "\n";
	  this->progress = ss.str();
	  return data;
  }
  
  #pragma omp parallel for schedule(static, 1) private(r)       // OpenMP
  for (int y = 0; y < h; y++) // Loop over image rows
  {
#if 0
    #pragma omp critical
	{
	ss << " Thread Number: " << omp_get_num_threads()<<  "\t Thread ID: " << omp_get_thread_num() << "\n";
	}
#endif
    for (unsigned short x = 0, Xi[3]={0,0,y*y*y}; x < w; x++)   // Loop cols
      for (int sy=0, i=(y) * w + x; sy < 2; sy++)     // 2x2 subpixel rows
        for (int sx = 0; sx < 2; sx++, r=Vec())			  // 2x2 subpixel cols
        {        
          for (int s = 0; s < samps; s++)
          {
            double r1=2*erand48(Xi), dx=r1<1 ? sqrt(r1)-1: 1-sqrt(2-r1);
            double r2=2*erand48(Xi), dy=r2<1 ? sqrt(r2)-1: 1-sqrt(2-r2);
            Vec d = cx*( ( (sx+.5 + dx)/2 + x) / w - .5) +
                    cy*( ( (sy+.5 + dy)/2 + y) / h - .5) + cam.d;
            r = r + radiance(Ray(cam.o+d*140, d.norm()), 0, Xi) * (1./samps);
            //r = r + radiance(Ray(cam.o+d*140, d.norm()), 0, Xi);
          }
          c[i] = c[i] + Vec(clamp(r.x),clamp(r.y),clamp(r.z)) * .25;
        }
  }
  this->iterates++;

  uint64_t count = this->iterates;
  // Convert to float
  for (int i = 0, j = 0; i < w * h * 3 && j < w * h; i += 3, j += 1)
  {
      data[i + 0] = c[j].x / count;
      data[i + 1] = c[j].y / count;
      data[i + 2] = c[j].z / count;
  }

  return data;
}

