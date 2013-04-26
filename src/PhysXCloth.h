#ifndef _PHYSX_CLOTH_H_
#define _PHYSX_CLOTH_H_

#include "ObjMeshExt.h"

class PhysXCloth
{
public:
	PhysXCloth(NxScene* scene, Ogre::SceneManager* sceneMgr, NxClothDesc &desc,
	           ObjMeshExt* objMesh, int index);
	~PhysXCloth();

	NxCloth* getNxCloth() {return mCloth;}
	void render();

private:
	bool saveMeshDesc(NxClothMeshDesc &desc, ObjMeshExt* objMesh);
	void init(NxClothDesc &desc, NxClothMeshDesc &meshDesc);
	void allocateReceiveBuffers(NxU32 numVertices, NxU32 numTriangles);
	bool cookMesh(NxClothMeshDesc &desc);
	void releaseMeshDescBuffers(const NxClothMeshDesc& desc);
	void releaseReceiveBuffers();
	void initOgreScene();

	bool   mInitDone;

	NxScene*     mScene;
	NxCloth*     mCloth;
	NxClothMesh* mClothMesh;
	NxMeshData   mReceiveBuffers;

	Ogre::SceneManager* mSceneMgr;
	Ogre::SceneNode*    mClothNode;
	Ogre::ManualObject* mManualObj;
	Ogre::String        mName;

	std::vector<NxVec3> mPositions;
	std::vector<NxVec3> mNormals;
	std::vector<NxU32>  mIndices;

	NxU32 mNumVertices;
	NxU32 mNumIndices;
	NxU32 mMeshDirtyFlags;
	NxU32 mNumTriangles;
};

#endif
