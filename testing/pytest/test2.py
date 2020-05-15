import ctypes
import sys
import os.path as path


cur_dir = path.dirname(path.realpath(__file__))

print(cur_dir)

print(sys.path)

sys.path.append("/Users/andy/src/mechanica/src")
sys.path.append("/Users/andy/src/mx-xcode/Debug/lib")
#sys.path.append("/Users/andy/src/mx-eclipse/src")

print(sys.path)



import mechanica as m
import _mechanica

print ("mechanica file: " + m.__file__, flush=True)
print("_mechanica file: " + _mechanica.__file__, flush=True)

print(dir(m))

s = m.Simulator()

print(dir(s))



# import the library
import mechanica as m
import numpy as n

# make a new simulatorm default values for everythign 
m.Simulator()

# create a particle type
# all new Particle derived types are automatically
# registered with the universe
class Argon(m.Particle):
    mass = 39.4
    
# create a potential representing a 12-6 Lennard-Jones potential
# A The first parameter of the Lennard-Jones potential.
# B The second parameter of the Lennard-Jones potential.
# cutoff 
pot = m.Potential.LennardJones126(9.5075e-06 , 6.1545e-03, cutoff=1)

# bind the potential with the *TYPES* of the particles
m.Universe.bind(pot, Argon, Argon)

# uniform random cube    
positions = n.random.uniform(low=0, high=1, size=(100, 3))

for pos in positions:
    # calling the particle constructor implicitly adds 
    # the particle to the universe
    Argon(pos)
    
# run the simulator interactivy
m.Simulator.irun()
    
    
