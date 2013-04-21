#ifndef SOFTBODYMESH_H
#define SOFTBODYMESH_H

#include <NxPhysics.h>

#include <Ogre.h>

class SoftBodyMesh
{
public:
	SoftBodyMesh(Ogre::MeshPtr mesh, std::vector<std::vector<unsigned int>> *indexLinks);
	~SoftBodyMesh();

	struct MeshTetraLink
	{
		int tetraNr;
		NxVec3 barycentricCoords;
	};

	void loadTetFile(const Ogre::String &filename, NxArray<NxVec3>* vertices, NxArray<NxU32>* indices);
	NxVec3 computeBaryCoords(NxVec3 vertex, NxVec3 p0, NxVec3 p1, NxVec3 p2, NxVec3 p3);
	void buildTetraLinks(const NxVec3 *vertices, const NxU32 *indices, const NxU32 numTets);
	void allocateReceiveBuffers(NxU32 numVertices, NxU32 numTetrahedra);
	void removeTrisRelatedToVertex(const std::vector<unsigned int> &indexLinks);
	bool updateTetraLinks();

	std::vector<MeshTetraLink> mTetraLinks;
	std::vector<bool>          mDrainedTriVertices;
	NxBounds3                  mBounds;
	NxMeshData                 mReceiveBuffers;

private:
	inline NxVec3 Vector3toVec3(Ogre::Vector3 &vec)
	{
		return NxVec3(vec.x, vec.y, vec.z);
	}
	std::vector<std::vector<unsigned int>> *mIndexLinks;
	Ogre::MeshPtr mMesh;
};

#endif
