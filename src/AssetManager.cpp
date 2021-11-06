#include "AssetManager.hpp"
#include "Logger.hpp"

namespace NovaEngine
{
	bool AssetManager::onInitialize(const char* execPath)
	{
		rootDir_ = Utils::Path::combine(execPath, "assets").string();
		Logger::get()->info("Loaded AssetManager with rootDir: ", rootDir_.c_str());
		return true;
	}

	bool AssetManager::onTerminate()
	{
		return true;
	}


	bool AssetManager::loadFile(const char* assetPath, std::vector<char>& fileContents)
	{
		if (!fileExists(assetPath))
			return false;

		std::string path = createAbsolutePath(assetPath);
		std::ifstream file(path.c_str(), std::ios::binary | std::ios::ate);
		std::streamsize size = file.tellg();

		file.seekg(0, std::ios::beg);
		fileContents.resize(size + 1);


		if (file.read(fileContents.data(), size))
		{
			fileContents.data()[size + 1] = '\0';
			return true;
		}
		return false;
	}

	bool AssetManager::fileExists(const char* assetPath)
	{
		struct stat buffer;
		std::string path = createAbsolutePath(assetPath);
		return stat(path.c_str(), &buffer) == 0;
	}
}
