// ManagedTriangulation.h
#pragma once

#include <vcclr.h>
#include "../../src/triangulation/triangulation.h"
#include "ManagedTriangulationTypes.h"
#include "../core/ManagedPoint2D.h"

namespace BeyondConvexCLI {

    /// <summary>
    /// Supported triangulation algorithms
    /// </summary>
    public enum class ManagedTriangulationAlgorithmType {
        kEarClipping = 0,          // Ear clipping algorithm
        kSweepLine = 1,           // Sweep line algorithm
        kDelaunay = 2,            // Delaunay triangulation
        kDelaunayOptimized = 3   // Optimized Delaunay
    };

    /// <summary>
    /// Managed wrapper for Triangulation facade class
    /// </summary>
    public ref class ManagedTriangulation {
    public:
        /// <summary>
        /// Triangulate a polygon or point set using default algorithm
        /// </summary>
        static ManagedTriangulationResult^ Triangulate(
            cli::array<ManagedPoint2D^>^ points) 
        {
            std::vector<geometry::Point2D> nativePoints;
            for each (ManagedPoint2D^ p in points) {
                nativePoints.push_back(*p->m_native);
            }

            auto result = geometry::Triangulation::Triangulate(nativePoints);
            return gcnew ManagedTriangulationResult(result);
        }

        /// <summary>
        /// Triangulate using specified algorithm
        /// </summary>
        static ManagedTriangulationResult^ Triangulate(
            cli::array<ManagedPoint2D^>^ points,
            ManagedTriangulationAlgorithmType algorithm) 
        {
            std::vector<geometry::Point2D> nativePoints;
            for each (ManagedPoint2D^ p in points) {
                nativePoints.push_back(*p->m_native);
            }

            auto result = geometry::Triangulation::Triangulate(
                nativePoints, ToNativeAlgorithm(algorithm));
            return gcnew ManagedTriangulationResult(result);
        }

        static cli::array<String^>^ GetSupportedAlgorithms() {
            auto algs = geometry::Triangulation::GetSupportedAlgorithms();
            cli::array<String^>^ result = gcnew cli::array<String^>(algs.size());
            for (size_t i = 0; i < algs.size(); ++i) {
                result[i] = gcnew String(geometry::Triangulation::GetAlgorithmName(algs[i]).c_str());
            }
            return result;
        }

        static String^ GetAlgorithmName(ManagedTriangulationAlgorithmType algorithm) {
            return gcnew String(geometry::Triangulation::GetAlgorithmName(
                ToNativeAlgorithm(algorithm)).c_str());
        }

        static String^ GetAlgorithmComplexity(ManagedTriangulationAlgorithmType algorithm) {
            return gcnew String(geometry::Triangulation::GetAlgorithmComplexity(
                ToNativeAlgorithm(algorithm)).c_str());
        }

    private:
        static geometry::TriangulationAlgorithmType ToNativeAlgorithm(
            ManagedTriangulationAlgorithmType algorithm) {
            switch (algorithm) {
                case ManagedTriangulationAlgorithmType::kEarClipping:
                    return geometry::TriangulationAlgorithmType::kEarClipping;
                case ManagedTriangulationAlgorithmType::kSweepLine:
                    return geometry::TriangulationAlgorithmType::kSweepLine;
                case ManagedTriangulationAlgorithmType::kDelaunay:
                    return geometry::TriangulationAlgorithmType::kDelaunay;
                case ManagedTriangulationAlgorithmType::kDelaunayOptimized:
                    return geometry::TriangulationAlgorithmType::kDelaunayOptimized;
                default:
                    return geometry::TriangulationAlgorithmType::kSweepLine;
            }
        }
    };
}
