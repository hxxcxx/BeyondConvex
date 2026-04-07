// ManagedHalfEdge.cpp
#include "ManagedHalfEdge.h"
#include "ManagedVertex.h"
#include "ManagedFace.h"

namespace BeyondConvexCLI {

    ManagedVertex^ ManagedHalfEdge::Origin::get() {
        if (m_native->GetOrigin() == nullptr) {
            return nullptr;
        }
        return ManagedVertex::FromNative(m_native->GetOrigin());
    }

    void ManagedHalfEdge::Origin::set(ManagedVertex^ value) {
        m_native->SetOrigin(value != nullptr ? value->GetNative() : nullptr);
    }

    ManagedVertex^ ManagedHalfEdge::Destination::get() {
        return gcnew ManagedVertex(m_native->GetDestination());
    }

    ManagedHalfEdge^ ManagedHalfEdge::Twin::get() {
        if (m_native->GetTwin() == nullptr) {
            return nullptr;
        }
        return ManagedHalfEdge::FromNative(m_native->GetTwin());
    }

    void ManagedHalfEdge::Twin::set(ManagedHalfEdge^ value) {
        m_native->SetTwin(value != nullptr ? value->GetNative() : nullptr);
    }

    ManagedHalfEdge^ ManagedHalfEdge::Next::get() {
        if (m_native->GetNext() == nullptr) {
            return nullptr;
        }
        return ManagedHalfEdge::FromNative(m_native->GetNext());
    }

    void ManagedHalfEdge::Next::set(ManagedHalfEdge^ value) {
        m_native->SetNext(value != nullptr ? value->GetNative() : nullptr);
    }

    ManagedHalfEdge^ ManagedHalfEdge::Prev::get() {
        if (m_native->GetPrev() == nullptr) {
            return nullptr;
        }
        return ManagedHalfEdge::FromNative(m_native->GetPrev());
    }

    void ManagedHalfEdge::Prev::set(ManagedHalfEdge^ value) {
        m_native->SetPrev(value != nullptr ? value->GetNative() : nullptr);
    }

    ManagedFace^ ManagedHalfEdge::Face::get() {
        if (m_native->GetFace() == nullptr) {
            return nullptr;
        }
        return ManagedFace::FromNative(m_native->GetFace());
    }

    void ManagedHalfEdge::Face::set(ManagedFace^ value) {
        m_native->SetFace(value != nullptr ? value->GetNative() : nullptr);
    }

    ManagedHalfEdge^ ManagedHalfEdge::FromNative(geometry::HalfEdge* native) {
        return gcnew ManagedHalfEdge(*native);
    }
}
