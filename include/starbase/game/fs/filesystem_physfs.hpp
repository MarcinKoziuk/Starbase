#pragma once

#include <starbase/game/fs/ifilesystem.hpp>

namespace Starbase {

class FilesystemPhysFS : public IFilesystem {
private:
	bool m_isInitialized;

public:
	FilesystemPhysFS();
	virtual ~FilesystemPhysFS();

	static FilesystemPhysFS& instance();

	virtual bool Init();
	virtual void Shutdown();
	virtual std::size_t GetLength(const std::string& path);
	virtual bool ReadBytes(const std::string& path, std::vector<std::uint8_t>* destVector);
	virtual bool ReadString(const std::string& path, std::string* destString);
	virtual std::unique_ptr<std::istream> OpenAsStream(const std::string& path);
};

} // namespace Starbase