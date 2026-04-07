// ManagedEdge2D.h
#pragma once

#include <vcclr.h>
#include "../src/core/edge2d.h"
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
            ManagedPoint2D^ get() { return gcnew ManagedPoint2D(m_native->start); }
        }

        property ManagedPoint2D^ End {
            ManagedPoint2D^ get() { return gcnew ManagedPoint2D(m_native->end); }
        }

        double Length() const { return m_native->Length(); }
        double LengthSquared() const { return m_native->LengthSquared(); }

        ManagedPoint2D^ GetMidpoint() {
            return gcnew ManagedPoint2D(m_native->Midpoint());
        }

        geometry::Edge2D* GetNative() { return m_native; }

    internal:
        geometry::Edge2D* m_native;
    };
}
