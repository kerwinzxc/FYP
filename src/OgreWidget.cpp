#include "OgreWidget.h"

OgreWidget::OgreWidget()
	: mTreeEntity(0)
{
	mTreeLoader = NULL;
}

OgreWidget::~OgreWidget()
{
}

void OgreWidget::createScene()
{
	mTreeLoader = new ObjLoader();
	Ogre::MeshPtr tree = mTreeLoader->loadObj("tree.obj");
	mTreeEntity = mSceneMgr->createEntity(tree);
	mSceneMgr->getRootSceneNode()->attachObject(mTreeEntity);
}
