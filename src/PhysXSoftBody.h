#ifndef _PHYSX_SOFTBODY_H_
#define _PHYSX_SOFTBODY_H_

#include "ObjMeshExt.h"

class PhysXSoftBody
{
public:
	PhysXSoftBody(NxScene* scene, Ogre::SceneManager* sceneMgr,
	              NxSoftBodyDesc &desc, ObjMeshExt* objMesh);
	~PhysXSoftBody();

	NxSoftBody* getNxSoftBody() {return mSoftBody;}
	Ogre::SceneNode* getSceneNode() {return mSoftBodyNode;}
	void render();

private:
	bool saveMeshDesc(NxSoftBodyMeshDesc &desc);
	bool loadTetFile(NxArray<NxVec3>* vertices, NxArray<NxU32>* tetrahedras);
	void init(NxSoftBodyDesc &desc, NxSoftBodyMeshDesc &meshDesc);
	bool cookMesh(NxSoftBodyMeshDesc& desc);
	void releaseMeshDescBuffers(const NxSoftBodyMeshDesc& desc);
	void allocateReceiveBuffers(NxU32 numVertices, NxU32 numTetrahedra);
	void releaseReceiveBuffers();
	void initEntity();

	bool             mInitDone;
	std::vector<int> mVertexOffsets;

	ObjMeshExt*     mObjMesh;

	NxScene*        mScene;
	NxSoftBody*     mSoftBody;
	NxSoftBodyMesh* mSoftBodyMesh;
	NxMeshData      mReceiveBuffers;

	Ogre::SceneManager* mSceneMgr;
	Ogre::SceneNode*    mSoftBodyNode;
	Ogre::Entity*       mEntity;
};

#endif
