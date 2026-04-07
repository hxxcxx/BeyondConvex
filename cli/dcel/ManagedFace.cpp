// ManagedFace.cpp
#include "ManagedFace.h"
#include "ManagedHalfEdge.h"

namespace BeyondConvexCLI {

    ManagedHalfEdge^ ManagedFace::OuterComponent::get() {
        if (m_native->GetOuterComponent() == nullptr) {
            return nullptr;
        }
        return ManagedHalfEdge::FromNative(m_native->GetOuterComponent());
    }

    void ManagedFace::OuterComponent::set(ManagedHalfEdge^ value) {
        m_native->SetOuterComponent(value != nullptr ? value->GetNative() : nullptr);
    }

    ManagedFace^ ManagedFace::FromNative(geometry::Face* native) {
        return gcnew ManagedFace(*native);
    }
}
