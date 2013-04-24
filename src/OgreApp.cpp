#include "OgreApp.h"

OgreApp::OgreApp()
	: mTerrian(NULL), mTree(NULL)
{
	mPhysXSys = new PhysXSystem();
}

OgreApp::~OgreApp()
{
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
	// createTree();
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
