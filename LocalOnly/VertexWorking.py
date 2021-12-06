import mechanica as mx
import numpy as np
from csv import reader
import pandas as pd
import uuid
import os

"""
ToDiscuss:

-Multiple Structures
    currently we have one slot for the ID of the structure to instantiate a vertex/edge/surface so that we can search locally.

    If we have multiple structures, maybe self.Structure attribute would be a list of IDs? I struggle to foresee cases where one object as a part of multiple structures would 
    really be necessary but I want to build the software with this functionality as a distant possibility in case that use case arrises.

"""

'''
Input = 3 csv files

Vertex_CSV = 3 x n spreadsheet where columns correspond to x, y, z. row corresponds to the individual vertex
Edge_CSV = 1 x n spreadsheet where elements equals [[x1,y1,z1], [x2,y2,z2]] for vertex 1 and 2 respectively
Surface_CSV = 2 or 3 x n spreadsheet where column elements are edges like in Edge_CSV and rows are the individual surfaces



Not sure what cells/tissues inputs would look like...
    Many Cells: a folder per cell where each cell folder has the above three CSV files in them?
    existing literature vertex models would only have one "cell" to represent the whole tissue 
        because all the focus is on the physics of the vertices only

    Maybe at this moment, the Cell should be an object holding many dictionaries?
    should there be a tissue option in which case the tissue is the one holding the dictionaries?

    Organizational level ID's should be numbers because they wont be dynamically creating and deleting




'''

GlobalVertex = {}
GlobalEdge = {}
GlobalSurface = {}
GlobalStructure = {}

# Initialize:
CSV_File_Path = ''
# Initialization order:
VertexInit = {}
EdgeInit = {}
SurfaceInit = {}
StructureInit = {}

'''
for each cell,
instantiate all vertices first, append to the VertexInit dictionary
instantintiate all edges, append to the EdgeInit dictionary
instantiate all surfaces, append to the SurfaceInit dictionary
Instantiate Structure, pass in all of these Init dictionaries

clear the dictionaries: dict.clear()
'''


def init_Vertices(Path):
    # creates vertex objects from vertex, adds ID and obj to VertexInit dict, appends that dict to GlobalVertex
    CSV_Path = Path + '\\Vertex_CSV.csv'
    df = pd.read_csv(CSV_Path)
    VertexList = list(df.to_records(index=False))
    [make_vertex(x, len(StructureInit), str='Init') for x in
     VertexList]  # len(StructureInit) shows how many structures in the sim, if no structures, default = 0
    GlobalVertex.update(VertexInit)
    return


def init_Edges(Path, pb=None):  ###HOW DO WE PRE-ASSIGN POTENTIALS? WHEN DO WE ASK THIS FROM THE USER? DO WE RE-ASSIGN?
    # creates edge objects from edge, adds ID and obj to EdgeInit dict, appends that dict to GlobalEdge
    CSV_Path = Path + '\\Edge_CSV.csv'
    df = pd.read_csv(CSV_Path)
    EdgeList = list(df.to_records(index=False))
    # this is really slow and sloppy but it only has to run once for initialization
    for x in EdgeList:
        Vert1 = x[0]
        Vert2 = x[1]
        for key, value in VertexInit.items():
            if [value.x, value.y, value.z] == Vert1:
                Vert1_id = key
            if [value.x, value.y, value.z] == Vert2:
                Vert2_id = key
        if Vert1_id and Vert2_id:
            make_edge(Vert1_id, Vert2_id, len(StructureInit), pb, str='Init')
        else:
            print('Error initializing Edges:')
            if Vert1_id:
                print(Vert1, ' Vertex exists in VertexInit Dictionary')
            else:
                print(Vert1, ' Vertex does not exist in VertexInit Dictionary')
            if Vert2_id:
                print(Vert2, ' Vertex exists in VertexInit Dictionary')
            else:
                print(Vert2, ' Vertex does not exist in VertexInit Dictionary')
    GlobalEdge.update(EdgeInit)
    return


def init_Surfaces(Path):  # THIS ONLY ACCOUNTS FOR CSV FILE OF 2 EDGES = 1 SURFACE, HOW TO MAKE THIS DYNAMIC?
    CSV_Path = Path + '\\Surface_CSV.csv'
    df = pd.read_csv(CSV_Path)
    SurfaceList = list(df.to_records(index=False))
    for x in SurfaceList:
        Edge1 = [x[0][0], x[0][1]]
        Edge2 = [x[1][0], x[1][1]]
        for key, value in EdgeInit.items():
            Vert1_id = value.find_vertices()[0]
            Vert2_id = value.find_vertices()[1]
            findedge = [[VertexInit[Vert1_id].x, VertexInit[Vert1_id].y, VertexInit[Vert1_id].z],
                        [VertexInit[Vert2_id].x, VertexInit[Vert2_id].y, VertexInit[Vert2_id].z]]
            if findedge == Edge1:
                Edge1_id = key
            if findedge == Edge2:
                Edge2_id = key
        if Edge1_id and Edge2_id:
            make_surface([[Edge1_id, Edge2_id]], len(StructureInit), str='Init')
        else:
            print('Error initializing Surfaces')
            if Edge1_id:
                print(Edge1, ' Edge exists in EdgeInit Dictionary')
            else:
                print(Edge1, ' Edge does not exist in EdgeInit Dictionary')
            if Edge2_id:
                print(Edge2, ' Edge exists in EdgeInit Dictionary')
            else:
                print(Edge2, ' Edge does not exist in EdgeInit Dictionary')
    GlobalEdge.update(EdgeInit)
    return


def init_Structures(Path):  # Figure out how this is going to work

    # So either one structure per OBJ file, or if multiple structures in OBJ file, we would need to figure out how to separate out which surfaces/edges/verts belong to which structure
    # Once this is done, we can go back and rework this Init_Structures function
    CSV_Path = Path + '\\'
    GlobalStructure.update(StructureInit)
    return


# MAKE THINGS
def make_vertex([x, y, z]

    , StructureID, str = 'Global'):
# assign uuid
ID = str(uuid.uuid4())
if str='Global':
    GlobalVertex[ID] = Vertex([x, y, z], ID)
elif str='Init':
    VertexInit[ID] = Vertex([x, y, z], ID)
return ID


def make_edge(Vertex1, Vertex2, StrucutreID, pb=None, str='Global'):
    # create edge by passing in two unique vertex id's, assign a new edge id and return it.
    # By default appends to global edge dictionary, however 'Init' option exists for initialization phase
    mx.bind(pb, Vertex1, Vertex2)
    ID = str(uuid.uuid4())
    if str='Global':
        GlobalEdge[ID] = Edge(Vertex1, Vertex2, ID, pb)
    elif str='Init':
        EdgeInit[ID] = Edge(Vertex1, Vertex2, ID, pb)
    return


def make_surface(EdgeList, StrucutreID, str='Global')
    # call remove_doubles()
    # If something happened (triangle), create new edge along unshared vertices
    # *** How to choose new edge if square?
    ID = str(uuid.uuid4())
    if str='Global':
        GlobalSurface[ID] = Surface(EdgeList, ID)
    elif str='Init':
        SurfaceInit[ID] = Surface(EdgeList, ID)
    return


def make_structure(SurfaceDict, EdgeDict, VertDict, Volume, GroupID=0, str='Global')
    ID = len(GlobalStructure)
    if str='Global':
        GlobalStructure[ID] = Structure(SurfaceDict, EdgeDict, VertDict, Volume, GroupID=0)
    if str='Init':
        SurfaceInit[ID] = structure


# def make_tissue(str='Global')


# remove doubles currently incomplete
# every timestep, there should be a flag (engine side) that indicates whether things are sitting on top of each other and remove_doubles needs to be called.
# Where will this flag exist?
def remove_doubles(object, threshold=5):  # default threshold? option in settings?
    if object.type == 'cell':
    # if any sets of vertices are within the threshold, delete them as below
    elif object.type == 'surface':
    # do the thing
    # if any sets of vertices are within the threshold, delete them as below
    elif object.type == 'edge'


# if vertex 1 and 2 are within threshold, delete vertex 1 and 2 from global and all attached structures, replace with new vertex3 and id


class Vertex(mx.ParticleType):
    radius = 0.001  # is there a limit to how small things can be?
    dynamics = mx.Overdamped
    mass = 1
    style = {"color": "MediumSeaGreen"}

    def __init__(self, [x, y, z]

        , id, StructureID = 0):
    self.x = x
    self.y = y
    self.z = z
    self.id = id
    self.type = 'vertex'
    self.Structure = StructureID


def find_edges(self):
    # returns list of edge id's that this vertex is a part of
    edgelist = [key for key, val in GlobalStructure[self.Structure].edges if self.id in val.find_vertices()]

    # edgelist = [key for key,val in GlobalEdge.items() if self.id in val.find_vertices()]
    # this works because GlobalEdge is a dictionary of edges, where every edge id can return a list of vertex id's

    # edgelist = [x.id for x in EdgeList.GLobal if self.id in x.find_vertices()] ### ADD FIND VERTICES METHOD TO EDGES
    return edgelist


def find_surfaces(self):
    # returns all surfaces that this vertex is a part of
    surfacelist = []
    for edge in self.find_edges():
        surfacelist.append(edge.find_surfaces())

        # for x in SurfaceList.Global:
        #     for y in x.find_edges(): ###REMEMBER TO ADD FIND_EDGES METHOD TO SURFACE
        #         if self.id in y.find_vertices and x not in surfacelist:
        #             surfacelist.append(x)

        return surfacelist


def find_Structures(self):
    # Because structures are organizational, this function would only return self.Structure
    # How to handle multiple structures? Do we append multiple structures to StructureID?

    if len(self.Structure) > 1:
        return self.Structure
    else:
        print("This vertex belongs to only one structure, access the Structure ID with self.Structure")
        return self.Structure


def neighbors(self):
    # returns list of neighbor vertice id's
    neighborlist = [x.find_vertices() for x in self.find_edges()]
    flat_neighborlist = [item for sublist in neighborlist for item in sublist]

    # remove duplicates:
    NeighborList = []
    [NeighborList.append(x) for x in flat_neighborlist if x not in flat_neighborlist]
    return NeighborList


'''
    flat_list = []
    for sublist in t:
        for item in sublist:
            flat_list.append(item)
    flat_list = [item for sublist in t for item in sublist]
    return neighborlist
'''


class Edges:
    pb = mx.Potential.coulomb(q=0.01, min=0.01, max=3)

    def __init__(self, Vertex1, Vertex2, id, potential=None, StructureID=0):
        self.pb = potential  # initialized as none but can be set through the interface: Slot
        self.Vertex1 = Vertex1.id
        self.Vertex2 = Vertex.id
        self.type = 'edge'
        self.id = id
        self.Structure = StructureID
        if self.pb is not None:
            mx.bind(self.pb, self.Vertex1, self.Vertex2)

    def find_structures:
        # assuming self.structure is a list of structure IDs. Returns self.Structure if more than 1, sends a nice message along with that if < 1
        if len(self.Structure) > 1:
            return self.Structure
        else:
            print("This vertex belongs to only one structure, access the Structure ID with self.Structure")
            return self.Structure

    def find_vertices:
        return [self.Vertex1, self.Vertex2]

    def find_surfaces(self, edgeid):
        # search through all surfaces(locally) for list item of [self.vertex1, self.vertex2], return the vertex IDs
        surfacelist = [key for key, val in GlobalStructure[self.Structure].surfaces if self.id in val.find_edges()]
        return SurfaceList


class Surface:
    def __init__(self, EdgeList, id, StructureID=0):
        self.EdgeList = EdgeList
        self.type = 'surface'
        self.id = id

    def find_edges(self):
        return self.EdgeList

    def find_structures(self):
        # returns cell ids that this surface is a part of, search all cell objects for this surface's ID
        if len(self.Structure) > 1:
            return self.Structure
        else:
            print("This vertex belongs to only one structure, access the Structure ID with self.Structure")
            return self.Structure

    def find_vertices(self):
        vertlist = [edge.find_vertices for edge in self.EdgeList]
        # flatten because vertlist is a list of vertice pairs from the edges that make up the surface
        flat_vertlist = [item for sublist in vertlist for item in sublist]
        # remove duplicate items
        VertList = []
        [VertList.append(x) for x in flat_vertlist if x not in VertList]
        return VertList


# structure is organizational for searching locally
class Structure:
    def __init__(self, SurfaceDict, EdgeDict, VertDict, Volume, GroupID=0):
        # self.vertices = vertices
        # self.edges = edges
        self.surfaces = SurfaceDict
        self.edges = EdgeDict
        self.vertices = VertDict
        self.type = 'structure'
        self.volume = Volume
        # should volume be optional? We had envisioned one structure's volume constraint would exert a force on all vertices. Not sure how this plays along
        # with volume/surface area/distance constraints at the surface and/or edge level

        # discuss constraints


# GROUP IS A LARGER ORGANIZATIONAL STRUCTURE TO BE ADDED LATER AFTER MORE VERTEX SIMULATION FUNCTIONALITY IS ADDED
# An example of when this might be useful is if we have multiple groups of structures where group 1 has its own set of physics applied and group 2 has a separate
# set of physics applied and we might not want them to mix. Not sure if this is entirely useful
class Group:
    def __init__(self, CellDict, SurfaceDict, EdgeDict, VertDict):
        # self.vertices = vertices
        # self.edges = edges
        self.surface = surfacelist
        self.type = 'cell'


# NOTES DOWN HERE

# create functionality with cell first, and then extend it with tissue.

'''
More Notes on adding verts/surfaces/edges to a simulation without pointing to an OBJ

    # def add_vertices(self, pb, NewVertex, OldVertex):
    #     #add edge by specifying new vertex id and old vertex to connect to?
    #     # probably wouldnt do this?
    #     id = make_edge(pb=self.pb, NewVertex, OldVertex):
    #     return id

    # from nearest vertex (either self.vertex1 or self.vertex2), make a new edge between the new vertex and assign ID to it
    # def add_surfaces(self, NewEdge, OldEdge):
    #     #make edge
    #     #run remove_doubles
    #     #if remove doubles worked, only need to create 1 edge (triangle surface)
    #     #if remove doubles didnt work, need to create 2 edges (square surface)


add_surfaces in 2 steps, forming 2 triangles then calling a join surface method to join them into 1 square
consider adding target volumes and target lengths as an attribute
google mesh topology

'''