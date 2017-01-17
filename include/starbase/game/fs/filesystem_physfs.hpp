#pragma once

#include <unordered_map>

#include <starbase/game/fs/ifilesystem.hpp>

namespace Starbase {

class FilesystemPhysFS : public IFilesystem {
private:
	bool m_isInitialized;
	std::unordered_map<id_t, std::string> m_idPaths;

	void ReloadIdPaths();
	static void PathEnumCallback(void* self_, const char* rootDir, const char* path);

public:
	FilesystemPhysFS();
	virtual ~FilesystemPhysFS();

	static FilesystemPhysFS& instance();

	virtual bool Init();

	virtual void Shutdown();

	virtual std::size_t GetLength(id_t id);

	virtual std::size_t GetLength(const std::string& path);

	virtual bool ReadBytes(id_t id, std::vector<std::uint8_t>* destVector);

	virtual bool ReadBytes(const std::string& path, std::vector<std::uint8_t>* destVector);

	virtual bool ReadString(id_t id, std::string* destString);

	virtual bool ReadString(const std::string& path, std::string* destString);

	virtual std::unique_ptr<std::istream> OpenAsStream(id_t id);

	virtual std::unique_ptr<std::istream> OpenAsStream(const std::string& path);

	virtual bool GetPathForId(id_t id, std::string& dest);
};

} // namespace Starbase