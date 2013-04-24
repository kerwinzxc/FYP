#ifndef _PHYSX_SYSTEM_H_
#define _PHYSX_SYSTEM_H_

#include "Common.h"

class PhysXSystem
{
public:
	PhysXSystem();
	~PhysXSystem();

	bool initPhysX();
	void stepPhysX(NxReal elapsedTime);
	void shutdownPhysX();

	NxPhysicsSDK* getPhysicsSDK()        { return mPhysicsSDK; }
	NxScene*      getScene()             { return mScene; }
	bool          getGPUuse()            { return mUseGPU; }
	void          setGPUuse(bool useGPU) { mUseGPU = useGPU; }

private:
	NxPhysicsSDK* mPhysicsSDK;
	NxScene*      mScene;
	bool          mCookingInitialized;
	bool          mUseGPU;
};

#endif
