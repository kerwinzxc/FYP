#include "OgreApp.h"

OgreApp::OgreApp() : mTerrain(NULL), mTree(NULL), mTreeBody(0), mFluid(NULL),
                     mTerrainObj(NULL), mTreeObj(NULL), mLeafObj(NULL),
                     mLastState(CPU), mStatesPanel(0), mNumLeaves(100),
                     mMouseDistance(0.0, 0.0, 0.0), mWindVector(0.0, 0.0, 0.0),
                     mTerrainAnimState(NULL), mTreeAnimState(NULL), mFluidAnimState(NULL)
{
	mPhysXSys = new PhysXSystem();
	mPhysXSys->initPhysX();
}

OgreApp::~OgreApp()
{
	mSceneMgr->destroyAllCameras();
	if (mTerrainObj)
		delete mTerrainObj;
	if (mTreeObj)
		delete mTreeObj;
	if (mLeafObj)
		delete mLeafObj;
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

void OgreApp::createCamera()
{
	BaseApplication::createCamera();
	resetCamPos();
}

void OgreApp::createFrameListener()
{
	BaseApplication::createFrameListener();
	mTrayMgr->hideLogo();
	mTrayMgr->showCursor();
	mTrayMgr->showFrameStats(TL_TOPLEFT);
	mTrayMgr->toggleAdvancedFrameStats();

	mTrayMgr->destroyWidget(mDetailsPanel);

	StringVector labels;
	labels.push_back("(H) GPU");
	labels.push_back("(R) Poly Mode");
	labels.push_back("(1) Wind");
	labels.push_back("(2) Tree");
	labels.push_back("(3) Leaves");
	labels.push_back("(4) Fluid");
	labels.push_back("(+/-) Max Leaves");
	labels.push_back("Animation");
	mStatesPanel = mTrayMgr->createParamsPanel(TL_BOTTOMLEFT, "States", 210, labels);
	mStatesPanel->setParamValue(0, "Off");
	mStatesPanel->setParamValue(1, "Solid");
	mStatesPanel->setParamValue(2, "Off");
	mStatesPanel->setParamValue(6, StringConverter::toString(mNumLeaves));
	mStatesPanel->setParamValue(7, "None");
}

void OgreApp::createScene()
{
	mSceneMgr->setSkyBox(true, "Examples/EarlyMorningSkyBox");
	createTerrain();
	createTreeBody();
	createTree();
	createLeaves();
	createFluid();

	mCameraNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	mHelperNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(Ogre::Vector3(-80.0, 50.0, 50.0));
	mCameraNode->attachObject(mCamera);

	Animation* terrainAnim = mSceneMgr->createAnimation("TerrainCameraTrack", 20);
	terrainAnim->setInterpolationMode(Animation::IM_SPLINE);
	NodeAnimationTrack* terrainTrack = terrainAnim->createNodeTrack(0, mCameraNode);
	terrainTrack->createNodeKeyFrame( 0)->setTranslate(Ogre::Vector3( 300, 0,    0));
	terrainTrack->createNodeKeyFrame( 5)->setTranslate(Ogre::Vector3(   0, 0,  300));
	terrainTrack->createNodeKeyFrame(10)->setTranslate(Ogre::Vector3(-300, 0,    0));
	terrainTrack->createNodeKeyFrame(15)->setTranslate(Ogre::Vector3(   0, 0, -300));
	terrainTrack->createNodeKeyFrame(20)->setTranslate(Ogre::Vector3( 300, 0,    0));
	mTerrainAnimState = mSceneMgr->createAnimationState("TerrainCameraTrack");

	Animation* treeAnim = mSceneMgr->createAnimation("TreeCameraTrack", 20);
	treeAnim->setInterpolationMode(Animation::IM_SPLINE);
	NodeAnimationTrack* treeTrack = treeAnim->createNodeTrack(0, mCameraNode);
	treeTrack->createNodeKeyFrame( 0)->setTranslate(Ogre::Vector3( 105, 10,    0));
	treeTrack->createNodeKeyFrame( 5)->setTranslate(Ogre::Vector3(   0,  0,  105));
	treeTrack->createNodeKeyFrame(10)->setTranslate(Ogre::Vector3(-105,  0,    0));
	treeTrack->createNodeKeyFrame(15)->setTranslate(Ogre::Vector3(   0, 10, -105));
	treeTrack->createNodeKeyFrame(20)->setTranslate(Ogre::Vector3( 105, 10,    0));
	mTreeAnimState = mSceneMgr->createAnimationState("TreeCameraTrack");

	Animation* fluidAnim = mSceneMgr->createAnimation("FluidCameraTrack", 20);
	fluidAnim->setInterpolationMode(Animation::IM_SPLINE);
	NodeAnimationTrack* fluidTrack = fluidAnim->createNodeTrack(0, mCameraNode);
	fluidTrack->createNodeKeyFrame( 0)->setTranslate(Ogre::Vector3( 100, 0,    0));
	fluidTrack->createNodeKeyFrame( 5)->setTranslate(Ogre::Vector3(   0, 0,  100));
	fluidTrack->createNodeKeyFrame(10)->setTranslate(Ogre::Vector3(-100, 0,    0));
	fluidTrack->createNodeKeyFrame(15)->setTranslate(Ogre::Vector3(   0, 0, -100));
	fluidTrack->createNodeKeyFrame(20)->setTranslate(Ogre::Vector3( 100, 0,    0));
	mFluidAnimState = mSceneMgr->createAnimationState("FluidCameraTrack");
}

void OgreApp::destroyScene()
{
	clearPhysX();
	mSceneMgr->clearScene();
}

bool OgreApp::frameStarted(const FrameEvent& evt)
{
	if (mPhysXSys)
		mPhysXSys->stepPhysX(1.0f / 60.0f);
	if (mTree)
	{
		mTree->render();
		mStatesPanel->setParamValue(3, "On");
	}
	if (mLeaves.size() > 0)
	{
		if (mLeaves.size() != mNumLeaves)
		{
			for (int i = mNumLeaves; i < mLeaves.size(); i++)
				delete mLeaves[i];
			size_t i = mLeaves.size();
			mLeaves.resize(mNumLeaves);
			for (; i < mNumLeaves; ++i)
			{
				mLeafDesc.globalPose.t = getLeafPosition();
				mLeaves[i] = new PhysXCloth(mPhysXSys->getScene(), mSceneMgr, mLeafDesc, mLeafObj, i);
			}
		}

		for (size_t i = 0; i < mLeaves.size(); ++i)
		{
			NxVec3 position = mLeaves[i]->getNxCloth()->getPosition(0);
			if (!(-100.0 < position.x && position.x <  90.0 &&
			         0.0 < position.y && position.y < 120.0 &&
			         0.0 < position.z && position.z < 150.0))
			{
				delete mLeaves[i];
				mLeafDesc.globalPose.t = getLeafPosition();
				mLeaves[i] = new PhysXCloth(mPhysXSys->getScene(), mSceneMgr, mLeafDesc, mLeafObj, i);
			}
			if (!mWindVector.isZero())
				mLeaves[i]->getNxCloth()->wakeUp();
			mLeaves[i]->getNxCloth()->setWindAcceleration(mWindVector * NxMath::rand(0.01f, 0.35f));
			mLeaves[i]->render();
		}
		mStatesPanel->setParamValue(4, "On");
	}
	if (mFluid)
	{
		mFluid->render();
		mStatesPanel->setParamValue(5, "On");
	}
	return true;
}

bool OgreApp::frameRenderingQueued(const FrameEvent& evt)
{
	if(mWindow->isClosed())
		return false;

	if(mShutDown)
		return false;

	//Need to capture/update each device
	mKeyboard->capture();
	mMouse->capture();

	mTrayMgr->frameRenderingQueued(evt);

	if (!mTrayMgr->isDialogVisible())
	{
		Ogre::Vector3 position = mCamera->getPosition();
		if (position.x < -80)
			position.x = -80;
		else if (position.x > 190)
			position.x = 190;
		if (position.y < 50)
			position.y = 50;
		else if (position.y > 150)
			position.y = 150;
		if (position.z < -20)
			position.z = -20;
		else if (position.z > 300)
			position.z = 300;
		mCamera->setPosition(position);
		mCameraMan->frameRenderingQueued(evt);   // if dialog isn't up, then update the camera
	}

	if (mTerrainAnimState->getEnabled())
		mTerrainAnimState->addTime(evt.timeSinceLastFrame);
	else if (mTreeAnimState->getEnabled())
		mTreeAnimState->addTime(evt.timeSinceLastFrame);
	else if (mFluidAnimState->getEnabled())
		mFluidAnimState->addTime(evt.timeSinceLastFrame);

	return true;
}

bool OgreApp::keyPressed(const KeyEvent &arg)
{
	if (mTrayMgr->isDialogVisible()) return true;

	switch (arg.key)
	{
	case OIS::KC_F:
		mTrayMgr->toggleAdvancedFrameStats();
		break;
	case OIS::KC_C:
		resetCamPos();
		break;
	case OIS::KC_R:
		switch (mCamera->getPolygonMode())
		{
		case Ogre::PM_SOLID:
			mCamera->setPolygonMode(PM_WIREFRAME);
			mStatesPanel->setParamValue(1, "Wireframe");
			break;
		case Ogre::PM_WIREFRAME:
			mCamera->setPolygonMode(PM_POINTS);
			mStatesPanel->setParamValue(1, "Points");
			break;
		default:
			mCamera->setPolygonMode(PM_SOLID);
			mStatesPanel->setParamValue(1, "Solid");
			break;
		}
		break;
	case OIS::KC_H:
		clearPhysX();
		mPhysXSys = new PhysXSystem();
		switch (mLastState)
		{
		case CPU:
			mPhysXSys->setGPUuse(true);
			mNumLeaves = 50;
			mStatesPanel->setParamValue(0, "On");
			mStatesPanel->setParamValue(6, StringConverter::toString(mNumLeaves));
			mLastState = GPU;
			break;
		case GPU:
			mPhysXSys->setGPUuse(true);
			mNumLeaves = 150;
			mStatesPanel->setParamValue(0, "Optimized");
			mStatesPanel->setParamValue(6, StringConverter::toString(mNumLeaves));
			mLastState = OPTIMIZED;
			break;
		case OPTIMIZED:
			mNumLeaves = 100;
			mStatesPanel->setParamValue(0, "Off");
			mStatesPanel->setParamValue(6, StringConverter::toString(mNumLeaves));
			mLastState = CPU;
			break;
		}
		mPhysXSys->initPhysX();
		createTerrain();
		createTreeBody();
		if (mStatesPanel->getParamValue("(2) Tree") == "On")
			createTree();
		if (mStatesPanel->getParamValue("(3) Leaves") == "On")
			createLeaves();
		if (mStatesPanel->getParamValue("(4) Fluid") == "On")
			createFluid();
		break;
	case OIS::KC_1:
		if (!mWindVector.isZero())
		{
			mWindVector = NxVec3(0.0, 0.0, 0.0);
			mStatesPanel->setParamValue(2, "Off");
		}
		break;
	case OIS::KC_2:
		if (mTree)
		{
			delete mTree;
			mTree = NULL;
			mStatesPanel->setParamValue(3, "Off");
		}
		else
		{
			createTree();
			mStatesPanel->setParamValue(3, "On");
		}
		break;
	case OIS::KC_3:
		if (mLeaves.size() > 0)
		{
			for (size_t i = 0; i < mLeaves.size(); ++i)
				delete mLeaves[i];
			mLeaves.clear();
			mStatesPanel->setParamValue(4, "Off");
		}
		else
		{
			createLeaves();
			mStatesPanel->setParamValue(4, "On");
		}
		break;
	case OIS::KC_4:
		if (mFluid)
		{
			delete mFluid;
			mFluid = NULL;
			mStatesPanel->setParamValue(5, "Off");
		}
		else
		{
			createFluid();
			mStatesPanel->setParamValue(5, "On");
		}
		break;
	case OIS::KC_5:
		if (mTreeAnimState->getEnabled())
			mTreeAnimState->setEnabled(false);
		else if (mFluidAnimState->getEnabled())
			mFluidAnimState->setEnabled(false);
		else
			mCameraMan->setStyle(CS_MANUAL);
		mCamera->setAutoTracking(true, mTerrain->getSceneNode());
		mCamera->setPosition(Ogre::Vector3(0.0, 0.0, 200.0));
		mTerrainAnimState->setEnabled(true);
		mStatesPanel->setParamValue(7, "Terrain");
		break;
	case OIS::KC_6:
		if (mTerrainAnimState->getEnabled())
			mTerrainAnimState->setEnabled(false);
		else if (mFluidAnimState->getEnabled())
			mFluidAnimState->setEnabled(false);
		else
			mCameraMan->setStyle(CS_MANUAL);
		mCamera->setAutoTracking(true, mHelperNode);
		mCamera->setPosition(Ogre::Vector3(-80.0, 0.0, 50.0));
		mTreeAnimState->setEnabled(true);
		mStatesPanel->setParamValue(7, "Tree");
		break;
	case OIS::KC_7:
		if (mTerrainAnimState->getEnabled())
			mTerrainAnimState->setEnabled(false);
		else if (mTreeAnimState->getEnabled())
			mTreeAnimState->setEnabled(false);
		else
			mCameraMan->setStyle(CS_MANUAL);
		mCamera->setAutoTracking(true, mFluid->getSceneNode());
		mCamera->setPosition(Ogre::Vector3(3.5, 0.0, 10.0));
		mFluidAnimState->setEnabled(true);
		mStatesPanel->setParamValue(7, "Fluid");
		break;
	case OIS::KC_8:
		if (mTerrainAnimState->getEnabled())
			mTerrainAnimState->setEnabled(false);
		else if (mTreeAnimState->getEnabled())
			mTreeAnimState->setEnabled(false);
		else if (mFluidAnimState->getEnabled())
			mFluidAnimState->setEnabled(false);
		mCameraNode->setPosition(0.0, 0.0, 0.0);
		resetCamPos();
		mCameraMan->setStyle(CS_FREELOOK);
		mStatesPanel->setParamValue(7, "None");
		break;
	case OIS::KC_ADD:
		if ((mLastState != GPU && mNumLeaves < 400) ||
		    (mLastState == GPU && mNumLeaves < 50))
		{
			mNumLeaves += 10;
			mStatesPanel->setParamValue(6, StringConverter::toString(mNumLeaves));
		}
		break;
	case OIS::KC_SUBTRACT:
		if (mNumLeaves > 10)
		{
			mNumLeaves -= 10;
			mStatesPanel->setParamValue(6, StringConverter::toString(mNumLeaves));
		}
		break;
	case OIS::KC_SYSRQ:
		mWindow->writeContentsToTimestampedFile("screenshot", ".jpg");
		break;
	case OIS::KC_ESCAPE:
		mShutDown = true;
		break;
	}

	mCameraMan->injectKeyDown(arg);
	return true;
}

bool OgreApp::mouseMoved(const MouseEvent &arg)
{
	if (arg.state.buttonDown(MB_Right))
	{
		mTrayMgr->hideCursor();
		mCameraMan->injectMouseMove(arg);
	}
	else
	{
		if (mTrayMgr->injectMouseMove(arg)) return true;
		if (arg.state.buttonDown(MB_Left))
			mMouseDistance += NxVec3(arg.state.X.rel, arg.state.Y.rel, arg.state.Z.rel);
	}
	return true;
}

bool OgreApp::mousePressed(const MouseEvent &arg, MouseButtonID id)
{
	if (id == MB_Left)
		mMouseDistance = NxVec3(0.0, 0.0, 0.0);
	return BaseApplication::mousePressed(arg, id);
}

bool OgreApp::mouseReleased(const MouseEvent &arg, MouseButtonID id)
{
	if (id == MB_Left)
	{
		mMouseDistance += NxVec3(arg.state.X.rel, arg.state.Y.rel, arg.state.Z.rel);
		Ogre::Vector3 result = mMouseDistance.x * mCamera->getRight() -
		                       mMouseDistance.y * mCamera->getUp();
		mWindVector = NxVec3(result.x, result.y, result.z);
		if (mWindVector.isZero())
			mStatesPanel->setParamValue(2, "Off");
		else
			mStatesPanel->setParamValue(2, "On");
	}
	else if (id == MB_Right && !mTrayMgr->isCursorVisible())
	{
		mTrayMgr->showCursor();
	}
	return BaseApplication::mouseReleased(arg, id);
}

void OgreApp::resetCamPos()
{
	mCamera->setPosition(70.0, 80.0, 350.0);
	mCamera->lookAt(-50.0, 0.0, 20.0);
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

void OgreApp::createTerrain()
{
	NxTriangleMeshShapeDesc terrainShapeDesc;
	terrainShapeDesc.setToDefault();
	terrainShapeDesc.shapeFlags = NX_SF_FEATURE_INDICES;

	if (mTerrainObj == NULL)
	{
		mTerrainObj = new ObjMeshExt();
		char* filepath = strdup(getFilePath("scene.obj").c_str());
		mTerrainObj->loadFromObjFile(filepath);
		free(filepath);
	}

	mTerrain = new PhysXTriangleMesh(mPhysXSys->getScene(), mSceneMgr, terrainShapeDesc,
	                                 mTerrainObj);
}

void OgreApp::createTree()
{
	NxSoftBodyDesc softBodyDesc;
	softBodyDesc.setToDefault();
	softBodyDesc.particleRadius                 = 1.0f;
	softBodyDesc.volumeStiffness                = 0.9f;
	softBodyDesc.stretchingStiffness            = 1.0f;
	softBodyDesc.dampingCoefficient             = 0.8;
	softBodyDesc.friction                       = 1.0f;
	softBodyDesc.collisionResponseCoefficient   = 0.9f;
	softBodyDesc.solverIterations               = 1;
	softBodyDesc.globalPose.t                   = NxVec3(-80.0, 3.4, 50.0);

	softBodyDesc.flags |= NX_SBF_COLLISION_TWOWAY;
	if (mLastState == GPU)
		softBodyDesc.flags |= NX_SBF_HARDWARE;

	if (mTreeObj == NULL)
	{
		mTreeObj = new ObjMeshExt();
		char* filepath = strdup(getFilePath("tree.obj").c_str());
		mTreeObj->loadFromObjFile(filepath);
		free(filepath);
	}

	mTree = new PhysXSoftBody(mPhysXSys->getScene(), mSceneMgr, softBodyDesc, mTreeObj);
}

void OgreApp::createTreeBody()
{
	std::vector<CapsuleInfo> capsuleInfos;
	capsuleInfos.push_back(CapsuleInfo(NxVec3(-77.5f,  4.7f, 52.5f),  8.0f, 1.0f, NxVec3(  0.3,   0.1,  -0.1)));
	capsuleInfos.push_back(CapsuleInfo(NxVec3(-76.0f, 17.0f, 55.0f), 14.0f, 1.0f, NxVec3( 0.14,   0.1,  -0.1)));
	capsuleInfos.push_back(CapsuleInfo(NxVec3(-76.5f, 40.0f, 62.0f), 30.0f, 1.0f, NxVec3(  0.4,   0.1,   0.1)));
	capsuleInfos.push_back(CapsuleInfo(NxVec3(-76.5f, 71.0f, 72.5f), 25.0f, 0.5f, NxVec3(  0.3,   0.1,  -0.1)));
	capsuleInfos.push_back(CapsuleInfo(NxVec3(-68.0f, 34.0f, 56.5f), 18.0f, 0.5f, NxVec3( 0.14,  0.05,  -0.6)));
	capsuleInfos.push_back(CapsuleInfo(NxVec3(-74.5f, 35.0f, 69.5f), 14.0f, 0.5f, NxVec3(  1.1,  -0.2,  -0.1)));
	capsuleInfos.push_back(CapsuleInfo(NxVec3(-81.5f, 12.0f, 50.0f), 14.0f, 1.0f, NxVec3(-0.05,   0.1,   0.1)));
	capsuleInfos.push_back(CapsuleInfo(NxVec3(-85.5f, 32.0f, 51.0f), 20.0f, 1.0f, NxVec3( 0.17,   0.1,   0.2)));
	capsuleInfos.push_back(CapsuleInfo(NxVec3(-88.5f, 50.0f, 52.7f), 10.0f, 1.0f, NxVec3( 0.05,   0.1,   0.1)));
	capsuleInfos.push_back(CapsuleInfo(NxVec3(-96.0f, 66.0f, 52.7f), 10.0f, 0.5f, NxVec3( 0.05,  -0.2,   0.6)));
	capsuleInfos.push_back(CapsuleInfo(NxVec3(-90.0f, 24.0f, 49.0f), 10.0f, 1.0f, NxVec3(  0.6,  -0.3,   1.2)));
	capsuleInfos.push_back(CapsuleInfo(NxVec3(-78.5f, 22.0f, 47.5f), 27.0f, 1.0f, NxVec3(-0.15,   0.1,   0.0)));
	capsuleInfos.push_back(CapsuleInfo(NxVec3(-76.0f, 42.0f, 44.5f),  8.5f, 1.0f, NxVec3(-0.15, 0.075,  -0.5)));
	capsuleInfos.push_back(CapsuleInfo(NxVec3(-73.0f, 53.5f, 43.0f), 10.0f, 1.0f, NxVec3(-0.15, 0.075,  -0.0)));
	capsuleInfos.push_back(CapsuleInfo(NxVec3(-74.0f, 68.0f, 42.0f), 16.0f, 1.0f, NxVec3( -0.1, 0.075,  0.25)));

	mTreeBody.resize(capsuleInfos.size());
	for (size_t i = 0; i < capsuleInfos.size(); ++i)
		mTreeBody[i] = new PhysXCapsule(mPhysXSys->getScene(), capsuleInfos[i]);
	capsuleInfos.swap(std::vector<CapsuleInfo>());
}

void OgreApp::createLeaves()
{
	mLeafDesc.setToDefault();
	mLeafDesc.thickness           = 0.2f;
	mLeafDesc.bendingStiffness    = 1.0;
	mLeafDesc.stretchingStiffness = 1.0;
	mLeafDesc.dampingCoefficient  = 0.9f;
	mLeafDesc.solverIterations    = 5;
	mLeafDesc.friction            = 0.01;
	mLeafDesc.density             = 1000.0;

	mLeafDesc.flags |= NX_CLF_DAMPING;
	mLeafDesc.flags |= NX_CLF_COMDAMPING;
	mLeafDesc.flags |= NX_CLF_COLLISION_TWOWAY;
	mLeafDesc.flags |= NX_CLF_VISUALIZATION;
	if (mLastState == GPU)
		mLeafDesc.flags |= NX_CLF_HARDWARE;

	if (mLeafObj == NULL)
	{
		mLeafObj = new ObjMeshExt();
		char* filepath = strdup(getFilePath("leaf.obj").c_str());
		mLeafObj->loadFromObjFile(filepath);
		free(filepath);
	}

	mLeaves.resize(mNumLeaves);
	for (int i = 0; i < mNumLeaves; i++)
	{
		mLeafDesc.globalPose.t = getLeafPosition();
		mLeaves[i] = new PhysXCloth(mPhysXSys->getScene(), mSceneMgr, mLeafDesc, mLeafObj, i);
	}
}

void OgreApp::createFluid()
{
	NxFluidDesc fluidDesc;
	fluidDesc.setToDefault();
	fluidDesc.maxParticles                    = 15000;
	fluidDesc.kernelRadiusMultiplier          = 2.0f;
	fluidDesc.restParticlesPerMeter           = 1.5f;
	fluidDesc.motionLimitMultiplier           = 3.0f;
	fluidDesc.packetSizeMultiplier            = 8;
	fluidDesc.collisionDistanceMultiplier     = 0.1f;
	fluidDesc.stiffness                       = 50.0f;
	fluidDesc.viscosity                       = 10.0f;
	fluidDesc.restDensity                     = 200.0f;
	fluidDesc.damping                         = 0.0f;
	fluidDesc.restitutionForStaticShapes      = 0.0f;
	fluidDesc.restitutionForDynamicShapes     = 0.0f;
	fluidDesc.dynamicFrictionForStaticShapes  = 0.08f;
	fluidDesc.dynamicFrictionForDynamicShapes = 0.08f;
	fluidDesc.staticFrictionForStaticShapes   = 0.0f;
	fluidDesc.staticFrictionForDynamicShapes  = 0.0f;
	fluidDesc.numReserveParticles             = 500;
	fluidDesc.simulationMethod                = NX_F_SPH;

	fluidDesc.flags |= NX_FF_PRIORITY_MODE;
	if (mLastState == CPU)
		fluidDesc.flags &= ~NX_FF_HARDWARE;

	mFluid = new PhysXFluid(mPhysXSys->getScene(), mSceneMgr, fluidDesc);
}

void OgreApp::clearPhysX()
{
	if (mTerrain)
		delete mTerrain;
	if (mTree)
		delete mTree;
	if (mTreeBody.size() > 0)
		for (std::vector<PhysXCapsule*>::iterator i = mTreeBody.begin(); i != mTreeBody.end(); ++i)
			delete *i;
	if (mLeaves.size() > 0)
		for (std::vector<PhysXCloth*>::iterator i = mLeaves.begin(); i != mLeaves.end(); ++i)
			delete *i;
	if (mFluid)
		delete mFluid;
	delete mPhysXSys;
}
