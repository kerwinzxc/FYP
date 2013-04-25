#ifndef _PHYSX_CAPSULE_H_
#define _PHYSX_CAPSULE_H_

#include "Common.h"

class CapsuleInfo
{
public:
	CapsuleInfo(NxVec3 globalPose, NxReal height, NxReal radius, NxVec3 localPose)
		: mGlobalPose(globalPose), mHeight(height), mRadius(radius), mLocalPose(localPose) {}
	~CapsuleInfo() {}

	NxVec3 mGlobalPose;
	NxReal mHeight;
	NxReal mRadius;
	NxVec3 mLocalPose;
};

class PhysXCapsule
{
public:
	PhysXCapsule(NxScene* scene, const CapsuleInfo& info);
	~PhysXCapsule();

	NxActor* getCapsule() {return mCapsule;}

private:
	void createCapsule(const CapsuleInfo& info);

	NxScene* mScene;
	NxActor* mCapsule;
	int      mInvisible;
};

#endif
