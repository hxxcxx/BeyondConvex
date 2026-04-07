// ManagedHalfEdge.h
#pragma once

#include <vcclr.h>
#include "../../src/dcel/half_edge.h"
#include "ManagedVertex.h"
#include "ManagedFace.h"

namespace BeyondConvexCLI {

    /// <summary>
    /// Managed wrapper for HalfEdge in DCEL structure
    /// </summary>
    public ref class ManagedHalfEdge {
    public:
        ManagedHalfEdge() : m_native(new geometry::HalfEdge()) {}
        ManagedHalfEdge(const geometry::HalfEdge& native) : m_native(new geometry::HalfEdge(native)) {}
        ManagedHalfEdge(int id) : m_native(new geometry::HalfEdge(id)) {}
        
        ~ManagedHalfEdge() { delete m_native; }

        property int Id {
            int get() { return m_native->GetId(); }
            void set(int value) { m_native->SetId(value); }
        }

        property ManagedVertex^ Origin {
            ManagedVertex^ get();
            void set(ManagedVertex^ value);
        }

        property ManagedVertex^ Destination {
            ManagedVertex^ get();
        }

        property ManagedHalfEdge^ Twin {
            ManagedHalfEdge^ get();
            void set(ManagedHalfEdge^ value);
        }

        property ManagedHalfEdge^ Next {
            ManagedHalfEdge^ get();
            void set(ManagedHalfEdge^ value);
        }

        property ManagedHalfEdge^ Prev {
            ManagedHalfEdge^ get();
            void set(ManagedHalfEdge^ value);
        }

        property ManagedFace^ Face {
            ManagedFace^ get();
            void set(ManagedFace^ value);
        }

        property bool HasTwin {
            bool get() { return m_native->HasTwin(); }
        }

        property bool HasNext {
            bool get() { return m_native->HasNext(); }
        }

        property bool HasPrev {
            bool get() { return m_native->HasPrev(); }
        }

        property bool HasFace {
            bool get() { return m_native->HasFace(); }
        }

        bool IsOnBoundary() { return m_native->IsOnBoundary(); }

        geometry::HalfEdge* GetNative() { return m_native; }

        // Factory method to create from native pointer
        static ManagedHalfEdge^ FromNative(geometry::HalfEdge* native);

    internal:
        geometry::HalfEdge* m_native;
        static ManagedHalfEdge^ CreateFromNative(geometry::HalfEdge* native);
    };
}
