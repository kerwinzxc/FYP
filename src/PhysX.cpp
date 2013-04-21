#include "PhysX.h"

#include "Stream.h"

PhysX::PhysX(QWidget *parent)
{
	mPhysicsSDK = NULL;
	mCooking    = NULL;
	mScene      = NULL;
	mTree       = NULL;
	mTreeMesh   = NULL;
	mUseGPU = true;
}

PhysX::~PhysX()
{
	mPhysicsSDK->releaseScene(*mScene);
	NxReleasePhysicsSDK(mPhysicsSDK);
}

void PhysX::timerEvent(QTimerEvent *evt)
{
	if (mScene)
	{
		mScene->simulate(0.01);
		mScene->fetchResults(NX_RIGID_BODY_FINISHED, true);
	}
	if (mTreeMesh)
		mTreeMesh->updateTetraLinks();
	OgreWidget::timerEvent(evt);
}

void PhysX::createScene()
{
	OgreWidget::createScene();
	initPhysX();
}

void PhysX::initPhysX()
{
	NxPhysicsSDKDesc SDKDesc;

	SDKDesc.setToDefault();
	if (mUseGPU)
		SDKDesc.flags &= ~NX_SDKF_NO_HARDWARE;

	NxSDKCreateError errorCode = NXCE_NO_ERROR;
	mPhysicsSDK = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION, 0, 0, SDKDesc, &errorCode);

	assert(mPhysicsSDK != NULL);

	mCooking = NxGetCookingLib(NX_PHYSICS_SDK_VERSION);

	mPhysicsSDK->getFoundationSDK().getRemoteDebugger()->connect("127.0.0.1", 5425);
	mPhysicsSDK->setParameter(NX_SKIN_WIDTH, 0.1f);

	NxSceneDesc sceneDesc;
	sceneDesc.gravity = NxVec3(0.0f, -9.8f, 0.0f);
	mScene = mPhysicsSDK->createScene(sceneDesc);

	assert(mScene != NULL);

	createTree();
}

void PhysX::createTree()
{
	NxSoftBodyDesc softBodyDesc;
	softBodyDesc.setToDefault();
	softBodyDesc.particleRadius                 = 0.2f;
	softBodyDesc.volumeStiffness                = 0.5f;
	softBodyDesc.stretchingStiffness            = 1.0f;
	softBodyDesc.friction                       = 1.0f;
	softBodyDesc.collisionResponseCoefficient   = 0.9f;
	softBodyDesc.solverIterations               = 3;
	softBodyDesc.flags |= NX_SBF_COLLISION_TWOWAY;
	if(mUseGPU)
		softBodyDesc.flags |= NX_SBF_HARDWARE;

	NxSoftBodyMeshDesc meshDesc;

	mTreeMesh = new SoftBodyMesh(mTreeEntity->getMesh(), &(mTreeLoader->mIndexLinks));

	NxArray<NxVec3>* vertices = new NxArray<NxVec3>();
	NxArray<NxU32>*  indices  = new NxArray<NxU32>();
	mTreeMesh->loadTetFile("tree.tet", vertices, indices);

	meshDesc.numVertices            = vertices->size();
	meshDesc.numTetrahedra          = indices->size() / 4;
	meshDesc.vertexStrideBytes      = sizeof(NxVec3);
	meshDesc.tetrahedronStrideBytes = 4 * sizeof(NxU32);
	meshDesc.vertexMassStrideBytes  = sizeof(NxReal);
	meshDesc.vertexFlagStrideBytes  = sizeof(NxU32);
	meshDesc.vertices               = (NxVec3*) malloc(sizeof(NxVec3) * meshDesc.numVertices);
	meshDesc.tetrahedra             = (NxU32*)  malloc(sizeof(NxU32)  * meshDesc.numTetrahedra * 4);
	meshDesc.vertexMasses           = 0;
	meshDesc.vertexFlags            = 0;
	meshDesc.flags                  = 0;

	memcpy((NxVec3*)meshDesc.vertices,  vertices->begin(), vertices->size() * sizeof(NxVec3));
	memcpy((NxU32*)meshDesc.tetrahedra, indices->begin(),  indices->size()  * sizeof(NxU32));

	assert(meshDesc.isValid());

	assert(mCooking->NxInitCooking(0, 0));

	MemoryWriteBuffer* wb = new MemoryWriteBuffer();
	assert(mCooking->NxCookSoftBodyMesh(meshDesc, *wb));
	MemoryReadBuffer* rb = new MemoryReadBuffer(wb->data);
	NxSoftBodyMesh* treeMesh = mScene->getPhysicsSDK().createSoftBodyMesh(*rb);

	mTreeMesh->buildTetraLinks((NxVec3*) meshDesc.vertices, (NxU32*) meshDesc.tetrahedra,
	                          meshDesc.numTetrahedra);
	mTreeMesh->allocateReceiveBuffers(meshDesc.numVertices, meshDesc.numTetrahedra);

	softBodyDesc.softBodyMesh = treeMesh;
	softBodyDesc.meshData = mTreeMesh->mReceiveBuffers;
	softBodyDesc.globalPose.t = NxVec3(-80.0, 0.2, 50.0);
	mTree = mScene->createSoftBody(softBodyDesc);

	delete vertices, indices, wb, rb;
}
