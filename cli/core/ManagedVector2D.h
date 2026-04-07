// ManagedVector2D.h
#pragma once

#include <vcclr.h>
#include "../../src/core/vector2d.h"

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

        property double Magnitude {
            double get() { return m_native->Length(); }
        }

        property double MagnitudeSquared {
            double get() { return m_native->LengthSquared(); }
        }

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

        ManagedVector2D^ Add(ManagedVector2D^ other) {
            return gcnew ManagedVector2D(*m_native + *other->m_native);
        }

        ManagedVector2D^ Subtract(ManagedVector2D^ other) {
            return gcnew ManagedVector2D(*m_native - *other->m_native);
        }

        ManagedVector2D^ Multiply(double scalar) {
            return gcnew ManagedVector2D(*m_native * scalar);
        }

        ManagedVector2D^ Divide(double scalar) {
            return gcnew ManagedVector2D(*m_native / scalar);
        }

        ManagedVector2D^ Negate() {
            return gcnew ManagedVector2D(-*m_native);
        }

        String^ ToString() override {
            return String::Format("({0:F6}, {1:F6})", m_native->x, m_native->y);
        }

        geometry::Vector2D* GetNative() { return m_native; }
        const geometry::Vector2D* GetNative() const { return m_native; }

    internal:
        geometry::Vector2D* m_native;
    };
}
