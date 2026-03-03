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

std::vector<Edge2D> GeometryUtils::JarvisMarch(const std::vector<Point2D>& points) {
    std::vector<Edge2D> hull;
    
    if (points.size() < 3) {
        return hull;
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
    int prev = -1;
    
    do {
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
        
        // Create edge from prev to current
        if (prev != -1) {
            hull.push_back(Edge2D(points[prev], points[current]));
        }
        
        prev = current;
        current = next;
        
    } while (current != start_index);
    
    // Add the last edge to close the hull
    if (prev != -1) {
        hull.push_back(Edge2D(points[prev], points[start_index]));
    }
    
    return hull;
}

bool GeometryUtils::IsPointInConvexHull(const std::vector<Edge2D>& hull,
                                        const Point2D& point) {
    if (hull.size() < 3) return false;
    
    // Check if point is to the left of all hull edges (for counter-clockwise hull)
    for (const auto& edge : hull) {
        if (!ToLeftTest(edge.p1, edge.p2, point)) {
            return false;
        }
    }
    
    return true;
}

} // namespace geometry
