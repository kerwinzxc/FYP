#include "OgreApp.h"

OgreApp::OgreApp()
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
}

bool OgreApp::frameStarted(const FrameEvent& evt)
{
	if (evt.timeSinceLastFrame < 0.011f)
		mPhysXSys->stepPhysX(0.011f);
	else
		mPhysXSys->stepPhysX(evt.timeSinceLastFrame);
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
