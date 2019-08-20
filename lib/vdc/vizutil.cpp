#include <cmath>
#include "vapor/VAssert.h"
#include <vapor/vizutil.h>

namespace {

double dot2d(const double a[], const double b[]) {
	return((a[0] * b[0]) + (a[1] * b[1]));
}

};

void VAPoR::HexahedronToTets(
	const int hexahedron[8],
	int tets[5*4]
) {

	int i = 0;
	if (hexahedron[0] % 2 == 0) {
		tets[i++] = hexahedron[0];
		tets[i++] = hexahedron[1];
		tets[i++] = hexahedron[5];
		tets[i++] = hexahedron[3];

		tets[i++] = hexahedron[0];
		tets[i++] = hexahedron[5];
		tets[i++] = hexahedron[6];
		tets[i++] = hexahedron[3];

		tets[i++] = hexahedron[0];
		tets[i++] = hexahedron[5];
		tets[i++] = hexahedron[4];
		tets[i++] = hexahedron[6];

		tets[i++] = hexahedron[0];
		tets[i++] = hexahedron[3];
		tets[i++] = hexahedron[6];
		tets[i++] = hexahedron[2];

		tets[i++] = hexahedron[5];
		tets[i++] = hexahedron[6];
		tets[i++] = hexahedron[3];
		tets[i++] = hexahedron[7];
	}
	else {
		tets[i++] = hexahedron[1];
		tets[i++] = hexahedron[5];
		tets[i++] = hexahedron[4];
		tets[i++] = hexahedron[7];

		tets[i++] = hexahedron[1];
		tets[i++] = hexahedron[4];
		tets[i++] = hexahedron[2];
		tets[i++] = hexahedron[7];

		tets[i++] = hexahedron[1];
		tets[i++] = hexahedron[4];
		tets[i++] = hexahedron[0];
		tets[i++] = hexahedron[2];

		tets[i++] = hexahedron[1];
		tets[i++] = hexahedron[7];
		tets[i++] = hexahedron[2];
		tets[i++] = hexahedron[3];

		tets[i++] = hexahedron[4];
		tets[i++] = hexahedron[2];
		tets[i++] = hexahedron[7];
		tets[i++] = hexahedron[6];
	}
}

void VAPoR::QuadToTris(
	const int quad[4],
	int tris[2*3]
) {
	
	// Triangle #1
	//
	int i = 0;
	tris[i++] = quad[0];
	tris[i++] = quad[1];
	tris[i++] = quad[3];

	// Triangle #2
	//
	tris[i++] = quad[0];
	tris[i++] = quad[3];
	tris[i++] = quad[2];
}

#ifdef	VAPOR3_0_0_ALPHA

void VAPoR::BaryTet(
	const Point3d &p, const Point3d &a, 
	const Point3d &b, const Point3d &c, const Point3d &d
	Point3d &ans
) {

	Vect3d vap(a, p);
	Vect3d vbp(b, p);
	Vect3d vcp(c, p);
	Vect3d vdp(d, p);

	Vect3d vab(a, b);
	Vect3d vac(a, c);
	Vect3d vad(a, d);

	Vect3d vbc(b, c);
	Vect3d vbd(b, d);

	double va;
	double vb;
	double vc;
	double vd;
	double v;
	Vect3d temp;

	
	const double frecip = 1.0 / 6.0;

	temp = vbd.cross(vbc);
	va = vbp.dot(temp) * frecip;

	temp = vac.cross(vad);
	vb = vap.dot(temp ) * frecip;

	temp = vad.cross(vab);
	vc = vap.dot(temp) * frecip;

	temp = vab.cross(vac);
	vd = vap.dot(temp) * frecip;

	temp =  vac.cross(vad);
	v  = vab.dot(temp) * frecip;
	
	VAssert(v != 0.0);
	double vrecip = 1.0 / v;

	ans.x = va * vrecip;
	ans.y = vb * vrecip;
	ans.z = vc * vrecip;
	ans.w = vd * vrecip;

}

void VAPoR::BaryTri(
	const Point3d &p, const Point3d &a, const Point3d &b, const Point3d &c
	Point3d &ans
) {

	Vect3d vap(a,p);
	Vect3d vbp(b,p);
	Vect3d vcp(c,p);
	Vect3d vab(a,b);
	Vect3d vca(c,a);
	Vect3d vbc(b,c);
	Vect3d vac(a,c);


	Vect3d n;
	Vect3d na;
	Vect3d nb;
	Vect3d nc;

	n = vab.cross( vac );
	na = vbc.cross( vbp );
	nb = vca.cross( vcp );
	nc = vab.cross( vap );

	double nrecip = n.dot(n);
	VAssert(nrecip != 0);
	nrecip = 1.0 / nrecip;

	ans.x = n.dot(na) * nrecip;
	ans.y = n.dot(nb) * nrecip;
	ans.z = n.dot(nc) * nrecip;
}
#endif

// Compute signed area of a triangle using the 3x3 determinant. Actually we
// return 2 * A, where A is area
//
double VAPoR::SignedTriArea2D(
	const double a[2], 
	const double b[2], 
	const double c[2]
) {
  return (  a[0] * (b[1] * 1.0 - 1.0 * c[1])
          - b[0] * (a[1] * 1.0 - 1.0 * c[1])
          + c[0] * (a[1] * 1.0 - 1.0 * b[1]));
}

bool VAPoR::BarycentricCoordsTri(
    const double verts[], const double pt[], double lambda[]
) {
	// Vector v0 = b - a, v1 = c - a, v2 = p - a;
	//
	double v0[] = {verts[2]-verts[0], verts[3]-verts[1]};
	double v1[] = {verts[4]-verts[0], verts[5]-verts[1]};
	double v2[] = {pt[0]-verts[0], pt[1]-verts[1]};

	double d00 = dot2d(v0, v0);
	double d01 = dot2d(v0, v1);
	double d11 = dot2d(v1, v1);
	double d20 = dot2d(v2, v0);
	double d21 = dot2d(v2, v1);
	double denom = d00 * d11 - d01 * d01;
	lambda[1] = (d11 * d20 - d01 * d21) / denom;
	lambda[2] = (d00 * d21 - d01 * d20) / denom;
	lambda[0] = 1.0f - lambda[1] - lambda[2];

	return(lambda[0] >= 0.0 && lambda[1] >= 0.0 && lambda[2] >= 0.0);
}

bool VAPoR::WachspressCoords2D(
	const double verts[], const double pt[], int n, double lambda[]
) {
	if (n == 0) return(false);

	for (int i=0; i<n; i++) lambda[i] = 0.0;

	double wTotal = 0.0;
	const double epsilon = 1e-6;

	int curr = 0;
	int prev = (curr+n-1) % n;
	int next = (curr+1) % n;
	double Aprev = SignedTriArea2D(pt, &verts[prev*2], &verts[curr*2]);
	bool onEdge = (Aprev > -epsilon && Aprev < epsilon);
	for (; curr<n && ! onEdge; curr++) {
		prev = (curr+n-1) % n;
		next = (curr+1) % n;

		double C = SignedTriArea2D(&verts[prev*2],&verts[curr*2],&verts[next*2]);
		double A = SignedTriArea2D(pt, &verts[curr*2], &verts[next*2]);
		onEdge = (A > -epsilon && A < epsilon);

		if (onEdge) break;	// special handling required

		lambda[curr] = C / (Aprev * A);
		Aprev = A;

		wTotal += lambda[curr];
	}

	// Special handling required if point is on an edge: simply linearly
	// interpolate between the adjacent points
	//
	if (onEdge) {
		for (int i=0; i<n; i++) lambda[i] = 0.0;

		int i0, i1;

		// Which edge is point on? beteen points prev and curr, or curr 
		// and next ?
		//
		if (Aprev > -epsilon && Aprev < epsilon) {
			i0 = (curr+n-1) % n;
			i1 = curr;
		}
		else {
			i0 = curr;
			i1 = (curr+1) % n;
		}

		double w;
		if (verts[i0*2] != verts[i1*2]) {
			w = 1.0-((pt[0]-verts[i0*2])/(verts[i1*2]-verts[i0*2]));
		}
		else {
			w = 1.0-((pt[1]-verts[i0*2+1])/(verts[i1*2+1]-verts[i0*2+1]));
		}

		lambda[i0] = w;
		lambda[i1] = 1.0 - w;

		if (w<0.0 || w>1.0) return (false);
		else return(true);
	}

	// Normalize so sum of weights is 1.0 if point inside;
	//
	bool inside = true;
	wTotal = 1.0 / wTotal;
	for (int i=0; i<n; i++) {
		lambda[i] *= wTotal;
		if (lambda[i] < 0.0) inside = false;
	}

	return(inside);
}

bool VAPoR::InsideConvexPolygon(
	const double verts[], const double pt[], int n
) {
	VAssert(n >= 3);

	// Partition the convex polygon into a triangle fan, testing each triangle
	// to see if it contains the point
	//
	double triverts[6];
	triverts[0] = verts[0];
	triverts[1] = verts[1];
	double dummy[3];
	for (int i=2; i<n; i++) {
		triverts[2] = verts[2*(i-1)];
		triverts[3] = verts[2*(i-1)+1];
		triverts[4] = verts[2*(i)];
		triverts[5] = verts[2*(i)+1];
		if (BarycentricCoordsTri(triverts, pt, dummy)) return(true);
	}

	return(false);
}



