// ManagedTriangulationTypes.h
#pragma once

#include <vcclr.h>
#include "../../src/triangulation/triangulation_types.h"
#include "../core/ManagedPoint2D.h"
#include "../core/ManagedEdge2D.h"

namespace BeyondConvexCLI {

    /// <summary>
    /// Managed Triangle representation
    /// </summary>
    public ref class ManagedTriangle {
    public:
        ManagedTriangle(ManagedPoint2D^ v0, ManagedPoint2D^ v1, ManagedPoint2D^ v2)
            : m_native(new geometry::Triangle(*v0->m_native, *v1->m_native, *v2->m_native)) {}

        ManagedTriangle(const geometry::Triangle& native)
            : m_native(new geometry::Triangle(native)) {}

        ~ManagedTriangle() { delete m_native; }

        property ManagedPoint2D^ V0 {
            ManagedPoint2D^ get() { return gcnew ManagedPoint2D(m_native->v0); }
        }

        property ManagedPoint2D^ V1 {
            ManagedPoint2D^ get() { return gcnew ManagedPoint2D(m_native->v1); }
        }

        property ManagedPoint2D^ V2 {
            ManagedPoint2D^ get() { return gcnew ManagedPoint2D(m_native->v2); }
        }

        double Area() { return m_native->Area(); }

        bool Contains(ManagedPoint2D^ p) {
            return m_native->Contains(*p->m_native);
        }

        ManagedPoint2D^ Circumcenter() {
            return gcnew ManagedPoint2D(m_native->Circumcenter());
        }

        double Circumradius() { return m_native->Circumradius(); }

        bool IsDegenerate() { return m_native->IsDegenerate(); }

        geometry::Triangle* GetNative() { return m_native; }

    private:
        geometry::Triangle* m_native;
    };

    /// <summary>
    /// Managed Triangulation result
    /// </summary>
    public ref class ManagedTriangulationResult {
    public:
        ManagedTriangulationResult() : m_native(new geometry::TriangulationResult()) {}
        ManagedTriangulationResult(const geometry::TriangulationResult& native)
            : m_native(new geometry::TriangulationResult(native)) {}
        ~ManagedTriangulationResult() { delete m_native; }

        property cli::array<ManagedTriangle^>^ Triangles {
            cli::array<ManagedTriangle^>^ get() {
                cli::array<ManagedTriangle^>^ result = gcnew cli::array<ManagedTriangle^>(
                    m_native->triangles.size());
                for (size_t i = 0; i < m_native->triangles.size(); ++i) {
                    result[i] = gcnew ManagedTriangle(m_native->triangles[i]);
                }
                return result;
            }
        }

        property cli::array<ManagedPoint2D^>^ Vertices {
            cli::array<ManagedPoint2D^>^ get() {
                cli::array<ManagedPoint2D^>^ result = gcnew cli::array<ManagedPoint2D^>(
                    m_native->vertices.size());
                for (size_t i = 0; i < m_native->vertices.size(); ++i) {
                    result[i] = gcnew ManagedPoint2D(m_native->vertices[i]);
                }
                return result;
            }
        }

        property cli::array<ManagedEdge2D^>^ Edges {
            cli::array<ManagedEdge2D^>^ get() {
                cli::array<ManagedEdge2D^>^ result = gcnew cli::array<ManagedEdge2D^>(
                    m_native->edges.size());
                for (size_t i = 0; i < m_native->edges.size(); ++i) {
                    result[i] = gcnew ManagedEdge2D(m_native->edges[i]);
                }
                return result;
            }
        }

        property bool IsValid {
            bool get() { return m_native->IsValid(); }
        }

        property size_t TriangleCount {
            size_t get() { return m_native->TriangleCount(); }
        }

        geometry::TriangulationResult* GetNative() { return m_native; }

    private:
        geometry::TriangulationResult* m_native;
    };
}
