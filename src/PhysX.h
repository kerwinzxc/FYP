#ifndef PHYSX_H
#define PHYSX_H

#include <NxPhysics.h>
#include <NxCooking.h>

#include "OgreWidget.h"
#include "SoftBodyMesh.h"

class PhysX : public OgreWidget
{
	Q_OBJECT

public:
	PhysX(QWidget* parent = 0);
	~PhysX();

protected:
	void timerEvent(QTimerEvent *evt);

	void createScene();

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
