#include <QtGui>

#include "OgreWidget.h"

OgreWidget::OgreWidget(QWidget *parent)
	: QWidget(parent),
	  mRoot(0),
	  mWindow(0),
	  mSceneMgr(0),
	  mCamera(0),
	  mViewport(0),
	  mPluginsCfg(Ogre::StringUtil::BLANK)
{
	setAttribute(Qt::WA_OpaquePaintEvent);
	setAttribute(Qt::WA_NoSystemBackground);
}

OgreWidget::~OgreWidget()
{
	if (mWindow)
		mWindow->removeAllViewports();
	if (mRoot)
	{
		mRoot->detachRenderTarget(mWindow);
		if(mSceneMgr)
			mRoot->destroySceneManager(mSceneMgr);
	}

	delete mRoot;
}

QSize OgreWidget::sizeHint() const
{
	return QSize(800, 600);
}

void OgreWidget::showEvent(QShowEvent *evt)
{
#ifdef _DEBUG
	mPluginsCfg = "plugins_d.cfg";
#else
	mPluginsCfg = "plugins.cfg";
#endif

	if (!setup())
		return;

	startTimer(10);
}

void OgreWidget::paintEvent(QPaintEvent *evt)
{
	render();
}

void OgreWidget::resizeEvent(QResizeEvent *evt)
{
	if (evt->isAccepted())
	{
		const QSize &newSize = evt->size();

		if (mWindow)
		{
			mWindow->resize(newSize.width(), newSize.height());
			mWindow->windowMovedOrResized();
		}

		if (mCamera)
		{
			Ogre::Real aspectRatio = Ogre::Real(newSize.width()) / Ogre::Real(newSize.height());
			mCamera->setAspectRatio(aspectRatio);
		}
	}
}

void OgreWidget::timerEvent(QTimerEvent *evt)
{
	render();
}

bool OgreWidget::setup()
{
	mRoot = new Ogre::Root(mPluginsCfg);

	setupResources();

	if (!configure())
		return false;

	chooseSceneManager();
	createCamera();
	createViewports();

	// Load resources
	loadResources();

	// Create the scene
	createScene();

	return true;
}

void OgreWidget::setupResources()
{
	// Load resource paths from config file
	Ogre::ConfigFile cf;
	cf.load("resources.cfg");

	// Go through all sections & settings in the file
	Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();

	Ogre::String secName, typeName, archName;
	while (seci.hasMoreElements())
	{
		secName = seci.peekNextKey();
		Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
		Ogre::ConfigFile::SettingsMultiMap::iterator i;
		for (i = settings->begin(); i != settings->end(); ++i)
		{
			typeName = i->first;
			archName = i->second;
			Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
				archName, typeName, secName);
		}
	}
}

bool OgreWidget::configure()
{
	Ogre::String winHandle;
	Ogre::NameValuePairList params;

	Ogre::RenderSystem *mRenderSystem = mRoot->getRenderSystemByName("OpenGL Rendering Subsystem");
	mRoot->setRenderSystem(mRenderSystem);

	mRoot->initialise(false);

	winHandle = Ogre::StringConverter::toString((size_t)(HWND) winId());
	params["externalWindowHandle"] = winHandle;
	params["vsync"] = "false";

	mWindow = mRoot->createRenderWindow("OGREWIDGET Render Window", width(), height(), false, &params);

	if (mWindow)
		return true;
	else
		return false;
}

void OgreWidget::chooseSceneManager()
{
	// Get the SceneManager, in this case a generic one
	mSceneMgr = mRoot->createSceneManager(Ogre::ST_GENERIC);
}

void OgreWidget::createCamera()
{
	// Create the camera
	mCamera = mSceneMgr->createCamera("PlayerCam");

	// Set the position of camera
	mCamera->setPosition(Ogre::Vector3(0, 50, 150));

	// Look forward along Y
	mCamera->lookAt(Ogre::Vector3(0, 50, 0));
	// mCamera->setNearClipDistance(5);
}

void OgreWidget::createViewports()
{
	// Create one viewport, entire window
	mViewport = mWindow->addViewport(mCamera);
	mViewport->setBackgroundColour(Ogre::ColourValue(0, 0, 0));

	// Alter the camera aspect ratio to match the viewport
	mCamera->setAspectRatio(Ogre::Real(width()) / Ogre::Real(height()));
	// mCamera->setAspectRatio(Ogre::Real(mViewport->getActualWidth()) / Ogre::Real(mViewport->getActualHeight()));
}

void OgreWidget::loadResources()
{
	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}

void OgreWidget::render()
{
	mRoot->_fireFrameStarted();
	mWindow->update();
	mRoot->_fireFrameRenderingQueued();
	mRoot->_fireFrameEnded();
}
