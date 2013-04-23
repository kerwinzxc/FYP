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
	virtual ~PhysX();

protected:
	void createScene();
	virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);

	void initPhysX();
	void stepPhysX(NxReal timeSinceLastFrame);
	void createTree();
	void createTerrian();

private:
	NxPhysicsSDK*       mPhysicsSDK;
	NxCookingInterface* mCooking;
	NxScene*            mScene;

	NxActor*            mTerrian;
	NxSoftBody*         mTree;
	SoftBodyMesh*       mTreeMesh;

	bool mUseGPU;
};

#endif
