// ManagedEdge2D.h
#pragma once

#include <vcclr.h>
#include "../../src/core/edge2d.h"
#include "ManagedPoint2D.h"

namespace BeyondConvexCLI {

    /// <summary>
    /// Managed wrapper for Edge2D
    /// </summary>
    public ref class ManagedEdge2D {
    public:
        ManagedEdge2D(ManagedPoint2D^ start, ManagedPoint2D^ end) 
            : m_native(new geometry::Edge2D(*start->m_native, *end->m_native)) {}

        ManagedEdge2D(const geometry::Edge2D& native) 
            : m_native(new geometry::Edge2D(native)) {}

        ~ManagedEdge2D() { delete m_native; }
        !ManagedEdge2D() { delete m_native; }

        property ManagedPoint2D^ Start {
            ManagedPoint2D^ get() { return gcnew ManagedPoint2D(m_native->p1); }
        }

        property ManagedPoint2D^ End {
            ManagedPoint2D^ get() { return gcnew ManagedPoint2D(m_native->p2); }
        }

        property double Length {
            double get() { return m_native->Length(); }
        }

        property double LengthSquared {
            double get() { return m_native->LengthSquared(); }
        }

        ManagedPoint2D^ GetMidpoint() {
            return gcnew ManagedPoint2D(m_native->Midpoint());
        }

        ManagedVector2D^ Direction() {
            return gcnew ManagedVector2D(m_native->Direction());
        }

        bool Contains(ManagedPoint2D^ point) {
            return m_native->Contains(*point->m_native);
        }

        String^ ToString() override {
            return String::Format("[{0} -> {1}]", Start->ToString(), End->ToString());
        }

        geometry::Edge2D* GetNative() { return m_native; }

    internal:
        geometry::Edge2D* m_native;
    };
}
