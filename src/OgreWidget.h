#ifndef OGREWIDGET_H
#define OGREWIDGET_H

#include <QWidget>

#include <Ogre.h>

class OgreWidget : public QWidget
{
	Q_OBJECT

public:
	OgreWidget(QWidget *parent = 0);
	~OgreWidget();

protected:
	void showEvent(QShowEvent *evt);
	void paintEvent(QPaintEvent *evt);
	void resizeEvent(QResizeEvent *evt);
	void timerEvent(QTimerEvent *evt);

	bool setup();
	void setupResources();
	bool configure();
	void chooseSceneManager();
	void createCamera();
	void createViewports();
	void loadResources();
	void render();

	virtual void createScene() = 0;

	Ogre::Root         *mRoot;
	Ogre::RenderWindow *mWindow;
	Ogre::SceneManager *mSceneMgr;
	Ogre::Camera       *mCamera;
	Ogre::Viewport     *mViewport;
	Ogre::String        mPluginsCfg;

private:
	QSize sizeHint() const;
};

#endif
