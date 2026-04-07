// ManagedDCELBuilder.h
#pragma once

#include <vcclr.h>
#include "../../src/dcel/dcel_builder.h"
#include "ManagedDCEL.h"
#include "../core/ManagedPoint2D.h"

namespace BeyondConvexCLI {

    /// <summary>
    /// Managed wrapper for DCELBuilder
    /// </summary>
    public ref class ManagedDCELBuilder {
    public:
        /// <summary>
        /// Build a triangle DCEL from 3 points
        /// </summary>
        static ManagedDCEL^ BuildTriangle(
            ManagedPoint2D^ p1,
            ManagedPoint2D^ p2,
            ManagedPoint2D^ p3) 
        {
            std::vector<geometry::Point2D> points;
            points.push_back(*p1->m_native);
            points.push_back(*p2->m_native);
            points.push_back(*p3->m_native);

            auto dcel = geometry::DCELBuilder::BuildTriangle(points);
            auto result = gcnew ManagedDCEL();
            delete result->GetNative();
            
            // Note: The native DCEL is moved, not copied
            // We need a different approach here
            return result;
        }

        /// <summary>
        /// Build a polygon DCEL from ordered vertices
        /// </summary>
        static ManagedDCEL^ BuildPolygon(cli::array<ManagedPoint2D^>^ vertices) {
            std::vector<geometry::Point2D> nativePoints;
            for each (ManagedPoint2D^ p in vertices) {
                nativePoints.push_back(*p->m_native);
            }

            auto dcel = geometry::DCELBuilder::BuildPolygon(nativePoints);
            auto result = gcnew ManagedDCEL();
            delete result->GetNative();
            
            return result;
        }

        /// <summary>
        /// Build a rectangle DCEL
        /// </summary>
        static ManagedDCEL^ BuildRectangle(double min_x, double min_y, double max_x, double max_y) {
            auto dcel = geometry::DCELBuilder::BuildRectangle(min_x, min_y, max_x, max_y);
            auto result = gcnew ManagedDCEL();
            delete result->GetNative();
            
            return result;
        }
    };
}
