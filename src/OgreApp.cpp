#include "OgreApp.h"

OgreApp::OgreApp()
	: mTerrian(NULL), mTree(NULL), mTreeBody(0)
{
	mPhysXSys = new PhysXSystem();
	// mPhysXSys->setGPUuse(false);
}

OgreApp::~OgreApp()
{
	if (mTerrian)
		delete mTerrian;
	if (mTree)
		delete mTree;
	if (mTreeBody.size() > 0)
		for (std::vector<PhysXCapsule*>::iterator i = mTreeBody.begin(); i != mTreeBody.end(); ++i)
			delete *i;
	delete mPhysXSys;
}

bool OgreApp::configure()
{
	if (mRoot->showConfigDialog())
	{
		mWindow = mRoot->initialise(true, "PhysX 3D Virutal Simulation");
		return true;
	}
	else
	{
		return false;
	}
}

void OgreApp::createScene()
{
	createTerrian();
	createTree();
	createTreeBody();
}

bool OgreApp::frameStarted(const FrameEvent& evt)
{
	if (evt.timeSinceLastFrame < 0.010f)
		mPhysXSys->stepPhysX(0.010f);
	else
		mPhysXSys->stepPhysX(evt.timeSinceLastFrame);
	if (mTree)
		mTree->render();
	return true;
}

String OgreApp::getFilePath(const String& filename)
{
	ResourceGroupManager& resGrpMgr = ResourceGroupManager::getSingleton();
	String resGroup = resGrpMgr.findGroupContainingResource(filename);
	FileInfoListPtr fileList = resGrpMgr.findResourceFileInfo(resGroup, filename);
	VectorIterator<FileInfoList> fileListItr(*fileList);
	String filepath;
	if (fileListItr.hasMoreElements())
	{
		FileInfo fileInfo = fileListItr.getNext();
		filepath = fileInfo.archive->getName() + "/" + fileInfo.filename;
	}
	return filepath;
}

void OgreApp::createTerrian()
{
	NxTriangleMeshShapeDesc terrainShapeDesc;
	terrainShapeDesc.shapeFlags = NX_SF_FEATURE_INDICES;

	char* filepath = strdup(getFilePath("scene.new.obj").c_str());
	mTerrian = new PhysXTriangleMesh(mPhysXSys->getScene(), mSceneMgr, terrainShapeDesc,
	                                 filepath);
	free(filepath);
}

void OgreApp::createTree()
{
	NxSoftBodyDesc softBodyDesc;
	softBodyDesc.particleRadius                 = 1.0f;
	softBodyDesc.volumeStiffness                = 0.9f;
	softBodyDesc.stretchingStiffness            = 1.0f;
	softBodyDesc.dampingCoefficient             = 0.8;
	softBodyDesc.friction                       = 1.0f;
	softBodyDesc.collisionResponseCoefficient   = 0.9f;
	softBodyDesc.solverIterations               = 1;
	softBodyDesc.globalPose.t                   = NxVec3(-80.0, 0.2, 50.0);

	softBodyDesc.flags  = NX_SBF_GRAVITY | NX_SBF_VOLUME_CONSERVATION;
	softBodyDesc.flags |= NX_SBF_COLLISION_TWOWAY;
	if (mPhysXSys->getGPUuse())
		softBodyDesc.flags |= NX_SBF_HARDWARE;

	char* filepath = strdup(getFilePath("tree.obj").c_str());
	mTree = new PhysXSoftBody(mPhysXSys->getScene(), mSceneMgr, softBodyDesc, filepath);
	free(filepath);
}

void OgreApp::createTreeBody()
{
	std::vector<CapsuleInfo> capsuleInfos;
	capsuleInfos.push_back(CapsuleInfo(NxVec3(-77.5f,  4.7f, 52.5f),  8.0f, 1.0f, NxVec3(  0.3,   0.1,  -0.1)));
	capsuleInfos.push_back(CapsuleInfo(NxVec3(-76.0f, 17.0f, 55.0f), 14.0f, 1.0f, NxVec3( 0.14,   0.1,  -0.1)));
	capsuleInfos.push_back(CapsuleInfo(NxVec3(-76.5f, 40.0f, 62.0f), 30.0f, 1.0f, NxVec3(  0.4,   0.1,   0.1)));
	capsuleInfos.push_back(CapsuleInfo(NxVec3(-76.5f, 71.0f, 72.5f), 30.0f, 1.0f, NxVec3(  0.3,   0.1,  -0.1)));
	capsuleInfos.push_back(CapsuleInfo(NxVec3(-65.0f, 36.0f, 56.5f), 26.0f, 1.0f, NxVec3( 0.14,  0.05,  -0.6)));
	capsuleInfos.push_back(CapsuleInfo(NxVec3(-74.5f, 35.0f, 72.5f), 25.0f, 1.0f, NxVec3(  1.1,  -0.2,  -0.1)));
	capsuleInfos.push_back(CapsuleInfo(NxVec3(-70.0f, 45.0f, 71.5f), 17.0f, 1.0f, NxVec3(  1.1,   0.0, -0.65)));
	capsuleInfos.push_back(CapsuleInfo(NxVec3(-82.0f, 22.5f, 62.5f), 17.0f, 1.0f, NxVec3(  1.1,   0.0,   0.6)));
	capsuleInfos.push_back(CapsuleInfo(NxVec3(-80.0f, 60.0f, 77.5f), 17.0f, 1.0f, NxVec3(  1.1,   0.0,  0.35)));
	capsuleInfos.push_back(CapsuleInfo(NxVec3(-81.5f, 12.0f, 50.0f), 14.0f, 1.0f, NxVec3(-0.05,   0.1,   0.1)));
	capsuleInfos.push_back(CapsuleInfo(NxVec3(-85.5f, 32.0f, 51.0f), 20.0f, 1.0f, NxVec3( 0.17,   0.1,   0.2)));
	capsuleInfos.push_back(CapsuleInfo(NxVec3(-88.5f, 50.0f, 52.7f), 10.0f, 1.0f, NxVec3( 0.05,   0.1,   0.1)));
	capsuleInfos.push_back(CapsuleInfo(NxVec3(-96.0f, 66.0f, 52.7f), 20.0f, 1.0f, NxVec3( 0.05,  -0.2,   0.6)));
	capsuleInfos.push_back(CapsuleInfo(NxVec3(-96.0f, 29.0f, 49.0f), 20.0f, 1.0f, NxVec3(  0.6,  -0.3,   1.2)));
	capsuleInfos.push_back(CapsuleInfo(NxVec3(-92.0f, 48.0f, 42.0f), 20.0f, 1.0f, NxVec3(  2.0,   0.0, -0.35)));
	capsuleInfos.push_back(CapsuleInfo(NxVec3(-78.5f, 22.0f, 47.5f), 27.0f, 1.0f, NxVec3(-0.15,   0.1,   0.0)));
	capsuleInfos.push_back(CapsuleInfo(NxVec3(-76.0f, 42.0f, 44.5f),  8.5f, 1.0f, NxVec3(-0.15, 0.075,  -0.5)));
	capsuleInfos.push_back(CapsuleInfo(NxVec3(-73.0f, 53.5f, 43.0f), 10.0f, 1.0f, NxVec3(-0.15, 0.075,  -0.0)));
	capsuleInfos.push_back(CapsuleInfo(NxVec3(-74.0f, 68.0f, 42.0f), 16.0f, 1.0f, NxVec3( -0.1, 0.075,  0.25)));
	capsuleInfos.push_back(CapsuleInfo(NxVec3(-78.5f, 27.0f, 39.5f), 25.0f, 1.0f, NxVec3( -0.6,  -0.2,   0.0)));
	capsuleInfos.push_back(CapsuleInfo(NxVec3(-74.5f, 48.0f, 34.5f), 18.0f, 1.0f, NxVec3( -1.1,  -0.2,  -0.2)));

	for (std::vector<CapsuleInfo>::iterator i = capsuleInfos.begin(); i != capsuleInfos.end(); ++i)
	{
		mTreeBody.push_back(new PhysXCapsule(mPhysXSys->getScene(), *i));
	}
	capsuleInfos.swap(std::vector<CapsuleInfo>());

	if (mTree)
		mTree->getNxSoftBody()->attachToCollidingShapes(0);
}
