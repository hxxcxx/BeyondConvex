// ManagedPoint2D.h
#pragma once

#include <vcclr.h>
#include "../src/core/point2d.h"

namespace BeyondConvexCLI {

    /// <summary>
    /// Managed wrapper for Point2D
    /// </summary>
    public ref class ManagedPoint2D {
    public:
        ManagedPoint2D() : m_native(new geometry::Point2D()) {}
        ManagedPoint2D(double x, double y) : m_native(new geometry::Point2D(x, y)) {}
        ManagedPoint2D(const geometry::Point2D& native) : m_native(new geometry::Point2D(native)) {}
        ~ManagedPoint2D() { delete m_native; }
        !ManagedPoint2D() { delete m_native; }

        property double X {
            double get() { return m_native->x; }
            void set(double value) { m_native->x = value; }
        }

        property double Y {
            double get() { return m_native->y; }
            void set(double value) { m_native->y = value; }
        }

        double DistanceTo(ManagedPoint2D^ other) {
            return m_native->DistanceTo(*other->m_native);
        }

        double DistanceSquaredTo(ManagedPoint2D^ other) {
            return m_native->DistanceSquaredTo(*other->m_native);
        }

        // Cast operators
        static operator ManagedPoint2D^(const geometry::Point2D& native) {
            return gcnew ManagedPoint2D(native);
        }

        // Get native reference
        geometry::Point2D* GetNative() { return m_native; }
        const geometry::Point2D* GetNative() const { return m_native; }

    internal:
        geometry::Point2D* m_native;
    };
}
