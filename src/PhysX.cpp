#include "PhysX.h"

#include "Stream.h"

PhysX::PhysX()
{
	mPhysicsSDK = NULL;
	mCooking    = NULL;
	mScene      = NULL;
	mTerrian    = NULL;
	mTree       = NULL;
	mTreeMesh   = NULL;
	mUseGPU     = true;
}

PhysX::~PhysX()
{
	mPhysicsSDK->releaseScene(*mScene);
	NxReleasePhysicsSDK(mPhysicsSDK);
}

void PhysX::createScene()
{
	OgreWidget::createScene();
	initPhysX();
}

bool PhysX::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
	bool ret = BaseApplication::frameRenderingQueued(evt);

	assert(mScene);
	stepPhysX(evt.timeSinceLastFrame);

	return ret;
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

	createTerrian();
	createTree();
}

void PhysX::stepPhysX(NxReal timeSinceLastFrame)
{
	mScene->simulate(timeSinceLastFrame);
	mScene->fetchResults(NX_RIGID_BODY_FINISHED, true);
	if (mTreeMesh)
		mTreeMesh->updateTetraLinks();
}

void PhysX::createTerrian()
{
	if (mTerrian != NULL)
	{
		mScene->releaseActor(*mTerrian);
		mTerrian = NULL;
	}

	NxArray<NxVec3>* vertices = new NxArray<NxVec3>();
	NxArray<NxU32>*  indices  = new NxArray<NxU32>();

	NxU32 count = 0;
	for (int i = 0; i < mSceneEntity->getMesh()->getNumSubMeshes(); i++)
	{
		Ogre::SubMesh* submesh = mSceneEntity->getMesh()->getSubMesh(i);
		Ogre::VertexData* vertexData = submesh->vertexData;
		const Ogre::VertexElement* posElem = vertexData->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);
		Ogre::HardwareVertexBufferSharedPtr vbuf = vertexData->vertexBufferBinding->getBuffer(posElem->getSource());
		unsigned char* vertex = static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));

		Ogre::Real* pReal;
		size_t offset = vbuf->getVertexSize();
		for (size_t j = 0; j < vertexData->vertexCount; ++j, vertex += offset)
		{
			posElem->baseVertexPointerToElement(vertex, &pReal);
			vertices->push_back(NxVec3(pReal[0], pReal[1], pReal[2]));
			indices->push_back(count++);
		}

		vbuf->unlock();
	}

	vertices->resize(vertices->size());
	indices->resize(indices->size());

	NxTriangleMeshDesc terrainDesc;
	terrainDesc.numVertices               = vertices->size();
	terrainDesc.numTriangles              = count / 3;
	terrainDesc.pointStrideBytes          = sizeof(NxVec3);
	terrainDesc.triangleStrideBytes       = 3 * sizeof(NxU32);
	terrainDesc.flags                     = 0;
	terrainDesc.heightFieldVerticalAxis   = NX_Y;
	terrainDesc.heightFieldVerticalExtent = 1000.0f;
	terrainDesc.points                    = vertices->begin();
	terrainDesc.triangles                 = indices->begin();

	assert(terrainDesc.isValid());

	assert(mCooking->NxInitCooking(0, 0));
	MemoryWriteBuffer* wb = new MemoryWriteBuffer();
	assert(mCooking->NxCookTriangleMesh(terrainDesc, *wb));
	MemoryReadBuffer* rb = new MemoryReadBuffer(wb->data);
	NxTriangleMesh* terrainMesh = mPhysicsSDK->createTriangleMesh(*rb);
	mCooking->NxCloseCooking();

	NxTriangleMeshShapeDesc terrainShapeDesc;
	terrainShapeDesc.meshData   = terrainMesh;
	terrainShapeDesc.shapeFlags = NX_SF_FEATURE_INDICES;

	NxActorDesc ActorDesc;
	ActorDesc.shapes.pushBack(&terrainShapeDesc);
	ActorDesc.globalPose.t = NxVec3(0.0f,0.0f,0.0f);
	mTerrian = mScene->createActor(ActorDesc);

	if (terrainMesh->getReferenceCount() == 0)
		mPhysicsSDK->releaseTriangleMesh(*terrainMesh);

	delete wb, rb;
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
	if (mUseGPU)
		softBodyDesc.flags |= NX_SBF_HARDWARE;

	NxSoftBodyMeshDesc meshDesc;

	mTreeMesh = new SoftBodyMesh(mTreeEntity->getMesh(), &(mTreeLoader->mIndexLinks));

	NxArray<NxVec3>* vertices = new NxArray<NxVec3>();
	NxArray<NxU32>*  indices  = new NxArray<NxU32>();
	mTreeMesh->loadTetFile("tree.tet", vertices, indices);
	vertices->resize(vertices->size());
	indices->resize(indices->size());

	meshDesc.numVertices            = vertices->size();
	meshDesc.numTetrahedra          = indices->size() / 4;
	meshDesc.vertexStrideBytes      = sizeof(NxVec3);
	meshDesc.tetrahedronStrideBytes = 4 * sizeof(NxU32);
	meshDesc.vertexMassStrideBytes  = sizeof(NxReal);
	meshDesc.vertexFlagStrideBytes  = sizeof(NxU32);
	meshDesc.vertices               = vertices->begin();
	meshDesc.tetrahedra             = indices->begin();
	meshDesc.vertexMasses           = 0;
	meshDesc.vertexFlags            = 0;
	meshDesc.flags                  = 0;

	assert(meshDesc.isValid());

	assert(mCooking->NxInitCooking(0, 0));
	MemoryWriteBuffer* wb = new MemoryWriteBuffer();
	assert(mCooking->NxCookSoftBodyMesh(meshDesc, *wb));
	MemoryReadBuffer* rb = new MemoryReadBuffer(wb->data);
	NxSoftBodyMesh* treeMesh = mPhysicsSDK->createSoftBodyMesh(*rb);
	mCooking->NxCloseCooking();

	mTreeMesh->buildTetraLinks((NxVec3*) meshDesc.vertices, (NxU32*) meshDesc.tetrahedra,
							meshDesc.numTetrahedra);
	mTreeMesh->allocateReceiveBuffers(meshDesc.numVertices, meshDesc.numTetrahedra);

	softBodyDesc.softBodyMesh = treeMesh;
	softBodyDesc.meshData = mTreeMesh->mReceiveBuffers;
	softBodyDesc.globalPose.t = NxVec3(-80.0, 0.2, 50.0);
	mTree = mScene->createSoftBody(softBodyDesc);

	if (treeMesh->getReferenceCount() == 0)
		mPhysicsSDK->releaseSoftBodyMesh(*treeMesh);

	delete wb, rb;
}

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
	int main(int argc, char *argv[])
#endif
	{
		// Create application object
		PhysX app;

		try {
			app.go();
		} catch( Ogre::Exception& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
			MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
			std::cerr << "An exception has occured: " <<
				e.getFullDescription().c_str() << std::endl;
#endif
		}

		return 0;
	}

#ifdef __cplusplus
}
#endif
