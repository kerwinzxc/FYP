#include "PhysXSoftBody.h"

PhysXSoftBody::PhysXSoftBody(NxScene *scene, Ogre::SceneManager* sceneMgr, NxSoftBodyDesc &desc,
                             char* objFilePath)
	: mInitDone(false), mScene(scene), mSoftBodyMesh(NULL), mSoftBody(NULL),
	  mSceneMgr(sceneMgr), mEntity(NULL)
{
	mObjMesh = new ObjMeshExt();
	mObjMesh->loadFromObjFile(objFilePath);

	NxSoftBodyMeshDesc meshDesc;
	saveMeshDesc(meshDesc, objFilePath);
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
}

bool PhysXSoftBody::saveMeshDesc(NxSoftBodyMeshDesc &desc, char* path)
{
	NxArray<NxVec3>* vertices    = new NxArray<NxVec3>(mObjMesh->getNumVertices());
	NxArray<NxU32>*  tetrahedras = new NxArray<NxU32>(mObjMesh->getNumTriangles());

	loadTetFile(path, vertices, tetrahedras);

	NxU32 vertexCount = vertices->size();
	NxU32 tetCount    = tetrahedras->size() / 4;

	desc.numVertices            = vertexCount;
	desc.numTetrahedra          = tetCount;
	desc.vertexStrideBytes      = sizeof(NxVec3);
	desc.tetrahedronStrideBytes = 3 * sizeof(NxU32);
	desc.vertexMassStrideBytes  = sizeof(NxReal);
	desc.vertexFlagStrideBytes  = sizeof(NxU32);
	desc.vertices               = (NxVec3*)malloc(sizeof(NxVec3) * vertexCount);
	desc.tetrahedra             = (NxU32*) malloc(sizeof(NxU32)  * tetCount * 4);
	desc.vertexMasses           = 0;
	desc.vertexFlags            = 0;
	desc.flags                  = 0;

	memcpy((NxVec3*)desc.vertices,   vertices->begin(),    sizeof(NxU32) * vertexCount);
	memcpy((NxU32*) desc.tetrahedra, tetrahedras->begin(), sizeof(NxU32) * tetCount * 4);

	delete vertices;
	delete tetrahedras;

	return true;
}

bool PhysXSoftBody::loadTetFile(char* path, NxArray<NxVec3>* vertices, NxArray<NxU32>* tetrahedras)
{
	if (!mObjMesh)
		return false;

	vertices->clear();
	tetrahedras->clear();

	std::string tetpath;
	tetpath.append(path, strlen(path) - 4);
	tetpath.append(".tet");

	std::ifstream f(tetpath);
	std::string line;

	NxVec3 vertex;
	Ogre::Vector4 tetrahedra;
	while (std::getline(f, line))
	{
		if (line.compare(0, 2, "v ") == 0)
		{
			sscanf(line.substr(2).c_str(), "%f %f %f", &vertex.x, &vertex.y, &vertex.z);
			vertices->push_back(vertex);
		}
		else if (line.compare(0, 2, "t ") == 0)
		{
			sscanf(line.substr(2).c_str(), "%d %d %d %d", &tetrahedra.w, &tetrahedra.x,
			                                              &tetrahedra.y, &tetrahedra.z);
			tetrahedras->push_back(tetrahedra.w);
			tetrahedras->push_back(tetrahedra.x);
			tetrahedras->push_back(tetrahedra.y);
			tetrahedras->push_back(tetrahedra.z);
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

	std::vector<int> vertexOffsets;
	std::vector<int> triOffsets;
	vertexOffsets.resize(mObjMesh->getNumMaterials()); vertexOffsets.clear();
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
			vertexOffsets.push_back(max);
			triOffsets.push_back(j - 1);
			max = *std::max_element(tri.vertexNr, tri.vertexNr + 3);
		}
	}
	vertexOffsets.push_back(max);
	triOffsets.push_back(triangleCount - 1);

	int j = 0, k = 0, offset = 0;
	for (size_t i = 0; i < vertexOffsets.size(); ++i)
	{
		manualObject->estimateVertexCount(vertexOffsets[i] - j + 1);
		manualObject->estimateIndexCount((triOffsets[i] - k + 1) * 3);

		ObjMeshMaterial material = mObjMesh->getMaterial(i);
		manualObject->begin(material.name, Ogre::RenderOperation::OT_TRIANGLE_LIST);

		for (; j <= vertexOffsets[i]; j++)
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
		offset = vertexOffsets[i] + 1;

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

	std::vector<int> vertexOffsets;
	vertexOffsets.resize(mObjMesh->getNumMaterials()); vertexOffsets.clear();

	int prevMat = mObjMesh->getTriangle(0).materialNr;
	int max = -1;
	for (int j = 0; j < mObjMesh->getNumTriangles(); j++)
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
			vertexOffsets.push_back(max);
			max = *std::max_element(tri.vertexNr, tri.vertexNr + 3);
		}
	}
	vertexOffsets.push_back(max);

	int j = 0;
	for (size_t i = 0; i < vertexOffsets.size(); ++i)
	{
		Ogre::SubMesh* submesh = mEntity->getMesh()->getSubMesh(i);
		Ogre::VertexData* vertexData = submesh->vertexData;
		const Ogre::VertexElement* posElem = vertexData->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);
		Ogre::HardwareVertexBufferSharedPtr vbuf = vertexData->vertexBufferBinding->getBuffer(posElem->getSource());
		unsigned char* ver = static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_NORMAL));
		size_t offset = vbuf->getVertexSize();
		Ogre::Real* pReal;

		for (; j <= vertexOffsets[i]; j++, ver += offset)
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
