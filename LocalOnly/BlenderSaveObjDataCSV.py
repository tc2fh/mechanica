import os
import bpy
import bmesh
import numpy as np




'''

if a vertex is part of more than one edge, 
get every surface that the edge is a part of, 
and then return all the vertices


to do 9/28: 
    assign list of ID's for each vertex
    output edges as pair of vertex IDs
    in mechanica: store the ID of a particle in the dictionary for that handle
        use that to create convenient links
    think about how to render surfaces

write class definition for a vertex model cell

class definition for a surface, 
class definition for an edge, 
class definition for a vertex

class definitions should have information about underlying mechnica components and information about the mesh

if I have a surface, should have reference to all vertex that it makes

every vertex should have a reference to nearby vertices/edges, 
    so that from one face, we can go to other surfaces adjacent to it
    
'''

def get_co(ob):
    v_count = len(ob.data.vertices)
    co = np.zeros(v_count*3)
    ob.data.vertices.foreach_get('co', co)
    co.shape = (v_count, 3)
    #zero = np.zeros((v_count,1))
    #final = np.hstack((co,zero))
    #final = final * 100
    return co

def get_edge(ob):
    #ob = bpy.context.active_object
    me = ob.data
    bm = bmesh.from_edit_mesh(me)
    edges = []
    edge_list = [tuple(list(v.co for v in e.verts)) for e in bm.edges]
    for verts in edge_list:
        edges.append([list(verts[0]),list(verts[1])])
    npEdges = np.asarray(edges)
    #Reformat Array
    XEdges = []
    YEdges = []
    for i in range(len(npEdges)):
        np.append(XEdges,npEdges[i][0])
        np.append(YEdges,npEdges[i][1])
    #XEdges.reshape(len(XEdges)*6,1)
    #YEdges.reshape(len(YEdges)*6,1)
    #FinalEdges = np.hstack(XEdges,YEdges)
    
    print("X/YEdges = ")
    print(XEdges)
    print(YEdges)
    print("help")
    #print(npEdges)
    print('haha')
    print(npEdges)
    #print(npEdges[1][0])
    #print(len(npEdges))
    #for edge in ob.data.edges:
    #    edges.append([edge.vertices[0], edge.vertices[1]])
    #print(edges)
    return

ob = bpy.context.object

get_edge(ob)

final = get_co(ob)
print(final)
print(os.getcwd())
os.chdir('C:\\Users\\TienAdmin\\Documents\\BlenderSave')
np.savetxt('MechanicaCellsMonkey.csv',final,delimiter=',')
