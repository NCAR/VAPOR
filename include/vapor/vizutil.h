#ifndef _vizutil_h_
#define _vizutil_h_

namespace VAPoR {

//! Decompose a hexahedron into 5 tetrahedra
//!
//! This function takes as input the indecies of eight vertices defining
//! a hexahedron, and decomposes the hexahedron into a sequence of 5
//! tetrahedra. The ordering of the vertices in the input vector
//! \p hexahedron must follow the diagram below. 
//!
//! Two possible decompositions are possible, case 1 and case 2. 
//!
//! Case 1 results in 5 tetraheda comprised of the following indecies:
//!
//! 0,1,5,3; 0,5,6,3; 0,5,4,6; 0,3,6,2; 5,6,3,7;
//!
//! Case 2 results in 5 tetraheda comprised of the following indecies:
//!
//! 1,5,4,7; 1,4,2,7; 1,4,0,2; 1,7,2,3; 4,2,7,6
//!
//! Case 1 tetraheda are generated if the parity of the 0-vertex is 
//! even, case 2 if odd. 
//!
//! Conformant meshes are possible when case 1 and case 2 are adjacent. I.e.
//! if the input hexahedra are part of a conformant mesh face-adjacent 
//! hexahedra should have alternating 0-vertex parities.
//!
//!
//!       6*--------*7
//!       /|       /|
//!      / |      / |
//!     /  |     /  |
//!    /  4*----/---*5
//!  2*--------*3  /
//!   |  /     |  /
//!   | /      | /
//!   |/       |/
//!  0*--------*1
//!
//!
//! \param[in] hexahedron A eight-element array containing the unique vertex
//! indecies of a hexahedron.
//! \param[out] tets A twenty-element array containing the resulting 
//! vertex indecies of the 5 tetrahedra that the hexahedron  is decomposed
//! into.
//
void HexahedronToTets(
	const int hexahedron[8],
	int tets[5*4]
);

//! Decompose a quadrilateral into 2 triangles
//!
//! This function takes as input the indecies of four vertices defining
//! a quadrilateral, and decomposes the quad into a sequence of 2
//! triangles. The ordering of the vertices in the input vector
//! \p hexahedron must follow the diagram below. 
//!
//!  2*--------*3
//!   |        |
//!   |        |
//!   |        |
//!  0*--------*1
//!
//! The decomposition results in 2 triangles comprised of the 
//! following indecies:
//!
//! 0,1,3; 0,3,2
//!
//!
//! \param[in] quad A four-element array containing the unique vertex
//! indecies of a quadrilateral.
//! \param[out] tets A six-element array containing the resulting 
//! vertex indecies of the 2 triangles that the quadrilateral  is decomposed
//! into.
//
void QuadToTris(
	const int quad[4],
	int tris[2*3]
);

#ifdef	DEAD
void BaryTet(
    const Point3d &p, const Point3d &a,
    const Point3d &b, const Point3d &c, const Point3d &d
    Point3d &ans
);

void BaryTri(
    const Point3d &p, const Point3d &a, const Point3d &b, const Point3d &c
    Point3d &ans
);
#endif

//! Return the signed, double-area of a 2D triangle
//!
//! This function uses determinants to calculate double the signed area of a 
//! triange with 2D coordinates. Equivalently, this is the 3x3 determinant
//! of a matrix whose last row is all ones.  The return value is positive 
//! if the vertices are given in counter-clockwise order, otherwise it 
//! is negtive. Thus the true area is 1/2 * |A|, where A is the returned 
//! value.
//!
//! \param[in] a 2D coordinates of first vertex
//! \param[in] b 2D coordinates of second vertex
//! \param[in] c 2D coordinates of third vertex
//!
double SignedTriArea2D(
	const double a[2],
	const double b[2],
	const double c[2]
);

//! Compute the Wachspress coordinates for a point inside an irregular, 
//! convex, n-sided, planar polygon.
//!
//! This function uses a method adapted from Meyer2005 (Generalized
//! Barycentric Coordinates on Irregular Polygons) to compute Wachspress
//! (generalized barycentric) coordinates for a point relative to an 
//! n-sided, 2D, irregular  polygon. Wachspress coordinates for a point, p,
//! inside a polygon, Q, have the properties that:
//!
//! The sum of the Wachspress coordinates is exactly 1.0
//!
//! The Cartesian coordinates of p are given by the sum of the products of
//! Q's vertices with the Wachspress coordinates. 
//!
//! If p is outside of Q at lease one of the Wachspress is negative
//! 
//! \param[in] verts an array of 2D polygon Cartesian coordinates
//! describing a possibly irregular, convex polygon.
//! \param[in] pt the 2D Cartesian coordinates
//! \param[in] n The number of vertices in \p verts. I.e. degree of polygon
//! \param[out] Wachspress coordinates for point \pt. The number of 
//! Wachspress coordinates is given by \p n.
//!
//! \retval inside a flag indicating whether the point \p pt
//! is inside (or on the edge) of Q. I.e. all of the Wachspress coordinates
//! are positive.

bool WachspressCoords2D(
	const double verts[], const double pt[], int n, double lambda[]
);

};

#endif
