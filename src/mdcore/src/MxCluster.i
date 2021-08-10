%{
    #include "MxCluster.hpp"

%}

// Currently swig isn't playing nicely with MxVector3f pointers for ::operator(), 
//  so we'll handle them manually for now
%rename(_call) MxClusterParticleHandle::operator();
%rename(_call) MxClusterParticleType::operator();

%include "MxCluster.hpp"

%template(list_ParticleType) std::list<MxParticleType*>;

%extend MxClusterParticleHandle {
    %pythoncode %{
        def __call__(self, particle_type, position=None, velocity=None):
            pos, vel = None, None
            if position is not None:
                pos = MxVector3f(list(position))

            if velocity is not None:
                vel = MxVector3f(list(velocity))

            return self._call(particle_type, pos, vel)

        @property
        def radius_of_gyration(self):
            return self.getRadiusOfGyration()

        @property
        def center_of_mass(self):
            return self.getCenterOfMass()

        @property
        def centroid(self):
            return self.getCentroid()

        @property
        def moment_of_inertia(self):
            return self.getMomentOfInertia()
    %}
}

%extend MxClusterParticleType {
    %pythoncode %{
        def __call__(self, position=None, velocity=None, cluster_id=None):
            ph = MxParticleType.__call__(self, position, velocity, cluster_id)
            return MxClusterParticleHandle(ph.id, ph.type_id)
    %}
}

%pythoncode %{
    Cluster = MxCluster
    ClusterHandle = MxClusterParticleHandle
%}

// In python, we'll specify cluster types using class attributes of the same name 
//  as underlying C++ struct. ClusterType is the helper class through which this 
//  functionality occurs. ClusterType is defined in particle_type.py