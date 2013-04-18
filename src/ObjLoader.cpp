#include "ObjLoader.h"

ObjLoader::ObjLoader(const Ogre::String& path, Ogre::SceneManager* mSceneMgr)
{
	this->mSceneMgr = mSceneMgr;

	Ogre::String ext, outPath;
	Ogre::StringUtil::splitFullFilename(path, name, ext, outPath);

	Ogre::FileHandleDataStream *stream = new Ogre::FileHandleDataStream(fopen(path.c_str(), "rb"));
	HandleStream(stream);
}

ObjLoader::~ObjLoader()
{
}

void ObjLoader::HandleStream(Ogre::FileHandleDataStream* stream)
{
	obj = this->mSceneMgr->createManualObject(name);

	std::vector<Ogre::Vector3> vertices, normals;
	std::vector<Ogre::Vector2> textureCoords;

	Ogre::uint32 count = 0;

	while (!stream->eof())
	{
		Ogre::String str = stream->getLine();

		if (Ogre::StringUtil::startsWith(str, "usemtl "))
		{
			if (obj->getNumSections() != 0)
				obj->end();
			obj->begin(str.substr(7));
		}
		else if (Ogre::StringUtil::startsWith(str, "v "))
		{
			vertices.push_back(Ogre::StringConverter::parseVector3(str.substr(2)));
		}
		else if (Ogre::StringUtil::startsWith(str, "vn "))
		{
			normals.push_back(Ogre::StringConverter::parseVector3(str.substr(3)));
		}
		else if (Ogre::StringUtil::startsWith(str, "vt "))
		{
			textureCoords.push_back(Ogre::StringConverter::parseVector2(str.substr(3)));
		}
		else if (Ogre::StringUtil::startsWith(str, "f "))
		{
			Ogre::StringVector vec = Ogre::StringUtil::tokenise(str.substr(2), " ", "/");

			if (vec.size() != 9)
				exit(0);

			Ogre::uint32 Indices[3][3];

			for (Ogre::uint32 i = 0; i < 3; i++)
				for (Ogre::uint32 j = 0; j < 3; j++)
					Indices[i][j] = Ogre::StringConverter::parseUnsignedInt(vec[i * 3 + j]) - 1;

			vec.swap(Ogre::StringVector());

			for (Ogre::uint32 i = 0; i < 3; i++)
			{
				obj->position(vertices[Indices[i][0]]);
				obj->textureCoord(textureCoords[Indices[i][1]]);
				obj->normal(normals[Indices[i][2]]);
			}

			obj->index(count++);
			obj->index(count++);
			obj->index(count++);
		}
	}
	if (obj->getNumSections() != 0)
		obj->end();

	vertices.swap(std::vector<Ogre::Vector3>());
	normals.swap(std::vector<Ogre::Vector3>());
	textureCoords.swap(std::vector<Ogre::Vector2>());

	stream->close();
}
