#ifndef PHYSX_H
#define PHYSX_H

#include "OgreWidget.h"

#include <NxOgre.h>
#include <Critter.h>

class PhysX : public OgreWidget {
	Q_OBJECT

public:
	PhysX(QWidget *parent = 0);
	~PhysX();

protected:
	void timerEvent(QTimerEvent *evt);

	void createScene();

private:
	void setupPhysics();
	void stopPhysics();

	NxOgre::World*          mWorld;
	NxOgre::Scene*          mScene;

	NxOgre::MeshManager*    mMeshManager;
	NxOgre::Material*       mDefaultMaterial;
	Critter::RenderSystem*  mRenderSystem;
};

#endif
