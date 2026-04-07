// ManagedVoronoiTypes.h
#pragma once

#include <vcclr.h>
#include "../../src/voronoi/voronoi_types.h"
#include "../core/ManagedPoint2D.h"
#include "../core/ManagedEdge2D.h"

namespace BeyondConvexCLI {

    /// <summary>
    /// Managed Voronoi cell representation
    /// </summary>
    public ref class ManagedVoronoiCell {
    public:
        ManagedVoronoiCell(const geometry::VoronoiCell& native)
            : m_site_index(native.site_index),
              m_site(gcnew ManagedPoint2D(native.site)) 
        {
            m_vertices = gcnew cli::array<ManagedPoint2D^>(native.vertices.size());
            for (size_t i = 0; i < native.vertices.size(); ++i) {
                m_vertices[i] = gcnew ManagedPoint2D(native.vertices[i]);
            }

            m_edges = gcnew cli::array<ManagedEdge2D^>(native.edges.size());
            for (size_t i = 0; i < native.edges.size(); ++i) {
                m_edges[i] = gcnew ManagedEdge2D(native.edges[i]);
            }
        }

        property size_t SiteIndex {
            size_t get() { return m_site_index; }
        }

        property ManagedPoint2D^ Site {
            ManagedPoint2D^ get() { return m_site; }
        }

        property cli::array<ManagedPoint2D^>^ Vertices {
            cli::array<ManagedPoint2D^>^ get() { return m_vertices; }
        }

        property cli::array<ManagedEdge2D^>^ Edges {
            cli::array<ManagedEdge2D^>^ get() { return m_edges; }
        }

        bool IsValid() { return m_native.IsValid(); }

    private:
        size_t m_site_index;
        ManagedPoint2D^ m_site;
        cli::array<ManagedPoint2D^>^ m_vertices;
        cli::array<ManagedEdge2D^>^ m_edges;
        geometry::VoronoiCell m_native;
    };

    /// <summary>
    /// Managed Voronoi diagram result
    /// </summary>
    public ref class ManagedVoronoiDiagramResult {
    public:
        ManagedVoronoiDiagramResult(const geometry::VoronoiDiagramResult& native)
            : m_native(native) 
        {
            m_sites = gcnew cli::array<ManagedPoint2D^>(native.sites.size());
            for (size_t i = 0; i < native.sites.size(); ++i) {
                m_sites[i] = gcnew ManagedPoint2D(native.sites[i]);
            }

            m_cells = gcnew cli::array<ManagedVoronoiCell^>(native.cells.size());
            for (size_t i = 0; i < native.cells.size(); ++i) {
                m_cells[i] = gcnew ManagedVoronoiCell(native.cells[i]);
            }

            m_edges = gcnew cli::array<ManagedEdge2D^>(native.edges.size());
            for (size_t i = 0; i < native.edges.size(); ++i) {
                m_edges[i] = gcnew ManagedEdge2D(native.edges[i]);
            }

            m_vertices = gcnew cli::array<ManagedPoint2D^>(native.vertices.size());
            for (size_t i = 0; i < native.vertices.size(); ++i) {
                m_vertices[i] = gcnew ManagedPoint2D(native.vertices[i]);
            }
        }

        property cli::array<ManagedPoint2D^>^ Sites {
            cli::array<ManagedPoint2D^>^ get() { return m_sites; }
        }

        property cli::array<ManagedVoronoiCell^>^ Cells {
            cli::array<ManagedVoronoiCell^>^ get() { return m_cells; }
        }

        property cli::array<ManagedEdge2D^>^ Edges {
            cli::array<ManagedEdge2D^>^ get() { return m_edges; }
        }

        property cli::array<ManagedPoint2D^>^ Vertices {
            cli::array<ManagedPoint2D^>^ get() { return m_vertices; }
        }

        property bool IsValid {
            bool get() { return m_native.IsValid(); }
        }

        property size_t CellCount {
            size_t get() { return m_native.CellCount(); }
        }

    private:
        geometry::VoronoiDiagramResult m_native;
        cli::array<ManagedPoint2D^>^ m_sites;
        cli::array<ManagedVoronoiCell^>^ m_cells;
        cli::array<ManagedEdge2D^>^ m_edges;
        cli::array<ManagedPoint2D^>^ m_vertices;
    };
}
