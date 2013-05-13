#ifndef _PHYSX_FLUID_H_
#define _PHYSX_FLUID_H_

#include "Common.h"

struct ParticleSDK
{
	NxVec3 position;
	NxVec3 velocity;
	NxReal density;
	NxReal lifetime;
	NxU32  id;
	NxVec3 collisionNormal;
};

class PhysXFluid
{
public:
	PhysXFluid(NxScene* scene, Ogre::SceneManager* sceneMgr, NxFluidDesc &desc);
	~PhysXFluid();

	NxFluid* getNxFluid() {return mFluid;}
	Ogre::SceneNode* getSceneNode() {return mFluidNode;}
	void render();

private:
	void init(NxFluidDesc &desc);
	NxVec3 RandNormalVec();
	bool initFrameActor();
	bool initFluidEmitter();
	bool initFluidDrain();

	void initParticleSystem(NxFluidDesc &desc);

	static const int msInitParticles = 10000;

	bool mInitDone;

	NxScene*        mScene;
	NxFluid*        mFluid;
	NxActor*        mFrameActor;
	NxActor*        mFluidDrain;

	Ogre::SceneManager*   mSceneMgr;
	Ogre::SceneNode*      mFluidNode;
	Ogre::ParticleSystem* mParticleSystem;

	NxVec3* mPositions;
	NxVec3* mVelocities;
	NxU32   mNumOfParticles;
};

#endif
