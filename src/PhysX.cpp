#include "PhysX.h"

PhysX::PhysX(QWidget *parent) : OgreWidget(parent)
{
}

PhysX::~PhysX()
{
	stopPhysics();
}

void PhysX::createScene()
{
	Ogre::ColourValue background = Ogre::ColourValue(16.f/255.f, 16.f/255.f, 16.f/255.f);
	mViewport->setBackgroundColour(background);
	mSceneMgr->setFog(Ogre::FOG_EXP, background, 0.001, 800, 1000);

	// set shadow properties
	mSceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_TEXTURE_MODULATIVE);
	mSceneMgr->setShadowColour(Ogre::ColourValue(0.5, 0.5, 0.5));
	mSceneMgr->setShadowTextureSize(1024);
	mSceneMgr->setShadowTextureCount(1);

	// create a floor mesh resource
	Ogre::MeshManager::getSingleton().createPlane("floor", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
	Ogre::Plane(Ogre::Vector3::UNIT_Y, 0), 1000, 1000, 1, 1, true, 1, 1, 1, Ogre::Vector3::UNIT_Z);

	// create a floor entity, give it a material, and place it at the origin
	Ogre::Entity* floor = mSceneMgr->createEntity("Floor", "floor");
	floor->setMaterialName("ground-from-nxogre.org");
	floor->setCastShadows(false);
	mSceneMgr->getRootSceneNode()->attachObject(floor);

	// use a small amount of ambient lighting
	mSceneMgr->setAmbientLight(Ogre::ColourValue(0.3, 0.3, 0.3));

	// add a bright light above the scene
	Ogre::Light* light = mSceneMgr->createLight();
	light->setType(Ogre::Light::LT_POINT);
	light->setPosition(-10, 40, 20);
	light->setSpecularColour(Ogre::ColourValue::White);

	mCamera->setPosition(10, 10, 10);
	mCamera->lookAt(0, 0, 0);
	mCamera->setNearClipDistance(0.02f);
	mCamera->setFarClipDistance(1000.0f);
	setupPhysics();
}

void PhysX::setupPhysics()
{
	// Create the world.
	mWorld = NxOgre::World::createWorld();

	// Allow NxOgre to use the Ogre resource system, via the Critter OgreResourceProtocol class.
	// - ResourceProtocols are normally owned by the ResourceSystem, so we don't have to delete it later.
	NxOgre::ResourceSystem::getSingleton()->openProtocol(new Critter::OgreResourceProtocol());

	mWorld->getRemoteDebugger()->connect();

	// Create the scene
	NxOgre::SceneDescription scene_description;
	scene_description.mGravity = NxOgre::Constants::MEAN_EARTH_GRAVITY;
	scene_description.mUseHardware = true;

	mScene = mWorld->createScene(scene_description);

	// Set default material properties
	mDefaultMaterial = mScene->getMaterial(0);
	mDefaultMaterial->setRestitution(0.1f);
	mDefaultMaterial->setDynamicFriction(0.9);
	mDefaultMaterial->setStaticFriction(0.5);

	// Get a copy of it's pointer
	mMeshManager = NxOgre::MeshManager::getSingleton();

	// Plane creation
	mScene->createSceneGeometry(NxOgre::PlaneGeometryDescription());

	// Create the rendersystem.
	mRenderSystem = new Critter::RenderSystem(mScene, mSceneMgr);

	//// mRenderSystem->setVisualisationMode(NxOgre::Enums::VisualDebugger_ShowAll);
}

void PhysX::stopPhysics()
{
	NxOgre::World::destroyWorld();
}
