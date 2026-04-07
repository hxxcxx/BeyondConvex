// ManagedGeometryCore.h
#pragma once

#include <vcclr.h>
#include "../../src/core/geometry_core.h"
#include "ManagedPoint2D.h"
#include "ManagedVector2D.h"

namespace BeyondConvexCLI {

    /// <summary>
    /// Managed wrapper for GeometryCore static methods
    /// </summary>
    public ref class ManagedGeometryCore {
    public:
        /// <summary>
        /// To-Left Test: Check if point r is to the left of the directed line pq
        /// </summary>
        static bool ToLeftTest(ManagedPoint2D^ p, ManagedPoint2D^ q, ManagedPoint2D^ r) {
            return geometry::internal::GeometryCore::ToLeftTest(*p->m_native, *q->m_native, *r->m_native);
        }

        /// <summary>
        /// To-Left Test with vectors: Check if vector v is to the left of vector u
        /// </summary>
        static bool ToLeftTest(ManagedVector2D^ u, ManagedVector2D^ v) {
            return geometry::internal::GeometryCore::ToLeftTest(*u->m_native, *v->m_native);
        }
    };
}
