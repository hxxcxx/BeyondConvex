// ManagedVertex.h
#pragma once

#include <vcclr.h>
#include "../../src/dcel/vertex.h"
#include "../core/ManagedPoint2D.h"

namespace BeyondConvexCLI {

    // Forward declarations
    ref class ManagedHalfEdge;

    /// <summary>
    /// Managed wrapper for Vertex in DCEL structure
    /// </summary>
    public ref class ManagedVertex {
    public:
        ManagedVertex() : m_native(new geometry::Vertex()) {}
        ManagedVertex(const geometry::Vertex& native) : m_native(new geometry::Vertex(native)) {}
        ManagedVertex(ManagedPoint2D^ coordinates, int id) 
            : m_native(new geometry::Vertex(*coordinates->m_native, id)) {}
        
        ~ManagedVertex() { delete m_native; }

        property ManagedPoint2D^ Coordinates {
            ManagedPoint2D^ get() { return gcnew ManagedPoint2D(m_native->GetCoordinates()); }
        }

        property int Id {
            int get() { return m_native->GetId(); }
            void set(int value) { m_native->SetId(value); }
        }

        property ManagedHalfEdge^ IncidentEdge {
            ManagedHalfEdge^ get();
            void set(ManagedHalfEdge^ value);
        }

        property bool HasIncidentEdge {
            bool get() { return m_native->HasIncidentEdge(); }
        }

        int Degree() { return m_native->Degree(); }
        bool IsOnBoundary() { return m_native->IsOnBoundary(); }

        geometry::Vertex* GetNative() { return m_native; }

    internal:
        geometry::Vertex* m_native;
        void SetInternalManagedHalfEdge(ManagedHalfEdge^ edge);
    };
}
