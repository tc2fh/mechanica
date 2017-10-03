/*
 * MxCell.cpp
 *
 *  Created on: Jul 7, 2017
 *      Author: andy
 */

#include <MxCell.h>
#include <MxMesh.h>
#include <iostream>
#include <algorithm>
#include <array>

#include "MxDebug.h"

bool operator == (const std::array<MxVertex *, 3>& a, const std::array<MxVertex *, 3>& b) {
  return a[0] == b[0] && a[1] == b[1] && a[2] == b[2];
}



static void connectPartialTriangles(MxPartialTriangle &pf1, MxPartialTriangle &pf2) {
    assert(pf1.triangle != pf2.triangle && "partial triangles are on the same triangle");

    assert((!pf1.neighbors[0] || !pf1.neighbors[1] || !pf1.neighbors[2])
           && "connecting partial face without empty slots");
    assert((!pf2.neighbors[0] || !pf2.neighbors[1] || !pf2.neighbors[2])
           && "connecting partial face without empty slots");

    for(uint i = 0; i < 3; ++i) {
        assert(pf1.neighbors[i] != &pf1 && pf1.neighbors[i] != &pf2);
        if(!pf1.neighbors[i]) {
            pf1.neighbors[i] = &pf2;
            break;
        }
    }

    for(uint i = 0; i < 3; ++i) {
        assert(pf2.neighbors[i] != &pf1 && pf2.neighbors[i] != &pf2);
        if(!pf2.neighbors[i]) {
            pf2.neighbors[i] = &pf1;
            break;
        }
    }
}

bool MxCell::connectBoundary() {
    // clear all of the boundaries before we connect them.
    for(MxPartialTriangle *pt : boundary) {
        pt->neighbors[0] = nullptr;
        pt->neighbors[1] = nullptr;
        pt->neighbors[2] = nullptr;
    }

    for (uint i = 0; i < boundary.size(); ++i) {
        MxPartialTriangle &pti = *boundary[i];
        MxTriangle &ti = *pti.triangle;

        for(uint j = i+1; j < boundary.size(); ++j) {
            MxPartialTriangle &ptj = *boundary[j];
            MxTriangle &tj = *ptj.triangle;


            for(int k = 0; k < 3; ++k) {
                if ((ti.vertices[0] == tj.vertices[k] &&
                        (ti.vertices[1] == tj.vertices[(k+1)%3] ||
                         ti.vertices[1] == tj.vertices[(k+2)%3] ||
                         ti.vertices[2] == tj.vertices[(k+1)%3] ||
                         ti.vertices[2] == tj.vertices[(k+2)%3])) ||
                    (ti.vertices[1] == tj.vertices[k] &&
                        (ti.vertices[0] == tj.vertices[(k+1)%3] ||
                         ti.vertices[0] == tj.vertices[(k+2)%3] ||
                         ti.vertices[2] == tj.vertices[(k+1)%3] ||
                         ti.vertices[2] == tj.vertices[(k+2)%3])) ||
                    (ti.vertices[2] == tj.vertices[k] &&
                        (ti.vertices[0] == tj.vertices[(k+1)%3] ||
                         ti.vertices[0] == tj.vertices[(k+2)%3] ||
                         ti.vertices[1] == tj.vertices[(k+1)%3] ||
                         ti.vertices[1] == tj.vertices[(k+2)%3]))) {


                            /*
                            std::cout << "k: " << k
                                      << ", (k+1)%3 : " << (k+1)%3
                                      << ", (k+2)%3 : " << (k+2)%3
                                      << ", (k+1)%3 : " << (k+1)%3
                                      << ", (k+2)%3 : " << (k+2)%3 << std::endl;

                            std::cout << "face 1" << std::endl;
                            std::cout << "pf[" << i << "] " << ti.vertices << " {" << std::endl;
                            std::cout << mesh->vertices[ti.vertices[0]].position << std::endl;
                            std::cout << mesh->vertices[ti.vertices[1]].position << std::endl;
                            std::cout << mesh->vertices[ti.vertices[2]].position << std::endl;
                            std::cout << "}" << std::endl;

                            std::cout << "face 2" << std::endl;
                            std::cout << "pf[" << j << "] " << tj.vertices << " {" << std::endl;
                            std::cout << mesh->vertices[tj.vertices[0]].position << std::endl;
                            std::cout << mesh->vertices[tj.vertices[1]].position << std::endl;
                            std::cout << mesh->vertices[tj.vertices[2]].position << std::endl;
                            std::cout << "}" << std::endl;
                             */

                            connectPartialTriangles(pti, ptj);
                    break;
                }
            }
        }

        // make sure face is completely connected after searching the rest of the
        // set of faces, if not, that means that our surface is not closed.
        if(!pti.neighbors[0] || !pti.neighbors[1] || !pti.neighbors[2]) {
            std::cout << "error, surface mesh for cell is not closed" << std::endl;
            return false;
        }
    }

    return true;
}

float MxCell::volume(VolumeMethod vm) {
}

struct Foo : MxObject {


    /**
     * indices of the three neighboring partial triangles.
     */
    std::array<PTrianglePtr, 3> neighbors;



};


float MxCell::area() {




}

void MxCell::vertexAtributeData(const std::vector<MxVertexAttribute>& attributes,
        uint vertexCount, uint stride, void* buffer) {
    uchar *ptr = (uchar*)buffer;
    for(uint i = 0; i < boundary.size() && ptr < vertexCount * stride + (uchar*)buffer; ++i, ptr += 3 * stride) {
        //for(auto& attr : attributes) {

        const MxTriangle &face = *(boundary[i])->triangle;
        Vector3 *ppos = (Vector3*)ptr;
        ppos[0] = face.vertices[0]->position;
        ppos[1] = face.vertices[1]->position;
        ppos[2] = face.vertices[2]->position;
    }
}

//void MxCell::indexData(uint indexCount, uint* buffer) {
//    for(int i = 0; i < boundary.size(); ++i, buffer += 3) {
//        const MxTriangle &face = *boundary[i]->triangle;
//        buffer[0] = face.vertices[0];
//        buffer[1] = face.vertices[1];
//        buffer[2] = face.vertices[2];
//    }
//}





void MxCell::dump() {

    for (uint i = 0; i < boundary.size(); ++i) {
        const MxTriangle &ti = *boundary[i]->triangle;

        std::cout << "face[" << i << "] {" << std::endl;
        //std::cout << "vertices:" << ti.vertices << std::endl;
        std::cout << "}" << std::endl;
    }

}

/* POV mesh format:
mesh2 {
vertex_vectors {
22
,<-1.52898,-1.23515,-0.497254>
,<-2.41157,-0.870689,0.048214>
,<-3.10255,-0.606757,-0.378837>
     ...
,<-2.22371,-1.5823,-2.07175>
,<-2.41157,-1.30442,-2.18785>
}
face_indices {
40
,<1,4,17>
,<1,17,18>
,<1,18,2>
  ...
,<8,21,20>
,<8,20,9>
}
inside_vector <0,0,1>
}
 */

void MxCell::writePOV(std::ostream& out) {
    out << "mesh2 {" << std::endl;
    out << "face_indices {" << std::endl;
    out << boundary.size()  << std::endl;
    for (int i = 0; i < boundary.size(); ++i) {
        const MxTriangle &face = *boundary[i]->triangle;
        //out << face.vertices << std::endl;
        //auto &pf = boundary[i];
        //for (int j = 0; j < 3; ++j) {
            //Vector3& vert = mesh->vertices[pf.vertices[j]].position;
        //        out << ",<" << vert[0] << "," << vert[1] << "," << vert[2] << ">" << std::endl;
        //}
    }
    out << "}" << std::endl;
    out << "}" << std::endl;
}

HRESULT MxCell::appendChild(TrianglePtr tri) {
    CellPtr *cell = nullptr;

    if(tri->cells[0] == this || tri->cells[1] == this) {
        return mx_error(E_FAIL, "triangle is already attached to this cell");
    }

    int index = tri->cells[0] == nullptr ? 0 : tri->cells[1] == nullptr ? 1 : -1;

    if (index < 0) {
        return mx_error(E_FAIL, "triangle is already attached to two cells");
    }

    if (tri->partialTriangles[index].neighbors[0] ||
        tri->partialTriangles[index].neighbors[1] ||
        tri->partialTriangles[index].neighbors[2]) {
        return mx_error(E_FAIL, "triangle partial triangles already connected");
    }

    tri->cells[index] = this;

    // index of other cell.
    int otherIndex = index == 0 ? 1 : 0;

    // if the triangle is already attached to another cell, we look for a corresponding
    // facet, if no facet, that means that this is a new fact between this cell, and
    // the other cell the triangle is attached to.
    if (tri->cells[0] || tri->cells[1]) {
        // look first to see if we're connected by a facet.
        FacetPtr facet = nullptr;
        for(auto f : facets) {
            if (incident(f, tri->cells[otherIndex])) {
                facet = f;
                break;
            }
        }

        // not connected, make a new facet that connects this cell and the other
        if (facet == nullptr) {
            facet = mesh->createFacet(nullptr, this, tri->cells[otherIndex]);
            facets.push_back(facet);
            if (tri->cells[otherIndex]) {
                tri->cells[otherIndex]->facets.push_back(facet);
            }
        }

        // add the tri to the facet
        facet->triangles.push_back(tri);
    }

    // scan through the list of partial triangles, and connect whichever ones share
    // an edge with the given triangle.
    for(MxPartialTriangle *pt : boundary) {
        TrianglePtr t = pt->triangle;

        for(int k = 0; k < 3; ++k) {
            if ((tri->vertices[0] == t->vertices[k] &&
                    (tri->vertices[1] == t->vertices[(k+1)%3] ||
                     tri->vertices[1] == t->vertices[(k+2)%3] ||
                     tri->vertices[2] == t->vertices[(k+1)%3] ||
                     tri->vertices[2] == t->vertices[(k+2)%3])) ||
                (tri->vertices[1] == t->vertices[k] &&
                    (tri->vertices[0] == t->vertices[(k+1)%3] ||
                     tri->vertices[0] == t->vertices[(k+2)%3] ||
                     tri->vertices[2] == t->vertices[(k+1)%3] ||
                     tri->vertices[2] == t->vertices[(k+2)%3])) ||
                (tri->vertices[2] == t->vertices[k] &&
                    (tri->vertices[0] == t->vertices[(k+1)%3] ||
                     tri->vertices[0] == t->vertices[(k+2)%3] ||
                     tri->vertices[1] == t->vertices[(k+1)%3] ||
                     tri->vertices[1] == t->vertices[(k+2)%3]))) {

                connectPartialTriangles(*pt, tri->partialTriangles[index]);
                break;
            }
        }
    }
    
    boundary.push_back(&tri->partialTriangles[index]);

    return S_OK;
}
