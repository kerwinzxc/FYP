#ifndef OGREWIDGET_H
#define OGREWIDGET_H

#include <QWidget>

#include <Ogre.h>

#include "ObjLoader.h"

class OgreWidget : public QWidget
{
	Q_OBJECT

public:
	OgreWidget(QWidget *parent = 0);
	~OgreWidget();

protected:
	virtual void showEvent(QShowEvent *evt);
	virtual void paintEvent(QPaintEvent *evt);
	virtual void resizeEvent(QResizeEvent *evt);
	virtual void timerEvent(QTimerEvent *evt);

	virtual bool setup();
	virtual void setupResources();
	virtual bool configure();
	virtual void chooseSceneManager();
	virtual void createCamera();
	virtual void createViewports();
	virtual void loadResources();
	virtual void render();
	virtual void createScene();

	Ogre::Root         *mRoot;
	Ogre::RenderWindow *mWindow;
	Ogre::SceneManager *mSceneMgr;
	Ogre::Camera       *mCamera;
	Ogre::Viewport     *mViewport;
	Ogre::String        mPluginsCfg;

	Ogre::Entity       *mTreeEntity;
	ObjLoader          *mTreeLoader;

private:
	QSize sizeHint() const;
};

#endif
