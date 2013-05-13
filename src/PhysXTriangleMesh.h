#ifndef _PHYSX_TRIANGLE_MESH_H_
#define _PHYSX_TRIANGLE_MESH_H_

#include "ObjMeshExt.h"

class PhysXTriangleMesh
{
public:
	PhysXTriangleMesh(NxScene* scene, Ogre::SceneManager* sceneMgr,
	                  NxTriangleMeshShapeDesc &desc, ObjMeshExt* objMesh);
	~PhysXTriangleMesh();

	NxActor* getNxActor() {return mActor;}
	Ogre::SceneNode* getSceneNode() {return mTriangleNode;}

private:
	bool saveMeshDesc(NxTriangleMeshDesc &desc, ObjMeshExt* objMesh);
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
