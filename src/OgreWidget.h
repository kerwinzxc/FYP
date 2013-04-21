#ifndef OGREWIDGET_H
#define OGREWIDGET_H

#include "BaseApplication.h"

#include "ObjLoader.h"

class OgreWidget : public BaseApplication
{
public:
	OgreWidget();
	~OgreWidget();

protected:
	virtual void createScene();

	Ogre::Entity       *mTreeEntity;
	ObjLoader          *mTreeLoader;
};

#endif
