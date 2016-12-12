#include <cstdlib>
#include <cassert>
#include <algorithm>

#include <physfs.h>
#include <physfs.hpp>

#include <starbase/starbase.hpp>
#include <starbase/game/logging.hpp>
#include <starbase/game/fs/filesystem_physfs.hpp>

// not thread safe, just a safeguard
static int instanceCount = 0;

static Starbase::FilesystemPhysFS* instance = nullptr;

namespace Starbase {

FilesystemPhysFS::FilesystemPhysFS()
	: m_isInitialized(false)
{
	instanceCount++;
	if (instanceCount != 1) {
		LOG(fatal) << "Only a single FilesystemPhysFS can exist at a time!";
		std::abort();
	}
	::instance = this;
}

FilesystemPhysFS::~FilesystemPhysFS()
{
	instanceCount--;
}

FilesystemPhysFS& FilesystemPhysFS::instance()
{
	return *::instance;
}

bool FilesystemPhysFS::Init()
{
	int result = PHYSFS_init(STARBASE_NAME);
	if (!result) {
		LOG(error) << "PhysFS initialization failed";
		return false;
	}

	int mounts = 0;
	mounts += (PHYSFS_mount("D:\\Dropbox\\Projects\\starbase\\data", "/", true) != 0);
	mounts += (PHYSFS_mount("../starbase/data", "/", true) != 0);
	mounts += (PHYSFS_mount("./data", "/", true) != 0);
	//mounts += (PHYSFS_mount("./Debug/data", "/", true) != null); //?
	//mounts += (PHYSFS_mount("./Release/data", "/", true) != null); //?

	if (!mounts) {
		LOG(error) << "Mounting of data directory failed";
		return false;
	}
	else {
		LOG(info) << "Number of PhysFS mounts: " << mounts;
	}

	LOG(trace) << "Currently mounted search paths are:";

	const std::vector<std::string> paths = PhysFS::getSearchPath();
	for (const std::string& path : paths) {
		LOG(trace) << path;
	}

	m_isInitialized = true;
	return true;
}

void FilesystemPhysFS::Shutdown()
{
	int result = PHYSFS_deinit();
	if (!result) {
		LOG(error) << "Failed to cleanly shut down PhysFS";
	}
}

std::size_t FilesystemPhysFS::GetLength(const std::string& path)
{
	assert(m_isInitialized);

	PHYSFS_File* file = PHYSFS_openRead(path.c_str());
	if (file) {
		return PHYSFS_fileLength(file);
	}
	else {
		return 0L;
	}
}

bool FilesystemPhysFS::ReadBytes(const std::string& path, std::vector<std::uint8_t>* destVector)
{
	assert(m_isInitialized);

	PHYSFS_File* file = PHYSFS_openRead(path.c_str());
	if (file) {
		std::size_t length = PHYSFS_fileLength(file);
		destVector->reserve(length);
		destVector->resize(length);

		PHYSFS_read(file, &(*destVector)[0], 1, length);
		PHYSFS_close(file);

		return true;
	}
	else {
		LOG(error) << "Could not open: " << path;
		return false;
	}
}

bool FilesystemPhysFS::ReadString(const std::string& path, std::string* destString)
{
	assert(m_isInitialized);

	PHYSFS_File* file = PHYSFS_openRead(path.c_str());
	if (file) {
		std::size_t length = PHYSFS_fileLength(file);
		destString->reserve(length);
		destString->resize(length);

		// TODO: check 0 termination and optimize reserve() if needed
		PHYSFS_read(file, &(*destString)[0], 1, length);
		PHYSFS_close(file);

		return true;
	}
	else {
		LOG(error) << "Could not open: " << path;
		return false;
	}
}

std::unique_ptr<std::istream> FilesystemPhysFS::OpenAsStream(const std::string& path)
{
	assert(m_isInitialized);

	PHYSFS_File* file = PHYSFS_openRead(path.c_str());
	if (file) {
		return std::unique_ptr<PhysFS::ifstream>(new PhysFS::ifstream(file));
	}
	else {
		LOG(error) << "Could not open: " << path;
		return nullptr;
	}
}

} // namespace Starbase