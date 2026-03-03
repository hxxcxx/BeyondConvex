#include "geometry_utils.h"
#include <cmath>
#include <limits>
#include <algorithm>

namespace geometry {

bool GeometryUtils::ToLeftTest(const Point2D& p, const Point2D& q, const Point2D& r) {
    Vector2D pq = q - p;
    Vector2D pr = r - p;
    return pq.Cross(pr) > 0;
}

bool GeometryUtils::ToLeftTest(const Vector2D& v1, const Vector2D& v2) {
    return v1.Cross(v2) > 0;
}

ConvexHull GeometryUtils::JarvisMarch(const std::vector<Point2D>& points) {
    std::vector<Point2D> hull_vertices;
    
    if (points.size() < 3) {
        return ConvexHull(hull_vertices);
    }
    
    // Find the leftmost point (lowest x, then lowest y if tie)
    int start_index = 0;
    for (size_t i = 1; i < points.size(); ++i) {
        if (points[i].x < points[start_index].x ||
            (points[i].x == points[start_index].x && points[i].y < points[start_index].y)) {
            start_index = static_cast<int>(i);
        }
    }
    
    int current = start_index;
    
    do {
        // Add current point to hull
        hull_vertices.push_back(points[current]);
        
        // Find the most counter-clockwise point from current
        int next = (current + 1) % static_cast<int>(points.size());
        
        for (size_t i = 0; i < points.size(); ++i) {
            if (static_cast<int>(i) == current) continue;
            
            // Use ToLeftTest to check if point i is more counter-clockwise than next
            if (ToLeftTest(points[current], points[i], points[next])) {
                next = static_cast<int>(i);
            } else if (!ToLeftTest(points[current], points[next], points[i]) &&
                       !ToLeftTest(points[current], points[i], points[next])) {
                // Points are collinear, choose the farther one
                double dist1 = (points[next] - points[current]).LengthSquared();
                double dist2 = (points[i] - points[current]).LengthSquared();
                if (dist2 > dist1) {
                    next = static_cast<int>(i);
                }
            }
        }
        
        current = next;
        
    } while (current != start_index);
    
    return ConvexHull(hull_vertices);
}

} // namespace geometry
