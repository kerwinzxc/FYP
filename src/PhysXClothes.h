#ifndef _PHYSX_CLOTHES_H_
#define _PHYSX_CLOTHES_H_

#include "Common.h"

class PhysXClothes
{
public:
	PhysXClothes(NxScene* scene, Ogre::SceneManager* sceneMgr, NxClothDesc &desc, char* objFilePath,
	             NxVec3 initPosition, NxU32 numOfClothes);
	~PhysXClothes();

	void render();

private:
	bool saveMeshDesc(char* objFilePath);
	void init();
	void allocateReceiveBuffers(NxU32 numVertices, NxU32 numTriangles, NxU32 offset);
	bool cookMesh(NxU32 offset);
	void createCloth(NxU32 offset);
	void releaseMeshDescBuffers();
	void releaseReceiveBuffers();
	void initOgreScene();

	bool   mInitDone;
	NxVec3 mInitPosition;
	NxU32  mNumOfClothes;

	NxScene*                  mScene;
	NxClothDesc               mDesc;
	NxClothMeshDesc           mMeshDesc;
	std::vector<NxCloth*>     mClothes;
	std::vector<NxClothMesh*> mClothMeshes;
	NxMeshData*               mReceiveBuffers;

	Ogre::SceneManager* mSceneMgr;
	Ogre::SceneNode*    mClothesNode;
	Ogre::ManualObject* mManualObj;
	Ogre::String        mName;

	std::vector<NxVec3*> mPositions;
	std::vector<NxVec3*> mNormals;
	std::vector<NxU32*>  mIndices;

	NxU32* mNumVertices;
	NxU32* mNumIndices;
	NxU32* mMeshDirtyFlags;
	NxU32  mNumTriangles;
};

#endif
