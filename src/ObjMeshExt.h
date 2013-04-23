#ifndef _OBJ_MESH_EXTEND_
#define _OBJ_MESH_EXTEND_

#include "Common.h"

class ObjMeshExt : public ObjMesh
{
public:
	ObjMeshExt() {clear();}

	void parseRef(char *s, int nr[]);
	bool loadFromObjFile(char *filename);

	int getNumMaterials() const { return (int)mMaterials.size(); }

	const NxVec3& getNormal(int i) const { return mNormals[i]; }
	const TexCoord& getTexCoord(int i) const { return mTexCoords[i]; }
	const ObjMeshMaterial& getMaterial(int i) const { return mMaterials[i]; }
};

#endif
