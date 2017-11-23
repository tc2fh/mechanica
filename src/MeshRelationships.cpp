/*
 * MeshRelationships.cpp
 *
 *  Created on: Sep 29, 2017
 *      Author: andy
 */

#include <MeshRelationships.h>
#include <algorithm>

bool incident(const TrianglePtr t, const CellPtr c) {
    return t->cells[0] == c || t->cells[1] == c;
}

bool adjacent(const TrianglePtr a, const TrianglePtr b) {
    if(a == b) {
        return false;
    }

    for(int k = 0; k < 3; ++k) {
        if ((a->vertices[0] == b->vertices[k] &&
             (a->vertices[1] == b->vertices[(k+1)%3] ||
              a->vertices[1] == b->vertices[(k+2)%3] ||
              a->vertices[2] == b->vertices[(k+1)%3] ||
              a->vertices[2] == b->vertices[(k+2)%3])) ||
            (a->vertices[1] == b->vertices[k] &&
             (a->vertices[0] == b->vertices[(k+1)%3] ||
              a->vertices[0] == b->vertices[(k+2)%3] ||
              a->vertices[2] == b->vertices[(k+1)%3] ||
              a->vertices[2] == b->vertices[(k+2)%3])) ||
            (a->vertices[2] == b->vertices[k] &&
             (a->vertices[0] == b->vertices[(k+1)%3] ||
              a->vertices[0] == b->vertices[(k+2)%3] ||
              a->vertices[1] == b->vertices[(k+1)%3] ||
              a->vertices[1] == b->vertices[(k+2)%3]))) {
                 return true;
             }
    }
    return false;
}

bool incident(const FacetPtr facet, const CellPtr cell) {
    assert(facet);
    return facet && (facet->cells[0] == cell || facet->cells[1] == cell);
}

bool adjacent(const PTrianglePtr a, const PTrianglePtr b) {
    if (!a || !b || a == b) {
        return false;
    }

    return (a->neighbors[0] == b || a->neighbors[1] == b || a->neighbors[2] == b) ||
           (b->neighbors[0] == a || b->neighbors[1] == a || b->neighbors[2] == a);
}

bool incident(const VertexPtr vertex, const FacetPtr facet) {
    return contains(vertex->facets(), facet);
}

void connect(TrianglePtr a, TrianglePtr b) {
    // check to see that triangles share adjacent vertices.
    assert(adjacent(a, b));

    #ifndef NDEBUG
    int conCnt = 0;
    int rootCnt = 0;
    #endif

    // hook up the partial triangles on the correct cell sides.
    for(uint i = 0; i < 2; ++i) {
        // don't connect root facing ptris.
        // in non-debug mode, we never hit the inner loop if
        // a[i] is the root cell.
        #ifdef NDEBUG
        if(a->cells[i]->isRoot()) continue;
        #endif

        for(uint j = 0; j < 2; ++j) {
            if(a->cells[i] == b->cells[j]) {
                #ifndef NDEBUG
                if(a->cells[i]->isRoot()) {
                    rootCnt++;
                    continue;
                }
                #endif
                connect(&a->partialTriangles[i], &b->partialTriangles[j]);
                #ifndef NDEBUG
                conCnt++;
                #endif
            }
        }
    }

    #ifndef NDEBUG
    assert(rootCnt > 0 || conCnt > 0);
    #endif
}

void connect(PTrianglePtr a, PTrianglePtr b) {
    assert(a->triangle != b->triangle && "partial triangles are on the same triangle");

    assert((!a->neighbors[0] || !a->neighbors[1] || !a->neighbors[2])
           && "connecting partial face without empty slots");
    assert((!b->neighbors[0] || !b->neighbors[1] || !b->neighbors[2])
           && "connecting partial face without empty slots");

    for(uint i = 0; i < 3; ++i) {
        assert(a->neighbors[i] != a && a->neighbors[i] != b);
        if(!a->neighbors[i]) {
            a->neighbors[i] = b;
            break;
        }
    }

    for(uint i = 0; i < 3; ++i) {
        assert(b->neighbors[i] != a && b->neighbors[i] != b);
        if(!b->neighbors[i]) {
            b->neighbors[i] = a;
            break;
        }
    }
}

void disconnect(PTrianglePtr a, PTrianglePtr b) {
    for(uint i = 0; i < 3; ++i) {
        if(a->neighbors[i] == b) {
            a->neighbors[i] = nullptr;
            break;
        }
    }

    for(uint i = 0; i < 3; ++i) {
        if(b->neighbors[i] == a) {
            b->neighbors[i] = nullptr;
            break;
        }
    }
}

bool incident(const TrianglePtr tri, const VertexPtr v) {
    assert(tri);
    return tri->vertices[0] == v || tri->vertices[1] == v || tri->vertices[2] == v;
}

/*
void disconnect(TrianglePtr tri, const Edge& edge) {
    assert(incident(tri, edge[0]));
    assert(incident(tri, edge[1]));
    if(!tri->cells[0]->isRoot()) { disconnect(&tri->partialTriangles[0], edge); }
    if(!tri->cells[1]->isRoot()) { disconnect(&tri->partialTriangles[1], edge); }
}

void disconnect(PTrianglePtr pt, const Edge& edge) {
    for(uint i = 0; i < 3; ++i) {
        if(pt->neighbors[i] &&
           pt->neighbors[i]->triangle &&
           incident(pt->neighbors[i]->triangle, edge[0]) &&
           incident(pt->neighbors[i]->triangle, edge[1])) {
            disconnect(pt, pt->neighbors[i]);
            return;
        }
    }
    assert(0 && "partial triangle is not adjacent to given edge");
}
*/

bool incident(const PTrianglePtr pt, const Edge& edge) {
    return incident(pt->triangle, edge);
}

bool incident(const TrianglePtr tri, const std::array<VertexPtr, 2>& edge) {
    return incident(tri, edge[0]) && incident(tri, edge[1]);
}


void reconnect(PTrianglePtr o, PTrianglePtr n, const std::array<VertexPtr, 2>& edge) {
    for(uint i = 0; i < 3; ++i) {
        if(o->neighbors[i] &&
           o->neighbors[i]->triangle &&
           incident(o->neighbors[i]->triangle, edge)) {
            PTrianglePtr adj = o->neighbors[i];
            disconnect(o, adj);
            connect(n, adj);
            return;
        }
    }
    assert(0 && "partial triangle is not adjacent to given edge");

}

bool incident(const PTrianglePtr tri, const VertexPtr v) {
    return incident(tri->triangle, v);
}

void disconnect(TrianglePtr tri, VertexPtr v) {
    if(!v) return;

    for(uint i = 0; i < 3; ++i) {
        if(tri->vertices[i] == v) {
            v->removeTriangle(tri);
            tri->vertices[i] = nullptr;
            return;
        }
    }

    assert(0 && "triangle did not match vertex");

    /*
    for(uint i = 0; i < 2; ++i) {
        for(uint j = 0; j < 3; ++j) {
            if(tri->partialTriangles[i].neighbors[j] &&
                    incident(tri->partialTriangles[i].neighbors[j], v)) {
                disconnect(&tri->partialTriangles[i], tri->partialTriangles[i].neighbors[j]);
            }
        }
    }
    */
}

void connect(TrianglePtr tri, VertexPtr v) {
    for(int i = 0; i < 3; ++i) {
        if(tri->vertices[i] == nullptr) {
            tri->vertices[i] = v;
            v->appendTriangle(tri);
            return;
        }
    }
    assert(0 && "triangle has no empty slot");
}
