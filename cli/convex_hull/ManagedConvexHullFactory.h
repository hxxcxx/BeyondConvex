// ManagedConvexHullFactory.h
#pragma once

#include <vcclr.h>
#include "../../src/convex_hull/convex_hull_factory.h"
#include "ManagedConvexHull.h"
#include "ManagedConvexHullBuilder.h"
#include "../core/ManagedPoint2D.h"

namespace BeyondConvexCLI {

    /// <summary>
    /// Managed factory for creating convex hulls with different algorithms
    /// </summary>
    public ref class ManagedConvexHullFactory {
    public:
        static ManagedConvexHull^ Create(
            ManagedConvexHullAlgorithm algorithm,
            cli::array<ManagedPoint2D^>^ points) 
        {
            std::vector<geometry::Point2D> nativePoints;
            for each (ManagedPoint2D^ p in points) {
                nativePoints.push_back(*p->m_native);
            }

            geometry::ConvexHullAlgorithm nativeAlgo = ToNativeAlgorithm(algorithm);
            auto result = geometry::ConvexHullFactory::Create(nativeAlgo, nativePoints);
            return gcnew ManagedConvexHull(result);
        }

        static bool IsAlgorithmSupported(ManagedConvexHullAlgorithm algorithm) {
            return geometry::ConvexHullFactory::IsAlgorithmSupported(ToNativeAlgorithm(algorithm));
        }

        static String^ GetAlgorithmName(ManagedConvexHullAlgorithm algorithm) {
            return gcnew String(geometry::GetAlgorithmName(ToNativeAlgorithm(algorithm)).c_str());
        }

        static String^ GetAlgorithmComplexity(ManagedConvexHullAlgorithm algorithm) {
            return gcnew String(geometry::GetAlgorithmComplexity(ToNativeAlgorithm(algorithm)).c_str());
        }

    private:
        static geometry::ConvexHullAlgorithm ToNativeAlgorithm(ManagedConvexHullAlgorithm algorithm) {
            switch (algorithm) {
                case ManagedConvexHullAlgorithm::JarvisMarch:
                    return geometry::ConvexHullAlgorithm::JarvisMarch;
                case ManagedConvexHullAlgorithm::GrahamScan:
                    return geometry::ConvexHullAlgorithm::GrahamScan;
                case ManagedConvexHullAlgorithm::QuickHull:
                    return geometry::ConvexHullAlgorithm::QuickHull;
                case ManagedConvexHullAlgorithm::MonotoneChain:
                    return geometry::ConvexHullAlgorithm::MonotoneChain;
                default:
                    return geometry::ConvexHullAlgorithm::MonotoneChain;
            }
        }
    };
}
