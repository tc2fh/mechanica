import mechanica as mx
import numpy as np
from csv import reader
import pandas as pd

mx.init()


'''
notes:
try not to fall into conditional hell, where every time we're trying to do things we fall into tons of conditionals
    mitigate divergence in logic as much as possible

'''







class Vertex(mx.ParticleType):
    radius = 0.001  # is there a limit to how small things can be?
    dynamics = mx.Overdamped
    mass = 1
    style = {"color": "MediumSeaGreen"}

    def __init__(self, x, y, z, Neighbors, EdgeList, SurfaceList):
        self.x = x
        self.y = y
        self.z = z
        self.Neighbors = Neighbors
        self.EdgeList = EdgeList
        self.SurfaceList = SurfaceList

    def update_neighbors(self, NeighborsList):
        for a in NeighborsList:
            self.Neighbors.append(a)

    def delete_neighbor(self, Neighbor):
        for a in Neighbor:
            self.Neighbor.remove(a)

    def find_cells(self):
    def get_id
    '''

    any edge defined by 2 vertices
    any surface defined by a set of edges, 3 or more
    a surface would have to have the edges as part of its internal data
    if each edge knows what its vertices are, the surface doesnt ned vertices as part of its internal data - call edge methods

    cell needs surface data
    surface needs edge data
    edge needs vertex data

    for every intermediate object, needs a list of information upstream and downstream

    edge needs vertex data and surface data, vertices that define it and list of surfaces that it contributes to
    apply to surfaces as well

    interactivity: create a mesh and vertices/edges/stuff all interconnected. if you have one we should be able to select the others

    what if in the iddle of a simulation, i want to modify the topology of one of hte cells.
        our interface should be able to support that
        should be able to dynamically add vertices to a surface, delete an edge and have the effects propagate



    '''


# consider vertex extending hte class Atype


class edges:
    pb = mx.Potential.coulomb(q=0.01, min=0.01, max=3)

    def __init__(self, Vertex1, Vertex2, potential=None):
        self.pb = potential  # initialized as none but can be set through the interface: Slot
        self.Vertex1 = Vertex1
        self.Vertex2 = Vertex
        if self.pb is not None:
            mx.bind(self.pb, self.Vertex1, self.Vertex2)

    def make_edge(self, pb, vertex1, vertex2):
        mx.bind(self.pb, self.Vertex1, self.Vertex2)
    def get_id:
        return [id]
    def get_vertices:
        return [self.Vertex1, self.Vertex2]
    def get_surfaces(self,edgeid):
        #search through all surfaces for list item of [self.vertex1, self.vertex2], return the vertex IDs
        return [id1, id2, id3...]
    def add_vertices(self, NewVertex,edge):
        # from nearest vertex (either self.vertex1 or self.vertex2), make a new edge between the new vertex and assign ID to it
    def add_surfaces(self,NewEdge):
        #from edge, create a surface inclusive of this edge id and the new edge id,
    def remove_surfaces:
        #should remove vertices/edges/surfaces be a global method? where that method checks if vertex/edge/surface and then does stuff to it?
    def remove_doubles(self,Threshold=5):###
        '''
        can this be a more global method?

        EDGES
        search through all edges, if the distance between vertex1 and vertex2 of any edge < threshold:
            1. delete id's of the edge in question and vertices in question
            2. for edges that it shares, make the vertices the center of the prior edge, and that one vertex is now going to be in place of the other vertices
            #THIS IS TO DELETE

        SURFACES
        search through all surfaces for surfaces with edges that have vertices of distance < threshold from each other
            1. delete id's of the surfaces, corresponding edges, and corresponding vertices in question
            2. for each edge id, merge the individual vertices at the average location of each vertex

        VERTEX
        search through all vertices for one with the same location as the vertex in question
            1. create new vertex ID at the average distance between the two vertices and replace both with this one.
            Should replace the proper IDs and propagate throughout all the edge/surface arrays


        Now this would mean that the user would have to call this method, either for every edge/vertex/surface or globally every so often,
        how would we go about doing this automatically?

        KEEP THIS AS PART OF A CLASS DEFINITION

        consider different functions/interfaces for user and engine. This seems like an engine method


        for edge/vertex/surface

        def check_quality(self,...)
            return flag

            flag will tell us whether a transformation needs to occur

            ask an edge, check_quality
            check_quality(edge)
            if flag--> remove_doubles(edge)

            if a tranformation occurs --> check_quality
            whatever returns from check_quality --> do other methods

            call check_quality every timestep, every time a user deletes something, every time a user does something

        '''



class surface:
    def __init__(self, VertexList, EdgeList,Cells):
        # self.VertexList = VertexList
        self.EdgeList = EdgeList
        self.cell_list = Cells

    def get_id(self):
        return [id]
    def get_edges(self):
        return [edgeid, edgeid, edgeid, edgeid...]
    def get_cells(self):
        #returns cell ids that this surface is a part of, search all cell objects for this surface's ID
        return [cellid]
    def add_edges(self,edgeid):
        #add edge id to surface
    def remove_cells(self):
    def remove_edges(self):
    def remove_doubles(self):



class cell:
    def __init__(self,  surfacelist):
        # self.vertices = vertices
        # self.edges = edges
        self.surface = surfacelist

    def get_id:
    def get_surface:
        return surfaces
    def recalculate:


class AType(mx.ParticleType):
    radius = 0.05
    dynamics = mx.Overdamped
    mass = 1
    style = {"color": "MediumSeaGreen"}


A = AType.get()

a = [A(p) for p in result]

# mx.MxVector3f

mx.show()
"""
create preferred volume

force on all particles/vertices to maintain preferred volume

create cell class definition, automatically creates/ipmports nodes to create the cell

see how blender structures the information and recreate it in mechanica

class definition that does it like blender, turns that information into an object with corresponding particles

think about visualizing surfaces, eventually allow it to be a barrier with permeability math in it
"""