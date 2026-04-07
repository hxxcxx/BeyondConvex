// ManagedLineSegmentIntersection.h
#pragma once

#include <vcclr.h>
#include "../../src/intersection/line_segment_intersection.h"
#include "../core/ManagedPoint2D.h"
#include "../core/ManagedEdge2D.h"

namespace BeyondConvexCLI {

    /// <summary>
    /// Managed intersection point result
    /// </summary>
    public ref class ManagedIntersectionPoint {
    public:
        ManagedIntersectionPoint(const geometry::IntersectionPoint& native)
            : m_point(gcnew ManagedPoint2D(native.point)),
              m_segment1_index(native.segment1_index),
              m_segment2_index(native.segment2_index) {}

        property ManagedPoint2D^ Point {
            ManagedPoint2D^ get() { return m_point; }
        }

        property int Segment1Index {
            int get() { return m_segment1_index; }
        }

        property int Segment2Index {
            int get() { return m_segment2_index; }
        }

    private:
        ManagedPoint2D^ m_point;
        int m_segment1_index;
        int m_segment2_index;
    };

    /// <summary>
    /// Managed result for line segment intersection operations
    /// </summary>
    public ref class ManagedLineSegmentIntersectionResult {
    public:
        ManagedLineSegmentIntersectionResult() : m_has_intersection(false) {}
        
        ManagedLineSegmentIntersectionResult(
            bool hasIntersection,
            cli::array<ManagedPoint2D^>^ points,
            cli::array<ManagedIntersectionPoint^>^ intersectionPoints)
            : m_has_intersection(hasIntersection),
              m_points(points),
              m_intersection_points(intersectionPoints) {}

        property bool HasIntersection {
            bool get() { return m_has_intersection; }
        }

        property cli::array<ManagedPoint2D^>^ Points {
            cli::array<ManagedPoint2D^>^ get() { return m_points; }
        }

        property cli::array<ManagedIntersectionPoint^>^ IntersectionPoints {
            cli::array<ManagedIntersectionPoint^>^ get() { return m_intersection_points; }
        }

    private:
        bool m_has_intersection;
        cli::array<ManagedPoint2D^>^ m_points;
        cli::array<ManagedIntersectionPoint^>^ m_intersection_points;
    };

    /// <summary>
    /// Managed wrapper for line segment intersection operations
    /// </summary>
    public ref class ManagedLineSegmentIntersection {
    public:
        /// <summary>
        /// Find all intersection points between line segments
        /// </summary>
        static ManagedLineSegmentIntersectionResult^ FindAllIntersections(
            cli::array<ManagedEdge2D^>^ segments) 
        {
            std::vector<geometry::Edge2D> nativeSegments;
            for each (ManagedEdge2D^ seg in segments) {
                nativeSegments.push_back(*seg->GetNative());
            }

            std::vector<geometry::IntersectionPoint> nativeResult;
            bool hasIntersection = 
                geometry::FindAllSegmentIntersections(nativeSegments, nativeResult);

            cli::array<ManagedPoint2D^>^ points = nullptr;
            cli::array<ManagedIntersectionPoint^>^ intPoints = nullptr;

            if (hasIntersection && !nativeResult.empty()) {
                points = gcnew cli::array<ManagedPoint2D^>(nativeResult.size());
                intPoints = gcnew cli::array<ManagedIntersectionPoint^>(nativeResult.size());
                for (size_t i = 0; i < nativeResult.size(); ++i) {
                    points[i] = gcnew ManagedPoint2D(nativeResult[i].point);
                    intPoints[i] = gcnew ManagedIntersectionPoint(nativeResult[i]);
                }
            }

            return gcnew ManagedLineSegmentIntersectionResult(
                hasIntersection, points, intPoints);
        }

        /// <summary>
        /// Check if two line segments intersect
        /// </summary>
        static bool DoSegmentsIntersect(
            ManagedPoint2D^ p1, ManagedPoint2D^ p2,
            ManagedPoint2D^ q1, ManagedPoint2D^ q2) 
        {
            return geometry::DoSegmentsIntersect(
                *p1->m_native, *p2->m_native,
                *q1->m_native, *q2->m_native);
        }

        /// <summary>
        /// Find intersection point of two line segments (if they intersect)
        /// </summary>
        static ManagedPoint2D^ GetIntersectionPoint(
            ManagedPoint2D^ p1, ManagedPoint2D^ p2,
            ManagedPoint2D^ q1, ManagedPoint2D^ q2) 
        {
            geometry::Point2D result;
            bool found = geometry::GetIntersectionPoint(
                *p1->m_native, *p2->m_native,
                *q1->m_native, *q2->m_native,
                result);
            if (found) {
                return gcnew ManagedPoint2D(result);
            }
            return nullptr;
        }
    };
}
