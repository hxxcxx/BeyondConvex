// ManagedVertex.cpp
#include "ManagedVertex.h"
#include "ManagedHalfEdge.h"

namespace BeyondConvexCLI {

    ManagedHalfEdge^ ManagedVertex::IncidentEdge::get() {
        if (m_native->GetIncidentEdge() == nullptr) {
            return nullptr;
        }
        return ManagedHalfEdge::FromNative(m_native->GetIncidentEdge());
    }

    void ManagedVertex::IncidentEdge::set(ManagedHalfEdge^ value) {
        m_native->SetIncidentEdge(value != nullptr ? value->GetNative() : nullptr);
    }

    void ManagedVertex::SetInternalManagedHalfEdge(ManagedHalfEdge^ edge) {
        m_native->SetIncidentEdge(edge != nullptr ? edge->GetNative() : nullptr);
    }
}
