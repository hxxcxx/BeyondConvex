// ManagedVoronoiDiagram.h
#pragma once

#include <vcclr.h>
#include "../../src/voronoi/voronoi_diagram.h"
#include "ManagedVoronoiTypes.h"
#include "../core/ManagedPoint2D.h"
#include "../dcel/ManagedDCEL.h"

namespace BeyondConvexCLI {

    /// <summary>
    /// Supported Voronoi algorithms
    /// </summary>
    public enum class ManagedVoronoiAlgorithm {
        kIncremental = 0,         // Incremental half-plane clipping
        kIncrementalDCEL = 1    // Incremental using DCEL structure
    };

    /// <summary>
    /// Managed wrapper for VoronoiDiagram
    /// </summary>
    public ref class ManagedVoronoiDiagram {
    public:
        /// <summary>
        /// Generate Voronoi diagram from sites using default algorithm
        /// </summary>
        static ManagedVoronoiDiagramResult^ Generate(
            cli::array<ManagedPoint2D^>^ sites,
            double boundsMinX, double boundsMinY,
            double boundsMaxX, double boundsMaxY)
        {
            std::vector<geometry::Point2D> nativeSites;
            for each (ManagedPoint2D^ p in sites) {
                nativeSites.push_back(*p->m_native);
            }

            auto result = geometry::VoronoiDiagram::Generate(
                nativeSites, boundsMinX, boundsMinY, boundsMaxX, boundsMaxY);
            return gcnew ManagedVoronoiDiagramResult(result);
        }

        /// <summary>
        /// Generate Voronoi diagram with specified algorithm
        /// </summary>
        static ManagedVoronoiDiagramResult^ Generate(
            cli::array<ManagedPoint2D^>^ sites,
            ManagedVoronoiAlgorithm algorithm,
            double boundsMinX, double boundsMinY,
            double boundsMaxX, double boundsMaxY)
        {
            std::vector<geometry::Point2D> nativeSites;
            for each (ManagedPoint2D^ p in sites) {
                nativeSites.push_back(*p->m_native);
            }

            auto result = geometry::VoronoiDiagram::Generate(
                nativeSites, ToNativeAlgorithm(algorithm),
                boundsMinX, boundsMinY, boundsMaxX, boundsMaxY);
            return gcnew ManagedVoronoiDiagramResult(result);
        }

        /// <summary>
        /// Generate DCEL-format Voronoi diagram
        /// </summary>
        static ManagedDCEL^ GenerateDCEL(
            cli::array<ManagedPoint2D^>^ sites,
            double boundsMinX, double boundsMinY,
            double boundsMaxX, double boundsMaxY)
        {
            std::vector<geometry::Point2D> nativeSites;
            for each (ManagedPoint2D^ p in sites) {
                nativeSites.push_back(*p->m_native);
            }

            auto dcel = geometry::VoronoiDiagram::GenerateDCEL(
                nativeSites, geometry::VoronoiAlgorithm::kIncrementalDCEL,
                boundsMinX, boundsMinY, boundsMaxX, boundsMaxY);
            
            // Wrap the DCEL
            auto result = gcnew ManagedDCEL();
            delete result->GetNative();
            // Note: Need to properly transfer ownership
            
            return result;
        }

    private:
        static geometry::VoronoiAlgorithm ToNativeAlgorithm(ManagedVoronoiAlgorithm algorithm) {
            switch (algorithm) {
                case ManagedVoronoiAlgorithm::kIncremental:
                    return geometry::VoronoiAlgorithm::kIncremental;
                case ManagedVoronoiAlgorithm::kIncrementalDCEL:
                    return geometry::VoronoiAlgorithm::kIncrementalDCEL;
                default:
                    return geometry::VoronoiAlgorithm::kIncremental;
            }
        }
    };
}
