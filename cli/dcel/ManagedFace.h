// ManagedFace.h
#pragma once

#include <vcclr.h>
#include "../../src/dcel/face.h"
#include "ManagedHalfEdge.h"
#include "ManagedVertex.h"

namespace BeyondConvexCLI {

    /// <summary>
    /// Managed wrapper for Face in DCEL structure
    /// </summary>
    public ref class ManagedFace {
    public:
        ManagedFace() : m_native(new geometry::Face()) {}
        ManagedFace(const geometry::Face& native) : m_native(new geometry::Face(native)) {}
        ManagedFace(int id) : m_native(new geometry::Face(id)) {}
        
        ~ManagedFace() { delete m_native; }

        property int Id {
            int get() { return m_native->GetId(); }
            void set(int value) { m_native->SetId(value); }
        }

        property ManagedHalfEdge^ OuterComponent {
            ManagedHalfEdge^ get();
            void set(ManagedHalfEdge^ value);
        }

        property bool HasOuterComponent {
            bool get() { return m_native->HasOuterComponent(); }
        }

        property bool HasHoles {
            bool get() { return m_native->HasHoles(); }
        }

        size_t OuterBoundarySize() { return m_native->OuterBoundarySize(); }

        geometry::Face* GetNative() { return m_native; }

        // Factory method to create from native pointer
        static ManagedFace^ FromNative(geometry::Face* native);

    internal:
        geometry::Face* m_native;
        static ManagedFace^ CreateFromNative(geometry::Face* native);
    };
}
