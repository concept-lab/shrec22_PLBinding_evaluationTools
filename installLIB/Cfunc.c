#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

/*
Author(s): L.Gagliardi Iit -- Genova
Contains implementation in C to be used with Ctypes in python scripts.
INSTRUCTIONS 
Create the library using:
    gcc -fPIC -c Cfunc.c -lm
OBS: might need : gcc -fPIC -c Cfunc.c -lm -std=c99
and then 
    gcc -shared Cfunc.o -o libCfunc.so
Within python script:
import os
import ctypes
libfile = os.path.abspath("libCfunc.so") 
Cfunc = ctypes.CDLL(libfile)

Use pointer to np array to pass it to the function:
c_array = numpy.ascontiguousarray(original np array, dtype=int)
pointer = c_array.ctypes.data_as(ctypes.c_void_p)   

*/

double distance_euclidean(const double p1[3],const double p2[3]){
    const int D = 3;
    double d2;
    d2=0;
    for (int i = 0; i < D; i++)
    {   
        d2+= (p1[i]-p2[i])*(p1[i]-p2[i]);
        // p12[i] = p1[i]-p2[i]; 
    }
    // d2 = p12[0]*p12[0] + p12[1]*p12[1] + p12[2]*p12[2];   
    return (sqrt(d2));  
}

void pdist(double* dist,const double* X1, const double* X2,const int same,const int data_size1,const int data_size2)
{   
    /*
    Input arrays are contiguos memory transformed 2d arrays. ith-row = ith-probe, columns=3 coordinates. 
    Each 3 subscessive memory location "I change row"
    */
    int m,k;
 
    if(same==1){
        //Identical arrays are passed 
        // printf("\nIdentical list of coordinates scenario");
        // fflush(stdout);
        m = data_size1;
        k = 0;
        //all distances excluding with itself (X1 and X2 are the same list of coordinates)
        for (int i = 0; i < m-1; i++){
            for (int j = i+1; j<m;j++){
                // dist[k] = distance_euclidean(p1[i], p2[j]);
                dist[k] = distance_euclidean(&X1[i*3], &X1[j*3]);
                k += 1;
            }
        }
    }
    else{
        // printf("\nDifferent list of coordinates scenario. Size first set: %d. Size second set %d", data_size1,data_size2);
        // fflush(stdout);
        k = 0;
        for (int i = 0; i < data_size1; i++)
        {
            for(int j =0; j<data_size2; j++){
                //dist[k] = distance_euclidean(p1[i], p2[j]);
                dist[k] = distance_euclidean(&X1[i*3], &X2[j*3]);
                // printf("\n %f", dist[k]);
                // dist[k]=1;
                k += 1;
            }
        }
    }

}


///////////////////////// ARVO_C ////////////////////////
/*
A modified version to be called as a function of the arvo_c software 
https://www.sciencedirect.com/science/article/pii/S0010465512001580 
-----------
 * arvo_c - a C version of program for calculation volume and surface of
 * molecule according to article Busa et. al.: ARVO: A Fortran package for
 * computing the solvent accessible surface area and the excluded volume of
 * overlapping spheres via analytic equations, Computer Physics Communications,
 * Vol. 165, Iss. 1, Pages 59-96.
 *
 * Author(s): Jan Busa Jr.
 * Institution: Academia Sinica
 * Version 2.0, 27.12.2011
 *
 * Version history:
 * V2.1_cracked (l. Gagliardi IIT): spheres passed as an argument to il fu main, 
 * returns Volume and surface. 
 * Also implements new simple volume calculator that could be handy when one wants to measure spheres overlap
 * ----
 * V2.0 - program rewritten into C. Added dynamic allocation of memory, changed
 *        loading of molecules (using files generated by input_structure),
 *        changed NorthPoleTest to NorthPoleFix removing the necessity of using
 *        rotation of whole molecule (which is in some cases impossible).
 * V1.0 - initial version written in FORTRAN. Can by downloaded from
 *        http://cpc.cs.qub.ac.uk/summaries/ADUL
 */


#define PI 3.14159265358979323846264
#define ARRAY_INCREMENT 10 // by this size are arrays resized when necessary
#define EPS_NORTH_POLE 0.0001 // accuracy in the subroutine NorthPoleFix
#define NORTH_POLE_REDUCE 0.9999  // reduction parameter in NorthPoleFix

#define EPS_DELTAT 1e-12 // accuracy in the subroutine CirclesIntersection
#define EPS_ANGLE  1e-12 // accuracy in the subroutine DeleteEqual (angles)
#define EPS_TWO_PI 1e-12

int maxNeighbors;

int sizeInd;
int *ind;    // there shouldn't be more neighbors

int spheresNumber;    // number of spheres
double *spheres;      // 4*spheresNumber
int *neighborsNumber; //   spheresNumber
int *indexStart;      //   spheresNumber
// rest will have dynamic size
int sizeNeighborsIndices;
int *neighborsIndices;
int sizeSphereLocal;
double *sphereLocal; // 4 * sizeSphereLocal
int sizeCircles;
double *circles;     // 4 * sizeCircles
int sizeArcs;
double *arcs;        // 3 * sizeArcs
int sizeNewArcs;
double *newArcs;     // 3 * sizeNewArcs
int sizeAngles;
double *angles;      // sizeAngles
int sizeNewAngles;
double *newAngles;   // sizeNewAngles



/*
 * Function Cleanup() checks whether some memory was allocated and if yes, it
 * relases this memory and sets appropriate pointer to NULL.
 */
void Cleanup(){
//   printf("Cleaning memory");
//   fflush(stdout);
  
  if (sphereLocal) {free(sphereLocal); sphereLocal=NULL;}
  if (circles) {free(circles); circles=NULL;}
  if (ind) {free(ind); ind=NULL;}
  if (arcs) {free(arcs); arcs=NULL;}
  if (newArcs) {free(newArcs); newArcs=NULL;}
  if (angles) {free(angles); angles=NULL;}
  if (newAngles) {free(newAngles); newAngles=NULL;}
  if (neighborsIndices) {free(neighborsIndices); neighborsIndices=NULL;}
  if (spheres) {free(spheres); spheres=NULL;}
  if (neighborsNumber) {free(neighborsNumber); neighborsNumber=NULL;}
  if (indexStart) {free(indexStart); indexStart=NULL;}
  
/* (arvo_c authors)for some reasons following lines lead to Segmentation fault. Technically they are correct
   so I don't understand the reason for this behavior.
  if (spheres) {free(spheres); spheres=NULL;}
  if (neighborsNumber) {free(neighborsNumber); neighborsNumber=NULL;}
  if (indexStart) {free(indexStart); indexStart=NULL;}
*/
}


/* 
 * Function Initialization sets all pointers used to NULL and counters to zero
 * so we can be sure, where we start.
 */
void Initialization(const int n){
  maxNeighbors = 0;
  sphereLocal=NULL;
  circles=NULL;
  ind=NULL;
  arcs=NULL;
  newArcs=NULL;
  angles=NULL;
  newAngles=NULL;
  spheres=NULL;
  neighborsNumber=NULL;
  neighborsIndices=NULL;
  indexStart=NULL;
  sizeNeighborsIndices = 0;
  sizeSphereLocal = 0;
  sizeCircles = 0;
  sizeArcs = 0;
  sizeNewArcs = 0;
  sizeAngles = 0;
  sizeNewAngles = 0;

  sizeInd =0; //<-- CORRECTED BUG preventing crushes for successive calls
 //NEW 
  spheresNumber = n;
  if (spheresNumber < 1) return;
  spheres = (double*) malloc(spheresNumber * 4 * sizeof(double));
  if (!spheres) {
    printf("Error allocating memory for spheres\n");
    fflush(stdout);
    Cleanup();
    exit(-2);
  }
  indexStart = (int*) malloc((spheresNumber +1 )* sizeof(int)); //indexStart = (int*) malloc((spheresNumber)* sizeof(int));
  //NEW --> Corrected allocation error, causing sometimes: "corrupted size vs. prev_size Aborted (core dumped)"
  if (!indexStart) {
    printf("Error allocating memory for startIndices\n");
    fflush(stdout);
    Cleanup();
    exit(-2);
  }
  neighborsNumber = (int*) malloc(spheresNumber * sizeof(int));
  if (!neighborsNumber) {
    printf("Error allocating memory for neighborsNumber\n");
    fflush(stdout);
    Cleanup();
    exit(-2);
  }
  ///
}



/*
 * Helper function for safe write into dynamic array.
 */
void SetInd(const int i, const int val) {
  if (i<sizeInd) {
    *(ind+i) = val;
    return;
  }
  while (sizeInd<=i)
    sizeInd += ARRAY_INCREMENT;
  ind = (int*) realloc(ind, sizeInd*sizeof(int));
  if (ind==NULL) {
    printf("Error (re)allocating neighborsIndices. Exiting.\n");
    fflush(stdout);
    Cleanup();
    exit(-1);
  }
  *(ind+i) = val;
}

/*
 * Helper function for safe write into dynamic array.
 */
void SetNI(const int i, const int val){
  if (i < sizeNeighborsIndices){
    *(neighborsIndices+i)=val;
    return;
  }
  while (sizeNeighborsIndices <= i)
    sizeNeighborsIndices+=ARRAY_INCREMENT;
  neighborsIndices = (int*) realloc(neighborsIndices,sizeNeighborsIndices*sizeof(int));
  if (neighborsIndices == NULL){
    printf("Error (re)allocating neighborsIndices. Exiting.\n");
    Cleanup();
    exit(-1);
  }
  *(neighborsIndices+i)=val;
}

/*
 * Helper function for safe write into dynamic array.
 */
void SetSL(const int i, const int j, const double val){
  if (i < sizeSphereLocal){
    *(sphereLocal+i*4+j)=val;
    return;
  }
  while (sizeSphereLocal <= i)
    sizeSphereLocal+=ARRAY_INCREMENT;
  sphereLocal = (double*) realloc(sphereLocal,sizeSphereLocal*4*sizeof(double));
  if (sphereLocal == NULL){
    printf("Error (re)allocating sphereLocal. Exiting.\n");
    Cleanup();
    exit(-1);
  }
  *(sphereLocal+i*4+j)=val;
}

/*
 * Helper function for safe write into dynamic array.
 */
void SetCirc(const int i, const int j, const double val){
  if (i < sizeCircles){
    *(circles+i*4+j)=val;
    return;
  }
  while (sizeCircles <= i)
    sizeCircles+=ARRAY_INCREMENT;
  circles = (double*) realloc(circles,sizeCircles*4*sizeof(double));
  if (circles == NULL){
    printf("Error (re)allocating circles. Exiting.\n");
    Cleanup();
    exit(-1);
  }
  *(circles+i*4+j)=val;
}

/*
 * Helper function for safe write into dynamic array.
 */
void SetArc(const int i, const int j, const double val){
  if (i < sizeArcs){
    *(arcs+i*3+j)=val;
    return;
  }
  while (sizeArcs <= i)
    sizeArcs+=ARRAY_INCREMENT;
  arcs = (double*) realloc(arcs,sizeArcs*3*sizeof(double));
  if (arcs == NULL){
    printf("Error (re)allocating arcs. Exiting.\n");
    Cleanup();
    exit(-1);
  }
  *(arcs+i*3+j)=val;
}

/*
 * Helper function for safe write into dynamic array.
 */
void SetNArc(const int i, const int j, const double val){
  if (i < sizeNewArcs){
    *(newArcs+i*3+j)=val;
    return;
  }
  while (sizeNewArcs <= i)
    sizeNewArcs+=ARRAY_INCREMENT;
  newArcs = (double*) realloc(newArcs,sizeNewArcs*3*sizeof(double));
  if (newArcs == NULL){
    printf("Error (re)allocating newArcs. Exiting.\n");
    Cleanup();
    exit(-1);
  }
  *(newArcs+i*3+j)=val;
}

/*
 * Helper function for safe write into dynamic array.
 */
void SetAng(const int i, const double val){
  if (i < sizeAngles){
    *(angles+i)=val;
    return;
  }
  while (sizeAngles <= i)
    sizeAngles+=ARRAY_INCREMENT;
  angles = (double*) realloc(angles,sizeAngles*sizeof(double));
  if (angles == NULL){
    printf("Error (re)allocating angles. Exiting.\n");
    Cleanup();
    exit(-1);
  }
  *(angles+i)=val;
}

/*
 * Helper function for safe write into dynamic array.
 */
void SetNA(const int i,const double val){
  if (i < sizeNewAngles){
    *(newAngles+i)=val;
    return;
  }
  while (sizeNewAngles <= i)
    sizeNewAngles+=ARRAY_INCREMENT;
  newAngles = (double*) realloc(newAngles,sizeNewAngles*sizeof(double));
  if (newAngles == NULL){
    printf("Error (re)allocating newAngles. Exiting.\n");
    Cleanup();
    exit(-1);
  }
  *(newAngles+i)=val;
}

/*
 * Function QuickSpheresEstimate estimates number of spheres (atoms). It reads
 * input file and calculates the number of rows not starting by '#' (comment
 * lines).
 */
int QuickSpheresEstimate(FILE *fpAts) {
  char buffer[85];
  int numSpheres = 0;
  rewind(fpAts);
  while (fgets(buffer, sizeof(buffer), fpAts)) {
    if (buffer[0] != '#') numSpheres++;
  }
  return numSpheres;
}


/*
 * Function loads protein from file fpAts and fills the structure spheres.
 * First it calls QuickSpheresEstimate to obtain an idea of number of spheres,
 * than it allocates fixed-size structures (spheres, indexStart,
 * neighborsNumber) and finaly it fills the structure spheres using data from
 * protein file.
 */
void LoadProtein(FILE *fpAts, FILE *fpLog) {
  char buffer[85];
  char *pBuf;
  //int idx, jdx;
  double rWater;
  pBuf = buffer;
  if (!fpAts) {
    printf("Error in LoadProtein, no valid atoms file provided\n");
    Cleanup();
    exit(-1);
  }

  // at this point we have valid input file
  spheresNumber = QuickSpheresEstimate(fpAts); // estimate number of spheres and allocate memory
  if (spheresNumber < 1) return;

  spheres = (double*) malloc(spheresNumber * 4 * sizeof(double));
  if (!spheres) {
    printf("Error allocating memory for spheres\n");
    Cleanup();
    exit(-2);
  }
  indexStart = (int*) malloc(spheresNumber * sizeof(int));
  if (!indexStart) {
    printf("Error allocating memory for startIndices\n");
    Cleanup();
    exit(-2);
  }
  neighborsNumber = (int*) malloc(spheresNumber * sizeof(int));
  if (!neighborsNumber) {
    printf("Error allocating memory for neighborsNumber\n");
    Cleanup();
    exit(-2);
  }

  // parse protein file
  rewind(fpAts);
  spheresNumber = 0;
  while (fgets(buffer, sizeof(buffer), fpAts)) {
    if (buffer[0] == '#') { // some comments have special meaning
      if (!strncmp((pBuf + 2), "Protein:", 8)) { // copy protein name
        printf("Protein name: %s", pBuf + 11);
        if (fpLog) fprintf(fpLog, "Protein name: %s", pBuf + 11);
        continue;
      }
      if (!strncmp((pBuf + 2), "Radii set:", 10)) { // copy radii set name
        printf("Radii set: %s", pBuf + 13);
        if (fpLog) fprintf(fpLog, "Radii set: %s", pBuf + 13);
        continue;
      }
      if (!strncmp((pBuf + 2), "Water radius:", 13)) { // get water radius
        sscanf((pBuf + 16), "%lf", &rWater);
        printf("Water radius: %lf\n", rWater);
        if (fpLog) fprintf(fpLog, "Water radius: %lf\n", rWater);
        continue;
      }
      continue;
    }
    // here follows protein
    sscanf(pBuf, "%lf %lf %lf %lf", (spheres + spheresNumber * 4),
        (spheres + spheresNumber * 4 + 1), (spheres + spheresNumber * 4 + 2),
        (spheres + spheresNumber * 4 + 3)); // load sphere
    spheresNumber++;
  }
}

// fraction evaluation for integral
double Fract(double A, double B, double C,double sinphi,double cosphi, double k){
  return (-B*sinphi+C*cosphi)/pow((A+B*cosphi+C*sinphi),k);
}

/*
 * Computing integrals over arcs given in arc structure according to paper
 * Busa et al.
 */
void AvIntegral(const int nArcs, double *pVolume, double *pArea, const double r1, const double z1){
  int idx;
  double t,s,r,A,B,C,S,rr, ca, sa, cb, sb, be, al;
  double vIone,vItwo,vIthree,vJone,vJtwo,vJthree,delta_vint,delta_aint;

  *pVolume=0.0;
  *pArea=0.0;

  for (idx = 0; idx < nArcs; idx++){ // cycle over all arcs
    t=circles[((int)(arcs[idx*3]))*4];
    s=circles[((int)(arcs[idx*3]))*4+1];
    r=circles[((int)(arcs[idx*3]))*4+2];
    A=(4.0*r1*r1+t*t+s*s+r*r)/2.0;
    B=t*r;
    C=s*r;
    S=sqrt(A*A-B*B-C*C);
    rr=r*r-A;
    if (fabs(fabs(arcs[idx*3+2])-2.0*PI) < EPS_TWO_PI) { // full circle arc
      vIone=2.0*PI/S;
      vItwo=2.0*PI*A/(pow(S,3.));
      vIthree=PI*(2.0*A*A+B*B+C*C)/(pow(S,5.));
      vJone=PI+rr/2.0*vIone;
      vJtwo=(vIone+rr*vItwo)/4.0;
      vJthree=(vItwo+rr*vIthree)/8.0;
      delta_vint=(128.0*vJthree*pow(r1,7.)+8.0*vJtwo*pow(r1,5.)+ 2.0*vJone*pow(r1,3.))/3.0-8.0*pow(r1,4.)*vJtwo*(z1+r1);
      delta_aint=2.0*vJone*r1*r1;
      if (arcs[idx*3+2] < 0){
        delta_vint=-delta_vint;
        delta_aint=-delta_aint;
      }
      *pVolume=*pVolume+delta_vint;
      *pArea=*pArea+delta_aint;
    }
    else { // integration over arcs
      if (arcs[idx*3+2] < 0){
        al=arcs[idx*3+1]+arcs[idx*3+2];
        be=arcs[idx*3+1];
      }
      else {
        be=arcs[idx*3+1]+arcs[idx*3+2];
        al=arcs[idx*3+1];
      }
      vIone=2.0*(PI/2.0-atan((A*cos((be-al)/2.0)+ B*cos((al+be)/2.0)+C*sin((al+be)/2.0))/ (S*sin((be-al)/2.0))))/S;

      sb=sin(be);
      cb=cos(be);
      sa=sin(al);
      ca=cos(al);
      vItwo=(Fract(A,B,C,sb,cb,1)-Fract(A,B,C,sa,ca,1)+ A*vIone)/(S*S);
      vIthree=(Fract(A,B,C,sb,cb,2)-Fract(A,B,C,sa,ca,2)+ (Fract(A,B,C,sb,cb,1)-Fract(A,B,C,sa,ca,1))/A+ (2.0*A*A+B*B+C*C)*vItwo/A)/(2.0*S*S);
      vJone=((be-al)+rr*vIone)/2.0;
      vJtwo=(vIone+rr*vItwo)/4.0;
      vJthree=(vItwo+rr*vIthree)/8.0;
      delta_vint=(128.0*vJthree*pow(r1,7.)+8.0*vJtwo*pow(r1,5.)+ 2.0*vJone*pow(r1,3.))/3.0-8.0*pow(r1,4.)*vJtwo*(z1+r1);
      delta_aint=2.0*vJone*r1*r1;
      if (arcs[idx*3+2] < 0){
        delta_vint=-delta_vint;
        delta_aint=-delta_aint;
      }
      *pVolume=*pVolume+delta_vint;
      *pArea=*pArea+delta_aint;
    }
  }
}

// Deletion of "equal" (to some precision eps_angle) angles in sorted vector angles
int DeleteEqual(const int nAngles) {
  double angle;
  int UniqueAngles, idx;
  UniqueAngles = 0;

  angle = angles[0];
  SetNA(UniqueAngles,angle);
  UniqueAngles++;
  for (idx = 1; idx<nAngles; idx++) {
    if (fabs(angles[idx]-angle)>EPS_ANGLE) {
      angle = angles[idx];
      SetNA(UniqueAngles,angle);
      UniqueAngles++;
    }
  }
  for (idx = 0; idx<UniqueAngles; idx++)
    angles[idx] = newAngles[idx];
  return UniqueAngles;
}

// sorting array angles in decreasing order num_angle is the angles array length
void MyDSort(const int nAngles) {
  // here no functions SetAng are necessary
  int idx, iidx, jdx;
  double amin;
  for (idx = 0; idx<nAngles-1; idx++) {
    iidx = idx;
    amin = angles[idx];
    for (jdx = idx+1; jdx<nAngles; jdx++) {
      if (amin<angles[jdx]) {
        iidx = jdx;
        amin = angles[jdx];
      }
    }
    if (iidx!=idx) {
      angles[iidx] = angles[idx];
      angles[idx] = amin;
    }
  }
}

// sorting array angles in increasing order num_angle is the angles array length
void MySort(const int nAngles) {
  // here no functions SetAng are necessary
  int idx, iidx, jdx;
  double amax;

  for (idx = 0; idx<nAngles-1; idx++) {
    iidx = idx;
    amax = angles[idx];
    for (jdx = idx+1; jdx<nAngles; jdx++) {
      if (amax>angles[jdx]) {
        iidx = jdx;
        amax = angles[jdx];
      }
    }
    if (iidx!=idx) {
      angles[iidx] = angles[idx];
      angles[idx] = amax;
    }
  }
}

/* 
 * Function PointInCircle returns
 * 1  if point (t,s) is inside k-th positive circle or outside k-th negative circle
 * 0  otherwise
 * WE KNOW, THAT POINT IS NOT ON THE CIRCLE
 */
int PointInCircle(const double t, const double s, const int Circle) {
  int PointInCircle;
  double d;

  PointInCircle = 0;

  d = sqrt((t-circles[Circle*4])*(t-circles[Circle*4])+(s-circles[Circle*4+1])
      *(s-circles[Circle*4+1]));
  if (d<circles[Circle*4+2])
    PointInCircle = (circles[Circle*4+3]>0) ? 1 : 0;
  else
    PointInCircle = (circles[Circle*4+3]>0) ? 0 : 1;
  return PointInCircle;
}

/* 
 * Function CircleInCircle returns
 * 1  if Circ1-th circle is inside Circ2-th positive circle or outside Circ2-th negative circle
 * 0  otherwise
 * WE KNOW, THAT CIRCLES HAVE LESS THAN 2 INTERSECTION POINTS
 * i -> Circ1, k->Circ2
 */
int CircleInCircle(const int Circ1, const int Circ2) {
  int CirclesInCircle;
  double d;
  CirclesInCircle = 0;

  d = sqrt((circles[Circ1*4]+circles[Circ1*4+2]-circles[Circ2*4])
      *(circles[Circ1*4]+circles[Circ1*4+2]-circles[Circ2*4])
      +(circles[Circ1*4+1]-circles[Circ2*4+1])*(circles[Circ1*4+1]
          -circles[Circ2*4+1]));

  if (d<circles[Circ2*4+2])
    CirclesInCircle = (circles[Circ2*4+3]>0) ? 1 : 0;
  else if (d>circles[Circ2*4+2])
    CirclesInCircle = (circles[Circ2*4+3]>0) ? 0 : 1;
  else {
    d = sqrt((circles[Circ1*4]-circles[Circ2*4])*(circles[Circ1*4]
        -circles[Circ2*4+0])+(circles[Circ1*4+1]-circles[Circ2*4+1])
        *(circles[Circ1*4+1]-circles[Circ2*4+1]));
    if (d<circles[Circ2*4+2])
      CirclesInCircle = (circles[Circ2*4+3]>0) ? 1 : 0;
    else
      CirclesInCircle = (circles[Circ2*4+3]>0) ? 0 : 1;
  }
  return CirclesInCircle;
}

/*
 * Function CirclesIntersection returns angles of two intersection points of
 * circles with indices ic1 and ic2 in circles structure circles (we will use
 * it ONLY IN CASE, WHEN 2 INTERSECTION POINTS EXIST)
 * a1 and a2 are corresponding angles with respect to the center of 1st circ
 * b1 and b2 are corresponding angles with respect to the center of 2nd circ
 */
void CirclesIntersection(const int Circ1, const int Circ2, double * a1,
    double * a2) {
  //     (t,s) - circle center, r - circle radius
  double t1, s1, r1, t2, s2, r2, b1, b2, A, B, C, D;
  *a1 = 0;
  *a2 = 0;
  b1 = 0;
  b2 = 0;

  t1 = circles[Circ1*4+0];
  s1 = circles[Circ1*4+1];
  r1 = circles[Circ1*4+2];
  t2 = circles[Circ2*4+0];
  s2 = circles[Circ2*4+1];
  r2 = circles[Circ2*4+2];

  if (fabs(t2-t1)<EPS_DELTAT) { // t2 == t1
    B = ((r1*r1-r2*r2)/(s2-s1)-(s2-s1))/2.0;
    A = sqrt(r2*r2-B*B);
    if (B==0) {
      b1 = 0.0;
      b2 = PI;
    }
    else if (B>0) {
      b1 = atan(fabs(B/A));
      b2 = PI-b1;
    }
    else {
      b1 = PI+atan(fabs(B/A));
      b2 = 3.0*PI-b1;
    }
    B = B+s2-s1;
    if (B==0) {
      *a1 = 0.0;
      *a2 = PI;
    }
    else if (B>0) {
      *a1 = atan(fabs(B/A));
      *a2 = PI-*a1;
    }
    else {
      *a1 = PI+atan(fabs(B/A));
      *a2 = 3.0*PI-*a1;
    }
  }
  else { // t2 != t1
    C = ((r1*r1-r2*r2-(s2-s1)*(s2-s1))/(t2-t1)-(t2-t1))/2.0;
    D = (s1-s2)/(t2-t1);
    B = (-C*D+sqrt((D*D+1.0)*r2*r2-C*C))/(D*D+1.0);
    A = C+D*B;
    if (A==0)
      b1 = (B>0) ? PI/2.0 : -PI/2.0;
    else if (A>0)
      b1 = atan(B/A);
    else
      b1 = PI+atan(B/A);
    B = B+s2-s1;
    A = A+t2-t1;
    if (A==0)
      *a1 = (B>0) ? PI/2.0 : -PI/2.0;
    else if (A>0)
      *a1 = atan(B/A);
    else
      *a1 = PI+atan(B/A);
    B = (-C*D-sqrt((D*D+1.0)*r2*r2-C*C))/(D*D+1.0);
    A = C+D*B;
    if (A==0)
      b2 = (B>0) ? PI/2.0 : -PI/2.0;
    else if (A>0)
      b2 = atan(B/A);
    else
      b2 = PI+atan(B/A);
    B = B+s2-s1;
    A = A+t2-t1;
    if (A==0)
      *a2 = (B>0) ? PI/2.0 : -PI/2.0;
    else if (A>0)
      *a2 = atan(B/A);
    else
      *a2 = PI+atan(B/A);
  }

  if (*a1<0) *a1 = *a1+2.0*PI;
  if (*a2<0) *a2 = *a2+2.0*PI;
  if (b1<0) b1 = b1+2.0*PI;
  if (b2<0) b2 = b2+2.0*PI;
}

/*
 * Function NewArcs prepares arcs, which are part of i-th circle in circle
 * structure circles. Interesting are these arcs, which are inside other
 * positive circles or outside other negative circles
 *
 * Matrix arcsnew in each row has elements
 *
 * arcsnew(i,1)=ic - ic is the index of arc-circle in circle
 * arcsnew(i,2)=sigma - sigma is the starting angle of arc
 * arcsnew(i,3)=delta - delta is oriented arc angle
 */
int NewArcs(const int Circle, const int NumAtoms) {
  int numArc, numAngle, numCond, idx, jdx;
  double ti, si, ri, t, s, r, d, a1, a2;
  numArc = 0;
  numAngle = 0;
  numCond = 0;

  ti = circles[Circle*4+0];
  si = circles[Circle*4+1];
  ri = circles[Circle*4+2];
  for (idx = 0; idx<NumAtoms; idx++) { // composition of angles vector, consisting of intersection points
    if (idx!=Circle) {
      t = circles[idx*4];
      s = circles[idx*4+1];
      r = circles[idx*4+2];
      d = sqrt((ti-t)*(ti-t)+(si-s)*(si-s));
      if ((d<r+ri)&&(fabs(r-ri)<d)) { // 2 intersection points exist
        CirclesIntersection(Circle, idx, &a1, &a2);
        SetAng(numAngle,a1);
        SetAng(numAngle+1,a2);
        numAngle += 2;
      }
    }
  }

  if (!numAngle) { // there are no double intersections of idx-th circles with
		   // others
    numCond = 0;
    for (idx = 0; idx<NumAtoms; idx++) { // if i-th circle is inside of all other
                                         // positive and outside of all other
                                         // negative circles, it will be new arc
      if (idx!=Circle) numCond = numCond+CircleInCircle(Circle, idx);
    }
    if (numCond==(NumAtoms-1)) { // all conditions hold
      SetNArc(numArc,0,Circle);
      SetNArc(numArc,1,0.0);
      SetNArc(numArc,2,2.0*PI*circles[Circle*4+3]);
      numArc++;
    }
  }
  else { // there are double intersection points
    if (circles[Circle*4+3] > 0)
      MySort(numAngle);
    else
      MyDSort(numAngle);

    numAngle = DeleteEqual(numAngle);
    for (idx = 0; idx<numAngle-1; idx++) {
      numCond = 0;
      for (jdx = 0; jdx<NumAtoms; jdx++) {
        if (jdx!=Circle) {
          t = ti+ri*cos((angles[idx]+angles[idx+1])/2.0);
          s = si+ri*sin((angles[idx]+angles[idx+1])/2.0);
          numCond = numCond+PointInCircle(t, s, jdx);
        }
      }
      if (numCond==(NumAtoms-1)) { // all conditions hold
        SetNArc(numArc,0,Circle);
        SetNArc(numArc,1,angles[idx]);
        SetNArc(numArc,2,angles[idx+1]-angles[idx]);
        numArc++; // zero based indices
      }
    }
    numCond = 0;
    for (idx = 0; idx<NumAtoms; idx++) {
      if (idx!=Circle) {
        t = ti+ri*cos((angles[0]+2.0*PI+angles[numAngle-1])/2.0);
        s = si+ri*sin((angles[0]+2.0*PI+angles[numAngle-1])/2.0);
        numCond = numCond+PointInCircle(t, s, idx);
      }
    }
    if (numCond==(NumAtoms-1)) { // all conditions hold
      SetNArc(numArc,0,Circle);
      SetNArc(numArc,1,angles[numAngle-1]);
      SetNArc(numArc,2,angles[0]+circles[Circle*4+3]*2.0*PI-angles[numAngle-1]);
      numArc++;
    }
  }
  return numArc;
}

/*
 * Function CirclesToArcs computes integration arcs
 *
 * arcs(i,1)=ci     - corresponding circle index
 * arcs(i,2)=sigma  - starting arc angle
 * arcs(i,3)=delta  - oriented arc angle
 * Arcs (with their orientation) are parts of circles, which bounds are
 * circles intersection points. If the center of arc lies inside all other
 * positive and outside all other negative circles, then we will put it
 * inside arcs structure
 */
int CirclesToArcs(const int NumAtoms) {
  int nArcs, idx, jdx, kdx, nna;
  nArcs = 0;
  if (NumAtoms==1) { // we have only 1 circle
    nArcs = 1;
    SetArc(0,0,0);
    SetArc(0,1,0);
    SetArc(0,2,2.0*PI*circles[3]);
  }
  else { // more than 1 circle
    for (idx = 0; idx<NumAtoms; idx++) {
      nna = NewArcs(idx, NumAtoms);
      if (nna) {
        for (jdx = 0; jdx<nna; jdx++) {
          for (kdx = 0; kdx<3; kdx++)
            SetArc(nArcs+jdx,kdx,newArcs[jdx*3+kdx]);
        }
        nArcs += nna;
      }
    }
  }
  return nArcs;
}

/*
 * Function MakeTsCircles prepares circles structure for 1st sphere
 * in array circles according to the paper Busa et al.
 *
 *   circles(i,1)=ti
 *   circles(i,2)=si    - ith circle's center coordinates
 *   circles(i,3)=ri    - ith circle's radius
 *   circles(i,4)=+1/-1 - circle orientation
 */
void MakeTsCircles(const int NumAtoms) {
  double r1, dx, dy, a, b, c, d;
  int idx;
  r1 = sphereLocal[3];
  for (idx = 0; idx<NumAtoms; idx++) {
    dx = sphereLocal[0]-sphereLocal[(idx+1)*4];
    dy = sphereLocal[1]-sphereLocal[(idx+1)*4+1];
    a = dx*dx+dy*dy+(sphereLocal[2]+r1-sphereLocal[(idx+1)*4+2])
        *(sphereLocal[2]+r1-sphereLocal[(idx+1)*4+2])-sphereLocal[(idx+1)*4+3]
        *sphereLocal[(idx+1)*4+3];
    b = 8.0*r1*r1*dx;
    c = 8.0*r1*r1*dy;
    d = 4.0*r1*r1*(dx*dx+dy*dy+(sphereLocal[2]-r1-sphereLocal[(idx+1)*4+2])
        *(sphereLocal[2]-r1-sphereLocal[(idx+1)*4+2])-sphereLocal[(idx+1)*4+3]
        *sphereLocal[(idx+1)*4+3]);
    SetCirc(idx,0,-b/(2.0*a));
    SetCirc(idx,1,-c/(2.0*a));
    SetCirc(idx,2,sqrt((b*b+c*c-4.0*a*d)/(4.0*a*a)));
    (a>0) ? SetCirc(idx,3,-1) : SetCirc(idx,3,1);
  }
}

/*
 * Function NorthPoleFix checks on local spheres if north pole of sphere
 * doesn't lie on the surface of other sphere. If it does, the radius of
 * sphere is multiplied by factor of NORTH_POLE_REDUCE. This function replaces
 * the NorthPoleTest and SpheresRotation from original algorithm.
 */
void NorthPoleFix(const int numAtoms) {
  double d;
  int idx;
  for (idx = 1; idx < numAtoms; idx++) { // all except atom 0
    d = fabs(
        sqrt(
            (sphereLocal[0] - sphereLocal[idx * 4])
                * (sphereLocal[0] - sphereLocal[idx * 4])
                + (sphereLocal[1] - sphereLocal[idx * 4 + 1])
                    * (sphereLocal[1] - sphereLocal[idx * 4 + 1])
                + (sphereLocal[2] + sphereLocal[3] - sphereLocal[idx * 4 + 2])
                    * (sphereLocal[2] + sphereLocal[3]
                        - sphereLocal[idx * 4 + 2]))
            - sphereLocal[idx * 4 + 3]);
    if (d < EPS_NORTH_POLE){
      sphereLocal[idx*4+3] = sphereLocal[idx*4+3]*NORTH_POLE_REDUCE;
      idx--;
    //   printf("Reduced sphere radius\n"); // print information that radius was
                                         // reduced
    }
  }
}

/*
 * Function LocalSpheres copies spheres whose indices are in the array ind
 * (actual atom and its neighbors) into the array sphereLocal and calls the
 * function NorthPoleFix to avoid that the north pole of actual atom lies on
 * some sphere.
 */
void LocalSpheres(const int numAtoms){
  int idx, jdx;
  for (idx = 0; idx < numAtoms; idx++){
    for (jdx = 0; jdx < 4; jdx++)
      SetSL(idx,jdx,spheres[ind[idx]*4+jdx]);
  }
  NorthPoleFix(numAtoms);
}

/*
 * Function AreaVolume computes Atom-th part of the whole volume - the volume
 * of domain inside Atom-th and outside of all other spheres
 */
void AreaVolume(const int Atom, double *pVolume, double *pArea) {
  // nls was originaly neighborsNumber[Atom]+1, shifted to neighborsNumber[Atom]!!!
  *pVolume = 0;
  *pArea = 0;

  // Determination of i-th sphere's neighbors (row indices in matrix spheres)
  if (neighborsNumber[Atom]<0) {
    // ith sphere is subset of other sphere, sphere(i,4) will be done negative
    *pVolume = 0.0;
    *pArea = 0.0;
    //check (dubug)
    // printf("Sphere is buried\n");
    // fflush(stdout);
  }
  else if (neighborsNumber[Atom]==0) {
    // there are no neighbors (nls - number of local spheres = ith sphere + neigh
    *pVolume = 4.0*PI*spheres[Atom*4+3]*spheres[Atom*4+3]*spheres[Atom*4+3]/3.0;
    *pArea = 4.0*PI*spheres[Atom*4+3]*spheres[Atom*4+3];
  }
  else {
    int idx, nArcs, nPos;
    double z1, r1, partVol, partArea;
    // there are neighbors
    SetInd(0,Atom);
    for (idx = 0; idx<neighborsNumber[Atom]; idx++)
      SetInd(idx+1,neighborsIndices[indexStart[Atom]+idx]);

    // we will work only with ith and neighbors spheres
    LocalSpheres(neighborsNumber[Atom]+1);

    *pVolume = 0.0;
    *pArea = 0.0;

    MakeTsCircles(neighborsNumber[Atom]);
    nArcs = CirclesToArcs(neighborsNumber[Atom]);

    nPos = 0;
    for (idx = 0; idx<neighborsNumber[Atom]; idx++) {
      if (circles[idx*4+3]>0) nPos++;
    }

    z1 = sphereLocal[2];
    r1 = sphereLocal[3];

    AvIntegral(nArcs, &partVol, &partArea, r1, z1);
    if (nPos) { // there exists positive oriented circle
      *pVolume = *pVolume+partVol;
      *pArea = *pArea+partArea;
    }
    else { // all circles are negative oriented - we have computed complement
      *pVolume = *pVolume+partVol+4.0*PI*sphereLocal[3]*sphereLocal[3]
          *sphereLocal[3]/3.0;
      *pArea = *pArea+partArea+4.0*PI*sphereLocal[3]*sphereLocal[3];
    }
  }
}

/*
 * If ith sphere is a subset of other sphere, index_number(i)=-1 and we change
 * radius in matrix spheres to -radius.
 * If some other sphere is subset of ith sphere, than we change its radius to
 * -radius.
 */
int Neighbors(int Atom) {
  int NeighborsNum = 0, idx; // number of neighbors
  double xi, yi, zi, ri, dd, rk;
  
  xi = spheres[Atom*4];
  yi = spheres[Atom*4+1];
  zi = spheres[Atom*4+2];
  ri = spheres[Atom*4+3];
  
  for (idx = 0; idx<spheresNumber; idx++) {
    if (idx==Atom) continue;
    if (fabs(xi-spheres[idx*4])<ri+spheres[idx*4+3]) {
      dd = sqrt((xi-spheres[idx*4])*(xi-spheres[idx*4])+(yi-spheres[idx*4+1])
          *(yi-spheres[idx*4+1])+(zi-spheres[idx*4+2])*(zi-spheres[idx*4+2]));
      rk = spheres[idx*4+3];
      if (dd<ri+rk) {
        if (dd+ri<=rk) { // ith sphere is inside of other sphere
          NeighborsNum = -1;
          return NeighborsNum;
        }
        else if (dd+rk>ri) { // kth sphere is neighbor
          SetInd(NeighborsNum++,idx);
        }
      }
    }
  }
  return NeighborsNum;
}

/*
 * Determination of neighbors for all atoms. We construct next structure:
 * neighborsNumber(i) = neighbors number for ith atom
 * indexStart(i)      = start of neighbors indices for ith atom in array
 *                      neighbors_indices
 * neighborsIndices   = array of neighbors indices for each atom
 * neighborsIndices(indexStart(i)):neighborsInd(indexStart(i)+neighborsNumber(i)
 *
 * For example: 1. atom has neighbors with indices 2, 4, 7
 *              2. atom has neighbors with indices 1, 3
 *              3. atom has neighbors with indices 2, 4
 *              4. atom has neighbors with indices 1, 3
 *              5. atom is subset of some atom
 *              6. atom has no neighbors
 *              7. atom has neighbors with index 1
 * then we have:
 *               neighborsNumber = (3,2,2,2,-1,0,1)
 *                    indexStart = (0,3,5,7,9,9,9)
 *              neighborsIndices = (1,3,6,0,2,1,3,0,2,0)
 */
void MakeNeighbors(void) {
 int idx, jdx;
  indexStart[0] = 0;
  for (idx = 0; idx<spheresNumber; idx++) {
    neighborsNumber[idx] = Neighbors(idx);
    if (neighborsNumber[idx] > maxNeighbors)
      maxNeighbors = neighborsNumber[idx];

    if (neighborsNumber[idx]<=0) // sphere is subset or there are no neighbors
      indexStart[idx+1] = indexStart[idx];
    else { //  there are neighbors
      indexStart[idx+1] = indexStart[idx]+neighborsNumber[idx];
      for (jdx = 0; jdx<neighborsNumber[idx]; jdx++)
        SetNI(indexStart[idx]+jdx,ind[jdx]);
    }
  }
}

/*
 * Function PrintUsage provides user with short information on how to use
 * the program arvo.
 */
void PrintUsage() {
  printf("ARVO_C 2.0.\n\nUsage: arvo protein=file1 [log=file2]\n\n");
  printf("Mandatory:\n  protein - name of input file as created by input_structure\n");
  printf("            (you can also use short form 'p=')\n");
  printf("Optional:\n");
  printf("      log - name of file for results and log messages\n");
}

/*
 * Main body of the program ARVO. It parses the parameters, loads protein file,
 * creates the neighbors structure, calls function for calculating the
 * contribution to the area and volume of every atom and collects the results.
 */

double simpleV(double* radii,const int ns){
  double r,V=0;
  
  for (int k = 0; k < ns; k++)
  {
      r = radii[k];
      V+= 4.0/3.0 * (r*r*r) * PI;

  }
  return V;
}

void ARVOLUME_C(double* result,const double* in_spheres,const int n) {
  /*
  Minimal changes to not alter too much the original module. 
  */
  int idx;
  double Volume, Area, retVolume, retArea; 

  Cleanup();
  Initialization(n); // initializate the memory and allocate spheres, index and neighbors arrays
  
  for (int i = 0; i < spheresNumber; i++)//running over rows of 4 columns: x,y,z,radius
  { 
    *(spheres + i * 4) = in_spheres[4*i]; //x
    *(spheres + i * 4 +1) = in_spheres[4*i+1]; //y
    *(spheres + i * 4 +2) = in_spheres[4*i+2]; //z
    *(spheres + i * 4 +3) = in_spheres[4*i+3]; //r
  }

  if (spheresNumber <= 0) {
    printf("No spheres to calculate volume. Exiting.\n");
    fflush(stdout);
    Cleanup();
    // return -1;
  }

  // Study the neighborhood relations
  MakeNeighbors();
  
  // Computation of area and volume as a sum of surface integrals
  Volume = 0;
  Area = 0;
  for (idx = 0; idx<spheresNumber; idx++) {
    AreaVolume(idx, &retVolume, &retArea);
    Volume += retVolume;
    Area += retArea;
  }

//   printf("\n Volume: %f\tArea: %f\tSpheres num: %d\n", Volume, Area, spheresNumber);
  
  result[0] = Volume;
  result[1] = Area;

  return;
//   return 0;
}

double MOC_score(const double* ligandANDpocket,const int n_total,const int n_ligand,const double ligand_area)
{
/*
Based on subsets of the spheres passed, we can compute surface area of ligand and pocket.
For the details of the implementation, with volume this does not work.. 
Probably when atom is not completely buried, there are not guaranties 
I'm actually getting the volume of only the non overlapping part of the ligand sphere.
Area is well defined instead.

 INPUT: stacked array containing both ligan and pocket. IMPORTANT: Ligand must be passed at the beginning
*/
  int idx;
  double Volume, Area, retVolume, retArea;
  double score;
  Cleanup();
  Initialization(n_total); // initializate the memory and allocate spheres, index and neighbors arrays
  
  
  for (int i = 0; i < spheresNumber; i++)//running over rows of 4 columns: x,y,z,radius
  { 
    *(spheres + i * 4) = ligandANDpocket[4*i]; //x
    *(spheres + i * 4 +1) = ligandANDpocket[4*i+1]; //y
    *(spheres + i * 4 +2) = ligandANDpocket[4*i+2]; //z
    *(spheres + i * 4 +3) = ligandANDpocket[4*i+3]; //r
  }

  
  if (spheresNumber <= 0) {
    printf("No spheres to calculate volume. Exiting.\n");
    fflush(stdout);
    Cleanup();
    // return -1;
  }

  // Study the neighborhood relations
  MakeNeighbors();
  
  // Computation of area and volume as a sum of surface integrals
  Volume = 0;
  Area = 0;
  for (idx = 0; idx<n_ligand; idx++) {
    AreaVolume(idx, &retVolume, &retArea);
    Volume += retVolume;
    Area += retArea;
    }
   score = (ligand_area- Area)/ligand_area;

   return score;
}
