#ifndef PHYSX_H
#define PHYSX_H

#include <NxPhysics.h>
#include <NxCooking.h>

#include "OgreWidget.h"
#include "SoftBodyMesh.h"

class PhysX : public OgreWidget
{
public:
	PhysX();
	~PhysX();

protected:
	void createScene();
	virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);

	void initPhysX();
	void createTree();

private:
	NxPhysicsSDK*       mPhysicsSDK;
	NxCookingInterface* mCooking;
	NxScene*            mScene;
	NxSoftBody*         mTree;
	SoftBodyMesh*       mTreeMesh;

	bool mUseGPU;
};

#endif
