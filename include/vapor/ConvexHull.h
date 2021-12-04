// A C++ program to find convex hull of a set of points. Refer
// https://www.geeksforgeeks.org/orientation-3-ordered-points/
// for explanation of orientation()
#include <iostream>
#include <stack>
#include <stdlib.h>
using namespace std;

// A utility function to find next to top in a stack
glm::vec2 nextToTop(stack<glm::vec2> &S);

// A utility function to swap two points
void swap(glm::vec2 &p1, glm::vec2 &p2);

// A utility function to return square of distance
// between p1 and p2
double distSq(glm::vec2 p1, glm::vec2 p2);

// To find orientation of ordered triplet (p, q, r).
// The function returns following values
// 0 --> p, q and r are collinear
// 1 --> Clockwise
// 2 --> Counterclockwise
double orientation(glm::vec2 p, glm::vec2 q, glm::vec2 r);

// A function used by library function qsort() to sort an array of
// points with respect to the first point
int compare(const void *vp1, const void *vp2);

// Prints convex hull of a set of n points.
std::stack<glm::vec2> convexHull(glm::vec2 *points, int n);
