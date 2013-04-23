#include "ObjMeshExt.h"

void ObjMeshExt::extractPath(char *filename)
{
	Ogre::String base, ext, path;
	Ogre::StringUtil::splitFullFilename(filename, base, ext, path);
	strncpy(mPath, path.c_str(), sizeof(mPath));
	strncpy(mName, base.c_str(), sizeof(mName));
	// strcpy(mPath, filename);
	// int i = (int) strlen(mPath)-1;
	// while (i >= 0 && mPath[i] != '\\' && mPath[i] != ':') i--;
	// if (i >= 0) mPath[i] = '\0';
	// else strcpy(mPath, ".");

	// int j = 0;
	// i++;
	// while (i < (int)strlen(filename))
	// 	mName[j++] = filename[i++];
	// mName[j] = '\0';
}

void ObjMeshExt::parseRef(char *s, int nr[])
{
	int i,j,k;
	for (k = 0; k < 3; k++)
		nr[k] = -1;

	int len = (int) strlen(s);
	char is[256]; i = 0;

	for (k = 0; k < 3; k++) {
		j = 0;
		while (i < len && s[i] != '/') {
			is[j] = s[i]; i++; j++;
		}
		i++;
		if (j > 0) {
			is[j] = 0;
			sscanf(is, "%i", &nr[k]);
		}
	}
}

bool ObjMeshExt::loadFromObjFile(char *filename)
{
	FILE *f = fopen(filename, "r");
	if (!f) return false;
	clear();
	ObjMeshString s, subs[maxVerticesPerFace];
	ObjMeshString mtllib, matName;

	mHasTextureCoords = false;
	mHasNormals = false;

	strcpy(mtllib, "");
	int materialNr = -1;
	int i,j;
	NxVec3 v;
	ObjMeshTriangle t;
	TexCoord tc;

	std::vector<NxVec3> centermVertices;
	std::vector<TexCoord> centermTexCoords;
	std::vector<NxVec3> centerNormals;

	extractPath(filename);

	while (!feof(f)) {
		if (fgets(s, OBJ_MESH_STRING_LEN, f) == NULL) break;

		if (strncmp(s, "mtllib", 6) == 0) {  // material library
			sscanf(&s[7], "%s", mtllib);
			// importMtlFile(mtllib);
		}
		else if (strncmp(s, "usemtl", 6) == 0) {  // use material
			materialNr++;
			ObjMeshMaterial material;
			sscanf(&s[7], "%s", material.name);
			material.texNr = materialNr;
			mMaterials.push_back(material);
			// sscanf(&s[7], "%s", matName);
			// materialNr = 0;
			// int numMaterials = (int)mMaterials.size();
			// while (materialNr < numMaterials &&
			// 	   strcasecmp(mMaterials[materialNr].name, matName) != 0)
			// 	materialNr++;
			// if (materialNr >= numMaterials)
			// 	materialNr = -1;
		}
		else if (strncmp(s, "v ", 2) == 0) {	// vertex
			sscanf(s, "v %f %f %f", &v.x, &v.y, &v.z);
			mVertices.push_back(v);
		}
		else if (strncmp(s, "vn ", 3) == 0) {	// normal
			sscanf(s, "vn %f %f %f", &v.x, &v.y, &v.z);
			mNormals.push_back(v);
		}
		else if (strncmp(s, "vt ", 3) == 0) {	// texture coords
			sscanf(s, "vt %f %f", &tc.u, &tc.v);
			mTexCoords.push_back(tc);
		}
		else if (strncmp(s, "f ", 2) == 0) {	// face
			int nr;
			nr = sscanf(s, "f %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s",
			subs[0], subs[1], subs[2], subs[3], subs[4],
			subs[5], subs[6], subs[7], subs[8], subs[9],
			subs[10], subs[11], subs[12],subs[13], subs[14]);
			int vertNr[maxVerticesPerFace], texNr[maxVerticesPerFace];
			int normalNr[maxVerticesPerFace];
			for (i = 0; i < nr; i++) {
				int refs[3];
				parseRef(subs[i], refs);
				vertNr[i] = refs[0]-1;
				texNr[i] = refs[1]-1;
				normalNr[i] = refs[2]-1;
			}
			if (nr <= 4) {	// simple non-singular triangle or quad
				if (vertNr[0] != vertNr[1] && vertNr[1] != vertNr[2] && vertNr[2] != vertNr[0]) {
					t.init();
					t.vertexNr[0] = vertNr[0];
					t.vertexNr[1] = vertNr[1];
					t.vertexNr[2] = vertNr[2];
					t.normalNr[0] = normalNr[0];
					t.normalNr[1] = normalNr[1];
					t.normalNr[2] = normalNr[2];
					t.texCoordNr[0] = texNr[0];
					t.texCoordNr[1] = texNr[1];
					t.texCoordNr[2] = texNr[2];
					t.materialNr = materialNr;
					mTriangles.push_back(t);
				}
				if (nr == 4) {	// non-singular quad -> generate a second triangle
					if (vertNr[2] != vertNr[3] && vertNr[3] != vertNr[0] && vertNr[0] != vertNr[2]) {
						t.init();
						t.vertexNr[0] = vertNr[2];
						t.vertexNr[1] = vertNr[3];
						t.vertexNr[2] = vertNr[0];
						t.normalNr[0] = normalNr[2];
						t.normalNr[1] = normalNr[3];
						t.normalNr[2] = normalNr[0];
						t.texCoordNr[0] = texNr[0];
						t.texCoordNr[1] = texNr[1];
						t.texCoordNr[2] = texNr[2];
						t.materialNr = materialNr;
						mTriangles.push_back(t);
					}
				}
			}
			else {	// polygonal face

				// compute center properties
				NxVec3 centerPos(0.0f, 0.0f, 0.0f);
				TexCoord centerTex; centerTex.zero();
				for (i = 0; i < nr; i++) {
					centerPos += mVertices[vertNr[i]];
					if (texNr[i] >= 0) centerTex += mTexCoords[texNr[i]];
				}
				centerPos /= (float)nr;
				centerTex /= (float)nr;
				NxVec3 d1 = centerPos - mVertices[vertNr[0]];
				NxVec3 d2 = centerPos - mVertices[vertNr[1]];
				NxVec3 centerNormal = d1.cross(d2); centerNormal.normalize();

				// add center vertex
				centermVertices.push_back(centerPos);
				centermTexCoords.push_back(centerTex);
				centerNormals.push_back(centerNormal);

				// add surrounding elements
				for (i = 0; i < nr; i++) {
					j = i+1; if (j >= nr) j = 0;
					t.init();
					t.vertexNr[0] = (int) mVertices.size() + (int) centermVertices.size()-1;
					t.vertexNr[1] = vertNr[i];
					t.vertexNr[2] = vertNr[j];

					t.normalNr[0] = (int) mNormals.size() + (int) centerNormals.size()-1;
					t.normalNr[1] = normalNr[i];
					t.normalNr[2] = normalNr[j];

					t.texCoordNr[0] = (int) mTexCoords.size() + (int) centermTexCoords.size()-1;
					t.texCoordNr[1] = texNr[i];
					t.texCoordNr[2] = texNr[j];
					t.materialNr = materialNr;
					mTriangles.push_back(t);
				}
			}
		}
	}
	std::cout<<"materialNr" << mTexCoords.size() <<"\n";
	fclose(f);

	// new center mVertices are inserted here.
	// If they were inserted when generated, the vertex numbering would be corrupted
	for (i = 0; i < (int)centermVertices.size(); i++)
		mVertices.push_back(centermVertices[i]);
	for (i = 0; i < (int)centerNormals.size(); i++)
		mNormals.push_back(centerNormals[i]);
	for (i = 0; i < (int)centermTexCoords.size(); i++)
		mTexCoords.push_back(centermTexCoords[i]);

	if (mTexCoords.size() > 0) mHasTextureCoords = true;
	if (mNormals.size() > 0)
		mHasNormals = true;
	else
		updateNormals();

	updateBounds();
	return true;
}
