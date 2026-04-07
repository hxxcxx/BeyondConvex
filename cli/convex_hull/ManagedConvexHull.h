// ManagedConvexHull.h
#pragma once

#include <vcclr.h>
#include <msclr/marshal.h>
#include "../../src/convex_hull/convex_hull.h"
#include "../core/ManagedPoint2D.h"
#include "../core/ManagedEdge2D.h"

namespace BeyondConvexCLI {

    /// <summary>
    /// Managed wrapper for ConvexHull
    /// </summary>
    public ref class ManagedConvexHull {
    public:
        ManagedConvexHull() : m_native(new geometry::ConvexHull()) {}
        
        ManagedConvexHull(cli::array<ManagedPoint2D^>^ points) : m_native(nullptr) {
            std::vector<geometry::Point2D> nativePoints;
            for each (ManagedPoint2D^ p in points) {
                nativePoints.push_back(*p->m_native);
            }
            m_native = new geometry::ConvexHull(nativePoints);
        }

        ~ManagedConvexHull() { delete m_native; }
        !ManagedConvexHull() { delete m_native; }

        property cli::array<ManagedPoint2D^>^ Vertices {
            cli::array<ManagedPoint2D^>^ get() {
                const auto& verts = m_native->GetVertices();
                cli::array<ManagedPoint2D^>^ result = gcnew cli::array<ManagedPoint2D^>(verts.size());
                for (size_t i = 0; i < verts.size(); ++i) {
                    result[i] = gcnew ManagedPoint2D(verts[i]);
                }
                return result;
            }
        }

        property size_t Size {
            size_t get() { return m_native->Size(); }
        }

        property bool IsEmpty {
            bool get() { return m_native->IsEmpty(); }
        }

        bool Contains(ManagedPoint2D^ point) {
            return m_native->Contains(*point->m_native);
        }

        bool ContainsByTangent(ManagedPoint2D^ point) {
            return m_native->ContainsByTangent(*point->m_native);
        }

        cli::array<ManagedEdge2D^>^ GetEdges() {
            auto edges = m_native->GetEdges();
            cli::array<ManagedEdge2D^>^ result = gcnew cli::array<ManagedEdge2D^>(edges.size());
            for (size_t i = 0; i < edges.size(); ++i) {
                result[i] = gcnew ManagedEdge2D(edges[i]);
            }
            return result;
        }

        double Area() { return m_native->Area(); }
        double Perimeter() { return m_native->Perimeter(); }

        ManagedPoint2D^ GetVertex(size_t index) {
            return gcnew ManagedPoint2D(m_native->GetVertex(index));
        }

        ManagedPoint2D^ GetPredecessor(size_t index) {
            return gcnew ManagedPoint2D(m_native->GetPredecessor(index));
        }

        ManagedPoint2D^ GetSuccessor(size_t index) {
            return gcnew ManagedPoint2D(m_native->GetSuccessor(index));
        }

        geometry::ConvexHull* GetNative() { return m_native; }

    internal:
        geometry::ConvexHull* m_native;
    };
}
