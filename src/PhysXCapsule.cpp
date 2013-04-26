#include "PhysXCapsule.h"

PhysXCapsule::PhysXCapsule(NxScene* scene, const CapsuleInfo& info)
	: mScene(scene), mCapsule(NULL)
{
	createCapsule(info);
}

PhysXCapsule::~PhysXCapsule()
{
	if (mCapsule)
		mScene->releaseActor(*mCapsule);
}

void PhysXCapsule::createCapsule(const CapsuleInfo& info)
{
	NxCapsuleShapeDesc capsuleDesc;
	capsuleDesc.setToDefault();
	capsuleDesc.height = info.mHeight;
	capsuleDesc.radius = info.mRadius;

	NxMat33 rot0, rot1,rot2;
	rot0.rotX(info.mLocalPose.x);
	rot1.rotY(info.mLocalPose.y);
	rot2.rotZ(info.mLocalPose.z);
	rot0 = rot0 * rot1 * rot2;
	capsuleDesc.localPose.M = rot0;

	NxActorDesc actorDesc;
	actorDesc.setToDefault();
	actorDesc.shapes.pushBack(&capsuleDesc);
	actorDesc.density = 1.0;
	actorDesc.globalPose.t = info.mGlobalPose;

	mCapsule = mScene->createActor(actorDesc);
	mCapsule->userData = (void*) &mInvisible;
}
