// ================================================================================
// ==      This file is a part of Turbo Badger. (C) 2011-2014, Emil Segerås      ==
// ==                     See tb_core.h for more information.                    ==
// ================================================================================

#include <tb/tb_system.h>
#include <physfs.h>
#include <starbase/game/logging.hpp>

namespace {

class PhysFSFile : public tb::TBFile
{
public:
	PhysFSFile(PHYSFS_File *f) : file(f) {}
	virtual ~PhysFSFile()
	{
		PHYSFS_close(file);
	}

	virtual long Size()
	{
		return static_cast<long>(PHYSFS_fileLength(file));
	}

	virtual size_t Read(void *buf, size_t elemSize, size_t count)
	{
		return PHYSFS_read(file, buf, (PHYSFS_uint32)elemSize, (PHYSFS_uint32)count);
	}
private:
	PHYSFS_File* file;
};

} // namespace

namespace tb {

// static
TBFile *TBFile::Open(const char *filename, TBFileMode mode)
{
	PHYSFS_File *f = nullptr;
	switch (mode)
	{
	case MODE_READ:
		f = PHYSFS_openRead(filename);
		break;
	default:
		break;
	}

	if (!f)
		return nullptr;

	PhysFSFile *tbf = new PhysFSFile(f);
	if (!tbf)
		PHYSFS_close(f);

	return tbf;
}

} // namespace tb
