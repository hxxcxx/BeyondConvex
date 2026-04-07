// ManagedConvexHullBuilder.h
#pragma once

#include <vcclr.h>
#include "../../src/convex_hull/convex_hull_builder.h"
#include "ManagedConvexHull.h"
#include "../core/ManagedPoint2D.h"

namespace BeyondConvexCLI {

    /// <summary>
    /// Supported convex hull algorithms
    /// </summary>
    public enum class ManagedConvexHullAlgorithm {
        JarvisMarch = 0,    // O(nh) - Gift Wrapping
        GrahamScan = 1,     // O(n log n) - Sort by angle
        QuickHull = 2,      // O(n log n) average
        MonotoneChain = 3  // O(n log n) - Sort by x then y
    };

    /// <summary>
    /// Managed wrapper for ConvexHullBuilder
    /// </summary>
    public ref class ManagedConvexHullBuilder {
    public:
        static ManagedConvexHull^ BuildJarvisMarch(cli::array<ManagedPoint2D^>^ points) {
            std::vector<geometry::Point2D> nativePoints;
            for each (ManagedPoint2D^ p in points) {
                nativePoints.push_back(*p->m_native);
            }
            auto result = geometry::ConvexHullBuilder::BuildJarvisMarch(nativePoints);
            return gcnew ManagedConvexHull(result);
        }

        static ManagedConvexHull^ BuildGrahamScan(cli::array<ManagedPoint2D^>^ points) {
            std::vector<geometry::Point2D> nativePoints;
            for each (ManagedPoint2D^ p in points) {
                nativePoints.push_back(*p->m_native);
            }
            auto result = geometry::ConvexHullBuilder::BuildGrahamScan(nativePoints);
            return gcnew ManagedConvexHull(result);
        }

        static ManagedConvexHull^ BuildMonotoneChain(cli::array<ManagedPoint2D^>^ points) {
            std::vector<geometry::Point2D> nativePoints;
            for each (ManagedPoint2D^ p in points) {
                nativePoints.push_back(*p->m_native);
            }
            auto result = geometry::ConvexHullBuilder::BuildMonotoneChain(nativePoints);
            return gcnew ManagedConvexHull(result);
        }
    };
}
