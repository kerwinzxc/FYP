#include "OgreWidget.h"

OgreWidget::OgreWidget()
	: mSceneEntity(0),
	  mTreeEntity(0)
{
	mSceneNode   = NULL;
	mSceneLoader = NULL;
	mTreeNode    = NULL;
	mTreeLoader  = NULL;
}

OgreWidget::~OgreWidget()
{
	if (mSceneLoader)
		delete mSceneLoader;
	if (mTreeLoader)
		delete mTreeLoader;
}

void OgreWidget::createScene()
{
	mTreeLoader = new ObjLoader(25657);
	Ogre::MeshPtr tree = mTreeLoader->loadObj("tree.obj");
	mTreeEntity = mSceneMgr->createEntity(tree);
	mTreeNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("TreeNode");
	mTreeNode->attachObject(mTreeEntity);

	mSceneLoader = new ObjLoader(131056, 2);
	Ogre::MeshPtr scene = mSceneLoader->loadObj("scene.new.obj");
	mSceneEntity = mSceneMgr->createEntity(scene);
	mSceneNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("SceneNode");
	mSceneNode->attachObject(mSceneEntity);
}
