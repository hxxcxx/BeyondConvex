// ManagedVector2D.h
#pragma once

#include <vcclr.h>
#include "../src/core/vector2d.h"

namespace BeyondConvexCLI {

    /// <summary>
    /// Managed wrapper for Vector2D
    /// </summary>
    public ref class ManagedVector2D {
    public:
        ManagedVector2D() : m_native(new geometry::Vector2D()) {}
        ManagedVector2D(double x, double y) : m_native(new geometry::Vector2D(x, y)) {}
        ManagedVector2D(const geometry::Vector2D& native) : m_native(new geometry::Vector2D(native)) {}
        ~ManagedVector2D() { delete m_native; }
        !ManagedVector2D() { delete m_native; }

        property double X {
            double get() { return m_native->x; }
            void set(double value) { m_native->x = value; }
        }

        property double Y {
            double get() { return m_native->y; }
            void set(double value) { m_native->y = value; }
        }

        double Magnitude() const { return m_native->Magnitude(); }
        double MagnitudeSquared() const { return m_native->MagnitudeSquared(); }

        double Dot(ManagedVector2D^ other) {
            return m_native->Dot(*other->m_native);
        }

        double Cross(ManagedVector2D^ other) {
            return m_native->Cross(*other->m_native);
        }

        void Normalize() { m_native->Normalize(); }

        ManagedVector2D^ Normalized() {
            return gcnew ManagedVector2D(m_native->Normalized());
        }

        geometry::Vector2D* GetNative() { return m_native; }
        const geometry::Vector2D* GetNative() const { return m_native; }

    internal:
        geometry::Vector2D* m_native;
    };
}
