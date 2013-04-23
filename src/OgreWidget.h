#ifndef OGREWIDGET_H
#define OGREWIDGET_H

#include "BaseApplication.h"

#include "ObjLoader.h"

class OgreWidget : public BaseApplication
{
public:
	OgreWidget();
	virtual ~OgreWidget();

protected:
	virtual void createScene();

	Ogre::SceneNode    *mSceneNode;
	Ogre::Entity       *mSceneEntity;
	ObjLoader          *mSceneLoader;

	Ogre::SceneNode    *mTreeNode;
	Ogre::Entity       *mTreeEntity;
	ObjLoader          *mTreeLoader;
};

#endif
