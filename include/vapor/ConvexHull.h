// A C++ program to find convex hull of a set of points. Refer
// https://www.geeksforgeeks.org/orientation-3-ordered-points/
// for explanation of orientation()
#include <iostream>
#include <stack>
#include <stdlib.h>
using namespace std;

#ifdef BUILD_STANDALONE
struct Point {
    double x, y;
};
#endif

// A global point needed for  sorting points with reference
// to  the first point Used in compare function of qsort()
glm::vec2 p0;

// A utility function to find next to top in a stack
glm::vec2 nextToTop(stack<glm::vec2> &S)
{
    glm::vec2 p = S.top();
    S.pop();
    glm::vec2 res = S.top();
    S.push(p);
    return res;
}

// A utility function to swap two points
void swap(glm::vec2 &p1, glm::vec2 &p2)
{
    glm::vec2 temp = p1;
    p1 = p2;
    p2 = temp;
}

// A utility function to return square of distance
// between p1 and p2
double distSq(glm::vec2 p1, glm::vec2 p2) { return (p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y); }

// To find orientation of ordered triplet (p, q, r).
// The function returns following values
// 0 --> p, q and r are collinear
// 1 --> Clockwise
// 2 --> Counterclockwise
double orientation(glm::vec2 p, glm::vec2 q, glm::vec2 r)
{
    double val = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);

    if (val == 0) return 0;      // collinear
    return (val > 0) ? 1 : 2;    // clock or counterclock wise
}

// A function used by library function qsort() to sort an array of
// points with respect to the first point
int compare(const void *vp1, const void *vp2)
{
    glm::vec2 *p1 = (glm::vec2 *)vp1;
    glm::vec2 *p2 = (glm::vec2 *)vp2;

    // Find orientation
    double o = orientation(p0, *p1, *p2);
    if (o == 0) return (distSq(p0, *p2) >= distSq(p0, *p1)) ? -1 : 1;

    return (o == 2) ? -1 : 1;
}

// Prints convex hull of a set of n points.
std::stack<glm::vec2> convexHull(glm::vec2 points[], int n)
{
    // Find the bottommost point
    double ymin = points[0].y;
    int    min = 0;
    for (int i = 1; i < n; i++) {
        double y = points[i].y;

        // Pick the bottom-most or chose the left
        // most point in case of tie
        if ((y < ymin) || (ymin == y && points[i].x < points[min].x)) ymin = points[i].y, min = i;
    }

    // Place the bottom-most point at first position
    swap(points[0], points[min]);

    // Sort n-1 points with respect to the first point.
    // A point p1 comes before p2 in sorted output if p2
    // has larger polar angle (in counterclockwise
    // direction) than p1
    p0 = points[0];
    qsort(&points[1], n - 1, sizeof(glm::vec2), compare);

    // If two or more points make same angle with p0,
    // Remove all but the one that is farthest from p0
    // Remember that, in above sorting, our criteria was
    // to keep the farthest point at the end when more than
    // one points have same angle.
    int m = 1;    // Initialize size of modified array
    for (int i = 1; i < n; i++) {
        // Keep removing i while angle of i and i+1 is same
        // with respect to p0
        while (i < n - 1 && orientation(p0, points[i], points[i + 1]) == 0) i++;


        points[m] = points[i];
        m++;    // Update size of modified array
    }

    // Ordered list of points
    stack<glm::vec2> S;

    // If modified array of points has less than 3 points,
    // convex hull is not possible
    if (m < 3) { return S; }

    // Create an empty stack and push first three points
    // to it.
    S.push(points[0]);
    S.push(points[1]);
    S.push(points[2]);

    // Process remaining n-3 points
    for (int i = 3; i < m; i++) {
        // Keep removing top while the angle formed by
        // points next-to-top, top, and points[i] makes
        // a non-left turn
        while (S.size() > 1 && orientation(nextToTop(S), S.top(), points[i]) != 2) S.pop();
        S.push(points[i]);
    }

    return S;

#ifdef BUILD_STANDALONE
    // Now stack has the output points, print contents of stack
    while (!S.empty()) {
        glm::vec2 p = S.top();
        cout << "(" << p.x << ", " << p.y << ")" << endl;
        S.pop();
    }
#endif
}

#ifdef BUILD_STANDALONE
// Driver program to test above functions
int main()
{
    // glm::vec2 points[] = {{0, 3}, {1, 1}, {2, 2}, {4, 4},
    //                  {0, 0}, {1, 2}, {3, 1}, {3, 3}};
    glm::vec2 points[] = {{0.397329, -.5}, {-0.397329, .5}, {-0.48774, -.5}, {0.48774, .5}, {0.625, 0.288675}, {-.625, -0.288675}};
    int       n = sizeof(points) / sizeof(points[0]);
    convexHull(points, n);
    return 0;
}
#endif
