#ifndef _PHYSX_TRIANGLE_MESH_
#define _PHYSX_TRIANGLE_MESH_

#include "Common.h"

class PhysXTriangleMesh
{
public:
	PhysXTriangleMesh(NxScene* scene, Ogre::SceneManager* sceneMgr,
	                  NxTriangleMeshShapeDesc &desc, char* objFilePath);
	~PhysXTriangleMesh();

	NxTriangleMesh* getNxTriangleMesh() {return mTriangleMesh;}

private:
	bool saveMeshDesc(NxTriangleMeshDesc &desc, char* filepath);
	void init(NxTriangleMeshShapeDesc &desc, NxTriangleMeshDesc &meshDesc);
	bool cookMesh(NxTriangleMeshDesc& desc);
	void releaseMeshDescBuffers(const NxTriangleMeshDesc& desc);

	bool            mInitDone;

	NxScene*        mScene;
	NxTriangleMesh* mTriangleMesh;
	NxActor*        mActor;

	Ogre::SceneManager* mSceneMgr;
	Ogre::ManualObject* mManualObj;
	Ogre::SceneNode*    mTriangleNode;
};

#endif
