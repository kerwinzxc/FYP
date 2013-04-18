#ifndef PHYSX_H
#define PHYSX_H

#include "OgreWidget.h"

class PhysX : public OgreWidget {
	Q_OBJECT

public:
	PhysX(QWidget *parent = 0);
	~PhysX();
};

#endif
