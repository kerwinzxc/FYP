#include "PhysXSoftBody.h"

PhysXSoftBody::PhysXSoftBody(NxScene *scene, Ogre::SceneManager* sceneMgr, NxSoftBodyDesc &desc,
                             char* objFilePath)
	: mInitDone(false), mScene(scene), mSoftBodyMesh(NULL), mSoftBody(NULL),
	  mSceneMgr(sceneMgr), mSoftBodyNode(NULL), mEntity(NULL)
{
	mObjMesh = new ObjMeshExt();
	mObjMesh->loadFromObjFile(objFilePath);
	mVertexOffsets.resize(mObjMesh->getNumMaterials()); mVertexOffsets.clear();

	NxSoftBodyMeshDesc meshDesc;
	saveMeshDesc(meshDesc);
	init(desc, meshDesc);
	initEntity();
}

PhysXSoftBody::~PhysXSoftBody()
{
	if (mInitDone)
	{
		if (mSoftBody)
			mScene->releaseSoftBody(*mSoftBody);
		if (mSoftBodyMesh)
			mScene->getPhysicsSDK().releaseSoftBodyMesh(*mSoftBodyMesh);
		releaseReceiveBuffers();
	}
	if (mObjMesh)
		delete mObjMesh;
	mVertexOffsets.swap(std::vector<int>());
}

bool PhysXSoftBody::saveMeshDesc(NxSoftBodyMeshDesc &desc)
{
	NxArray<NxVec3>* vertices    = new NxArray<NxVec3>(mObjMesh->getNumVertices());
	NxArray<NxU32>*  tetrahedras = new NxArray<NxU32>(mObjMesh->getNumTriangles());

	loadTetFile(vertices, tetrahedras);

	NxU32 vertexCount = vertices->size();
	NxU32 indexCount  = tetrahedras->size();
	NxU32 tetCount    = indexCount / 4;

	desc.numVertices            = vertexCount;
	desc.numTetrahedra          = tetCount;
	desc.vertexStrideBytes      = sizeof(NxVec3);
	desc.tetrahedronStrideBytes = 4 * sizeof(NxU32);
	desc.vertexMassStrideBytes  = sizeof(NxReal);
	desc.vertexFlagStrideBytes  = sizeof(NxU32);
	desc.vertices               = (NxVec3*)malloc(sizeof(NxVec3) * vertexCount);
	desc.tetrahedra             = (NxU32*) malloc(sizeof(NxU32)  * indexCount);
	desc.vertexMasses           = 0;
	desc.vertexFlags            = 0;
	desc.flags                  = 0;

	NxVec3* vSrc  = (NxVec3*)vertices->begin();
	NxVec3* vDest = (NxVec3*)desc.vertices;
	for (NxU32 i = 0; i < vertexCount; i++, ++vSrc, ++vDest)
		*vDest = (*vSrc);
	NxU32* tSrc  = (NxU32*)tetrahedras->begin();
	NxU32* tDest = (NxU32*)desc.tetrahedra;
	for (NxU32 i = 0; i < indexCount; i++, ++tSrc, ++tDest)
		*tDest = (*tSrc);

	delete vertices;
	delete tetrahedras;

	return true;
}

bool PhysXSoftBody::loadTetFile(NxArray<NxVec3>* vertices, NxArray<NxU32>* tetrahedras)
{
	if (!mObjMesh)
		return false;

	vertices->clear();
	tetrahedras->clear();

	std::string filepath;
	filepath.append(mObjMesh->getPath());
	filepath.append(mObjMesh->getName());
	filepath.append(".tet");

	std::ifstream f(filepath);
	std::string line;

	NxVec3 vertex;
	Ogre::StringVector tetrahedra;
	while (std::getline(f, line))
	{
		if (line.compare(0, 2, "v ") == 0)
		{
			sscanf(line.c_str(), "v %f %f %f", &vertex.x, &vertex.y, &vertex.z);
			vertices->push_back(vertex);
		}
		else if (line.compare(0, 2, "t ") == 0)
		{
			tetrahedra = Ogre::StringConverter::parseStringVector(line.substr(2));
			tetrahedras->push_back(atoi(tetrahedra[0].c_str()));
			tetrahedras->push_back(atoi(tetrahedra[1].c_str()));
			tetrahedras->push_back(atoi(tetrahedra[2].c_str()));
			tetrahedras->push_back(atoi(tetrahedra[3].c_str()));
			tetrahedra.swap(Ogre::StringVector());
		}
	}

	f.close();

	vertices->resize(vertices->size());
	tetrahedras->resize(tetrahedras->size());

	return true;
}

void PhysXSoftBody::init(NxSoftBodyDesc &desc, NxSoftBodyMeshDesc &meshDesc)
{
	mObjMesh->buildTetraLinks((NxVec3*)meshDesc.vertices, (NxU32*)meshDesc.tetrahedra,
	                          meshDesc.numTetrahedra);
	allocateReceiveBuffers(meshDesc.numVertices, meshDesc.numTetrahedra);

	cookMesh(meshDesc);
	releaseMeshDescBuffers(meshDesc);

	desc.softBodyMesh = mSoftBodyMesh;
	desc.meshData = mReceiveBuffers;
	mSoftBody = mScene->createSoftBody(desc);

	mInitDone = true;
}

bool PhysXSoftBody::cookMesh(NxSoftBodyMeshDesc& desc)
{
	MemoryWriteBuffer wb;
	assert(desc.isValid());

	if (!NxCookSoftBodyMesh(desc, wb))
		return false;

	MemoryReadBuffer rb(wb.data);
	mSoftBodyMesh = mScene->getPhysicsSDK().createSoftBodyMesh(rb);
	return true;
}

void PhysXSoftBody::releaseMeshDescBuffers(const NxSoftBodyMeshDesc& desc)
{
	NxVec3* y = (NxVec3*)desc.vertices;
	NxU32*  t = (NxU32*) desc.tetrahedra;
	NxReal* m = (NxReal*)desc.vertexMasses;
	NxU32*  z = (NxU32*) desc.vertexFlags;
	free(y);
	free(t);
	free(m);
	free(z);
}

void PhysXSoftBody::allocateReceiveBuffers(NxU32 numVertices, NxU32 numTetrahedra)
{
	// here we setup the buffers through which the SDK returns the dynamic softbody data
	// we reserve more memory for vertices than the initial mesh takes
	// because tearing creates new vertices
	// the SDK only tears softbodies as long as there is room in these buffers

	NxU32 maxVertices = 2 * numVertices;
	mReceiveBuffers.verticesPosBegin = (NxVec3*) malloc(sizeof(NxVec3) * maxVertices);
	mReceiveBuffers.verticesPosByteStride = sizeof(NxVec3);
	mReceiveBuffers.maxVertices = maxVertices;
	mReceiveBuffers.numVerticesPtr = (NxU32*) malloc(sizeof(NxU32));

	mReceiveBuffers.verticesNormalBegin = new NxVec3[maxVertices];
	mReceiveBuffers.verticesNormalByteStride = sizeof(NxVec3);

	// the number of tetrahedra is constant, even if the softbody is torn
	NxU32 maxIndices = 4 * numTetrahedra;
	mReceiveBuffers.indicesBegin = (NxU32*) malloc(sizeof(NxU32) * maxIndices);
	mReceiveBuffers.indicesByteStride = sizeof(NxU32);
	mReceiveBuffers.maxIndices = maxIndices;
	mReceiveBuffers.numIndicesPtr = (NxU32*) malloc(sizeof(NxU32));

	// init the buffers in case we want to draw the mesh
	// before the SDK as filled in the correct values
	*mReceiveBuffers.numVerticesPtr = 0;
	*mReceiveBuffers.numIndicesPtr = 0;
}

void PhysXSoftBody::releaseReceiveBuffers()
{
	// Parent Indices is always allocated
	free(mReceiveBuffers.parentIndicesBegin);
	mReceiveBuffers.setToDefault();
}

void PhysXSoftBody::initEntity()
{
	Ogre::ManualObject *manualObject;
	if (mSceneMgr != NULL)
		manualObject = mSceneMgr->createManualObject(mObjMesh->getName());

	int vertexCount    = mObjMesh->getNumVertices();
	int triangleCount  = mObjMesh->getNumTriangles();

	assert(vertexCount >= 3);

	std::vector<int> triOffsets;
	triOffsets.resize(mObjMesh->getNumMaterials());    triOffsets.clear();

	int prevMat = mObjMesh->getTriangle(0).materialNr;
	int max = -1;
	for (int j = 0; j < triangleCount; j++)
	{
		ObjMeshTriangle tri = mObjMesh->getTriangle(j);
		if (prevMat == tri.materialNr)
		{
			if (*std::max_element(tri.vertexNr, tri.vertexNr + 3) > max)
				max = *std::max_element(tri.vertexNr, tri.vertexNr + 3);
		}
		else
		{
			prevMat = tri.materialNr;
			mVertexOffsets.push_back(max);
			triOffsets.push_back(j - 1);
			max = *std::max_element(tri.vertexNr, tri.vertexNr + 3);
		}
	}
	mVertexOffsets.push_back(max);
	triOffsets.push_back(triangleCount - 1);

	int j = 0, k = 0, offset = 0;
	for (size_t i = 0; i < mVertexOffsets.size(); ++i)
	{
		manualObject->estimateVertexCount(mVertexOffsets[i] - j + 1);
		manualObject->estimateIndexCount((triOffsets[i] - k + 1) * 3);

		ObjMeshMaterial material = mObjMesh->getMaterial(i);
		manualObject->begin(material.name, Ogre::RenderOperation::OT_TRIANGLE_LIST);

		for (; j <= mVertexOffsets[i]; j++)
		{
			NxVec3 vertex = mObjMesh->getVertex(j);
			manualObject->position(vertex.x, vertex.y, vertex.z);
			NxVec3 normal = mObjMesh->getNormal(j);
			manualObject->normal(normal.x, normal.y, normal.z);
			TexCoord texCoord = mObjMesh->getTexCoord(j);
			manualObject->textureCoord(texCoord.u, texCoord.v);
		}
		for (; k <= triOffsets[i]; k++)
		{
			ObjMeshTriangle tri = mObjMesh->getTriangle(k);
			manualObject->triangle(tri.vertexNr[0] - offset, tri.vertexNr[1] - offset, tri.vertexNr[2] - offset);
		}
		offset = mVertexOffsets[i] + 1;

		manualObject->end();
	}

	mEntity = mSceneMgr->createEntity(manualObject->convertToMesh(manualObject->getName()));
	mSoftBodyNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	mSoftBodyNode->attachObject(mEntity);

	mSceneMgr->destroyManualObject(manualObject);
}

void PhysXSoftBody::render()
{
	mObjMesh->simulateMesh(mReceiveBuffers);

	int j = 0;
	for (size_t i = 0; i < mVertexOffsets.size(); ++i)
	{
		Ogre::SubMesh* submesh = mEntity->getMesh()->getSubMesh(i);
		Ogre::VertexData* vertexData = submesh->vertexData;
		const Ogre::VertexElement* posElem = vertexData->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);
		Ogre::HardwareVertexBufferSharedPtr vbuf = vertexData->vertexBufferBinding->getBuffer(posElem->getSource());
		unsigned char* ver = static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_NORMAL));
		size_t offset = vbuf->getVertexSize();
		Ogre::Real* pReal;

		for (; j <= mVertexOffsets[i]; j++, ver += offset)
		{
			NxVec3 vertex = mObjMesh->getVertex(j);

			posElem->baseVertexPointerToElement(ver, &pReal);
			pReal[0] = vertex.x;
			pReal[1] = vertex.y;
			pReal[2] = vertex.z;
		}
		vbuf->unlock();
	}
}
