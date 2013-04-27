#include "PhysXSystem.h"

PhysXSystem::PhysXSystem()
	: mPhysicsSDK(NULL),
	  mScene(NULL),
	  mCookingInitialized(false),
	  mUseGPU(true)
{
	initPhysX();
}

PhysXSystem::~PhysXSystem()
{
	shutdownPhysX();
}

bool PhysXSystem::initPhysX()
{
	NxPhysicsSDKDesc SDKDesc;
	SDKDesc.setToDefault();
	if (mUseGPU)
		SDKDesc.flags &= ~NX_SDKF_NO_HARDWARE;

	NxSDKCreateError errorCode = NXCE_NO_ERROR;
	mPhysicsSDK = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION, 0, 0, SDKDesc, &errorCode);

	if (mPhysicsSDK == NULL)
		return false;

	if (mPhysicsSDK->getFoundationSDK().getRemoteDebugger())
		mPhysicsSDK->getFoundationSDK().getRemoteDebugger()->connect("localhost",
					NX_DBG_DEFAULT_PORT, NX_DBG_EVENTMASK_EVERYTHING);

	if (!mCookingInitialized)
	{
		mCookingInitialized = true;
		if (!NxInitCooking(0, 0))
			return false;
	}

	mPhysicsSDK->setParameter(NX_SKIN_WIDTH, 0.05f);
	NxSceneDesc sceneDesc;
	sceneDesc.gravity = NxVec3(0.0f, -9.8f, 0.0f);
	mScene = mPhysicsSDK->createScene(sceneDesc);
	if(mScene == NULL)
		return false;

	return true;
}

void PhysXSystem::stepPhysX(NxReal elapsedTime)
{
	mScene->simulate(elapsedTime);
	mScene->flushStream();
	mScene->fetchResults(NX_RIGID_BODY_FINISHED, true);
}

void PhysXSystem::shutdownPhysX()
{
	if (mCookingInitialized)
		NxCloseCooking();

	if (mPhysicsSDK != NULL)
	{
		if (mScene != NULL)
			mPhysicsSDK->releaseScene(*mScene);
		mScene = NULL;
		NxReleasePhysicsSDK(mPhysicsSDK);
		mPhysicsSDK = NULL;
	}
}
