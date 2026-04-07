// ManagedConvexPolygonIntersection.h
#pragma once

#include <vcclr.h>
#include "../src/intersection/convex_polygon_intersection.h"
#include "ManagedConvexHull.h"
#include "ManagedPoint2D.h"

namespace BeyondConvexCLI {

    /// <summary>
    /// Algorithm type for convex polygon intersection
    /// </summary>
    public enum class ManagedConvexIntersectionAlgorithm {
        kLinearScan = 0,     // O(n + m) linear scan
        kBinarySearch = 1,   // O(log n + log m) binary search
    };

    /// <summary>
    /// Result of convex polygon intersection
    /// </summary>
    public ref class ManagedConvexIntersectionResult {
    public:
        ManagedConvexIntersectionResult() : m_native(new geometry::ConvexIntersectionResult()) {}
        ~ManagedConvexIntersectionResult() { delete m_native; }

        property cli::array<ManagedPoint2D^>^ Vertices {
            cli::array<ManagedPoint2D^>^ get() {
                const auto& verts = m_native->vertices;
                cli::array<ManagedPoint2D^>^ result = gcnew cli::array<ManagedPoint2D^>(verts.size());
                for (size_t i = 0; i < verts.size(); ++i) {
                    result[i] = gcnew ManagedPoint2D(verts[i]);
                }
                return result;
            }
        }

        property bool IsEmpty {
            bool get() { return m_native->is_empty; }
        }

        property bool IsPoint {
            bool get() { return m_native->is_point; }
        }

        property bool IsSegment {
            bool get() { return m_native->is_segment; }
        }

        ManagedConvexHull^ ToConvexHull() {
            auto hull = m_native->ToConvexHull();
            return gcnew ManagedConvexHull(hull);
        }

    internal:
        geometry::ConvexIntersectionResult* m_native;
    };

    /// <summary>
    /// Managed wrapper for ConvexPolygonIntersection algorithms
    /// </summary>
    public ref class ManagedConvexPolygonIntersection {
    public:
        static ManagedConvexIntersectionResult^ Intersect(
            ManagedConvexHull^ convex1,
            ManagedConvexHull^ convex2) 
        {
            auto result = geometry::ConvexPolygonIntersection::Intersect(
                *convex1->GetNative(), 
                *convex2->GetNative()
            );
            return gcnew ManagedConvexIntersectionResult(result);
        }

        static ManagedConvexIntersectionResult^ IntersectLinearScan(
            ManagedConvexHull^ convex1,
            ManagedConvexHull^ convex2) 
        {
            auto result = geometry::ConvexPolygonIntersection::IntersectLinearScan(
                *convex1->GetNative(), 
                *convex2->GetNative()
            );
            return gcnew ManagedConvexIntersectionResult(result);
        }

        static ManagedConvexIntersectionResult^ IntersectBinarySearch(
            ManagedConvexHull^ convex1,
            ManagedConvexHull^ convex2) 
        {
            auto result = geometry::ConvexPolygonIntersection::IntersectBinarySearch(
                *convex1->GetNative(), 
                *convex2->GetNative()
            );
            return gcnew ManagedConvexIntersectionResult(result);
        }

        static bool IsPointInConvexPolygon(
            ManagedPoint2D^ point, 
            ManagedConvexHull^ convex) 
        {
            return geometry::ConvexPolygonIntersection::IsPointInConvexPolygon(
                *point->m_native, 
                *convex->GetNative()
            );
        }
    };
}
