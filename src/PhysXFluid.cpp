#include "PhysXFluid.h"

PhysXFluid::PhysXFluid(NxScene* scene, Ogre::SceneManager* sceneMgr, NxFluidDesc &desc)
	: mInitDone(false), mScene(scene), mFluid(NULL),
	  mSceneMgr(sceneMgr), mFluidNode(NULL), mParticleSystem(NULL), mNumOfParticles(0)
{
	mPositions       = (NxVec3*) malloc(sizeof(NxVec3) * desc.maxParticles);
	mVelocities      = (NxVec3*) malloc(sizeof(NxVec3) * desc.maxParticles);

	init(desc);
	initParticleSystem(desc);
}

PhysXFluid::~PhysXFluid()
{
	if (mInitDone)
	{
		if (mFluidDrain)
			mScene->releaseActor(*mFluidDrain);
		if (mFrameActor)
			mScene->releaseActor(*mFrameActor);
		if (mFluid)
			mScene->releaseFluid(*mFluid);
	}

	if (mParticleSystem)
	{
		if (mParticleSystem->isAttached())
			mParticleSystem->getParentSceneNode()->detachObject(mParticleSystem);
		mSceneMgr->destroyParticleSystem(mParticleSystem);
		mParticleSystem = 0;
	}

	free(mPositions);
	free(mVelocities);
}

void PhysXFluid::init(NxFluidDesc &desc)
{
	ParticleSDK* particleSDK = new ParticleSDK[mInitParticles];
	unsigned particleCount = 0;
	while (particleCount < mInitParticles)
	{
		NxVec3 pos = RandNormalVec();
		ParticleSDK& newParticle = particleSDK[particleCount++];
		newParticle.position = pos;
		newParticle.velocity = NxVec3(0.0f, 0.0f, 0.0f);
	}

	NxParticleData particleData;
	particleData.numParticlesPtr      = &particleCount;
	particleData.bufferPos            = &particleSDK[0].position.x;
	particleData.bufferPosByteStride  = sizeof(ParticleSDK);
	particleData.bufferVel            = &particleSDK[0].velocity.x;
	particleData.bufferVelByteStride  = sizeof(ParticleSDK);
	particleData.bufferLife           = &particleSDK[0].lifetime;
	particleData.bufferLifeByteStride = sizeof(ParticleSDK);

	assert(particleData.checkValid() == 0);

	desc.initialParticleData = particleData;
	desc.particlesWriteData.bufferPos = &mPositions[0].x;
	desc.particlesWriteData.bufferPosByteStride = sizeof(NxVec3);
	desc.particlesWriteData.bufferVel = &mVelocities[0].x;
	desc.particlesWriteData.bufferVelByteStride = sizeof(NxVec3);
	desc.particlesWriteData.numParticlesPtr = &mNumOfParticles;

	mFluid = mScene->createFluid(desc);
	initFrameActor();
	initFluidEmitter();
	initFluidDrain();
	mInitDone = true;
}

NxVec3 PhysXFluid::RandNormalVec()
{
	NxReal x, y, z;

	//choose direction, uniformly distributed.
	x =  5.0f + NxMath::rand(-17.0f, 17.0f);
	y = 15.0f + NxMath::rand(  0.0f,  8.0f);
	z = 15.0f + NxMath::rand( -4.0f, 10.0f);

	return NxVec3(x, y, z);
}

bool PhysXFluid::initFrameActor()
{
	NxBoxShapeDesc boxDesc;
	boxDesc.setToDefault();
	boxDesc.dimensions.set(1.0, 1.0, 1.0);
	boxDesc.localPose.t = NxVec3(0.0, 1.0, 0.0);

	NxActorDesc actorDesc;
	actorDesc.setToDefault();
	actorDesc.shapes.pushBack(&boxDesc);
	actorDesc.globalPose.t = NxVec3(0.0, 4.0, 0.0);
	actorDesc.density = 2;

	if (actorDesc.isValid())
		mFrameActor = mScene->createActor(actorDesc);
	else
		return false;

	NxMat33 m;
	m.rotX(NxPi / 2);
	mFrameActor->setGlobalOrientation(m);
	mFrameActor->setGlobalPosition(NxVec3(3.5, 40.0, 10.0));

	return true;
}

bool PhysXFluid::initFluidEmitter()
{
	NxQuat q;
	q.fromAngleAxis(-90, NxVec3(1, 0, 0));
	NxMat34 mat;
	mat.M.fromQuat(q);
	mat.t = NxVec3(0.0f, 1.2f, 0.0f);

	NxFluidEmitterDesc emitterDesc;
	emitterDesc.setToDefault();
	emitterDesc.frameShape             = mFrameActor->getShapes()[0];
	emitterDesc.dimensionX             = 14.0f;
	emitterDesc.dimensionY             = 1.0f;
	emitterDesc.relPose                = mat;
	emitterDesc.rate                   = 500;
	emitterDesc.randomAngle            = 0.01f;
	emitterDesc.randomPos              = NxVec3(0.0f, 0.0f, 0.0f);
	emitterDesc.fluidVelocityMagnitude = 4.0f;
	emitterDesc.repulsionCoefficient   = 0.9f;
	emitterDesc.maxParticles           = 0;
	emitterDesc.particleLifetime       = 2.0;
	emitterDesc.type                   = NX_FE_CONSTANT_FLOW_RATE;
	emitterDesc.shape                  = NX_FE_ELLIPSE;

	if (emitterDesc.isValid())
		mFluid->createEmitter(emitterDesc);
	else
		return false;
	return true;
}

bool PhysXFluid::initFluidDrain()
{
	NxBoxShapeDesc drainDesc;
	NxActorDesc drainActorDesc;

	drainDesc.setToDefault();
	drainDesc.shapeFlags |= NX_SF_FLUID_DRAIN;
	drainDesc.dimensions.set(20.0f, 1.0f, 0.01f);
	drainActorDesc.shapes.pushBack(&drainDesc);

	if (drainActorDesc.isValid())
		mFluidDrain = mScene->createActor(drainActorDesc);
	else
		return false;

	mFluidDrain->setGlobalPosition(NxVec3(5, 18, 30));

	return true;
}

void PhysXFluid::initParticleSystem(NxFluidDesc &desc)
{
	mFluidNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	mParticleSystem = mSceneMgr->createParticleSystem(desc.maxParticles * 2);
	mFluidNode->attachObject(mParticleSystem);

	mParticleSystem->setDefaultDimensions(1.0f, 1.0f);
	mParticleSystem->setMaterialName("Examples/Water2");
	mParticleSystem->setParticleQuota(desc.maxParticles);
	mParticleSystem->setSpeedFactor(0.001f);
}

void PhysXFluid::render()
{
	if (mNumOfParticles > mParticleSystem->getNumParticles())
	{
		unsigned int newParticleCount = mNumOfParticles - mParticleSystem->getNumParticles();
		for (unsigned int i = 0; i < newParticleCount; i++)
		{
			Ogre::Particle* particle = mParticleSystem->createParticle();
			if (particle)
				particle->timeToLive = 1.0f;
		}
	}

	Ogre::ParticleIterator it = mParticleSystem->_getIterator();
	for (NxU32 i = 0; !it.end(); i++)
	{
		Ogre::Particle* particle = it.getNext();
		if (i < mNumOfParticles)
		{
			particle->position.x = mPositions[i].x;
			particle->position.y = mPositions[i].y;
			particle->position.z = mPositions[i].z;
			particle->direction.x = mVelocities[i].x;
			particle->direction.y = mVelocities[i].y;
			particle->direction.z = mVelocities[i].z;
		}
		else
		{
			particle->timeToLive = 0;
		}
	}
}
