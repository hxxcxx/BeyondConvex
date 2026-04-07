// ManagedDCEL.h
#pragma once

#include <vcclr.h>
#include "../../src/dcel/dcel.h"
#include "ManagedVertex.h"
#include "ManagedHalfEdge.h"
#include "ManagedFace.h"
#include "../core/ManagedPoint2D.h"

namespace BeyondConvexCLI {

    /// <summary>
    /// Managed wrapper for DCEL (Doubly Connected Edge List) data structure
    /// </summary>
    public ref class ManagedDCEL {
    public:
        ManagedDCEL() : m_native(new geometry::DCEL()) {}
        ~ManagedDCEL() { delete m_native; }

        // ========== Vertex Operations ==========

        ManagedVertex^ CreateVertex(ManagedPoint2D^ coordinates) {
            auto vertex = m_native->CreateVertex(*coordinates->m_native);
            return gcnew ManagedVertex(*vertex);
        }

        property cli::array<ManagedVertex^>^ Vertices {
            cli::array<ManagedVertex^>^ get() {
                const auto& verts = m_native->GetVertices();
                cli::array<ManagedVertex^>^ result = gcnew cli::array<ManagedVertex^>(verts.size());
                for (size_t i = 0; i < verts.size(); ++i) {
                    result[i] = gcnew ManagedVertex(*verts[i]);
                }
                return result;
            }
        }

        property size_t VertexCount {
            size_t get() { return m_native->GetVertexCount(); }
        }

        // ========== Half-Edge Operations ==========

        ManagedHalfEdge^ CreateEdge(ManagedVertex^ origin, ManagedVertex^ twinOrigin) {
            auto edge = m_native->CreateEdge(origin->GetNative(), twinOrigin->GetNative());
            return gcnew ManagedHalfEdge(*edge);
        }

        property cli::array<ManagedHalfEdge^>^ HalfEdges {
            cli::array<ManagedHalfEdge^>^ get() {
                const auto& edges = m_native->GetHalfEdges();
                cli::array<ManagedHalfEdge^>^ result = gcnew cli::array<ManagedHalfEdge^>(edges.size());
                for (size_t i = 0; i < edges.size(); ++i) {
                    result[i] = gcnew ManagedHalfEdge(*edges[i]);
                }
                return result;
            }
        }

        property size_t HalfEdgeCount {
            size_t get() { return m_native->GetHalfEdgeCount(); }
        }

        property size_t EdgeCount {
            size_t get() { return m_native->GetEdgeCount(); }
        }

        // ========== Face Operations ==========

        ManagedFace^ CreateFace() {
            auto face = m_native->CreateFace();
            return gcnew ManagedFace(*face);
        }

        property cli::array<ManagedFace^>^ Faces {
            cli::array<ManagedFace^>^ get() {
                const auto& faces = m_native->GetFaces();
                cli::array<ManagedFace^>^ result = gcnew cli::array<ManagedFace^>(faces.size());
                for (size_t i = 0; i < faces.size(); ++i) {
                    result[i] = gcnew ManagedFace(*faces[i]);
                }
                return result;
            }
        }

        property size_t FaceCount {
            size_t get() { return m_native->GetFaceCount(); }
        }

        // ========== Topology Operations ==========

        void ConnectHalfEdges(ManagedHalfEdge^ prev, ManagedHalfEdge^ next) {
            m_native->ConnectHalfEdges(prev->GetNative(), next->GetNative());
        }

        void SetFaceOfCycle(ManagedHalfEdge^ start, ManagedFace^ face) {
            m_native->SetFaceOfCycle(start->GetNative(), face->GetNative());
        }

        ManagedHalfEdge^ SplitFace(ManagedFace^ face, ManagedVertex^ v1, ManagedVertex^ v2) {
            auto edge = m_native->SplitFace(face->GetNative(), v1->GetNative(), v2->GetNative());
            return gcnew ManagedHalfEdge(*edge);
        }

        ManagedFace^ MergeFaces(ManagedHalfEdge^ edge) {
            auto face = m_native->MergeFaces(edge->GetNative());
            return gcnew ManagedFace(*face);
        }

        // ========== Validation ==========

        bool Validate() {
            return m_native->Validate();
        }

        void Clear() {
            m_native->Clear();
        }

        void PrintStats() {
            m_native->PrintStats();
        }

        geometry::DCEL* GetNative() { return m_native; }

    private:
        geometry::DCEL* m_native;
    };
}
