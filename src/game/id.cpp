#include <cassert>
#include <mutex>
#include <unordered_map>

#include <starbase/game/id.hpp>

// Extracted and adapted from Turbo Badger's TBID
// (C) 2011-2014, Emil Segerås

#ifndef NDEBUG

namespace Starbase {

// Hash table for checking if we get any collisions (same hash value for IDs created
// from different strings)
static std::unordered_map<id_t, std::string> g_hashed_strings;
static std::mutex g_hashed_strings_m;

void IDD::Set(const char *string)
{
	id = IDC(string);

	g_hashed_strings_m.lock();

	if (g_hashed_strings.count(id))
		assert(g_hashed_strings[id] == string);
	else
		g_hashed_strings[id] = string;

	g_hashed_strings_m.unlock();
}

} // namespae Starbase

#endif // NDEBUG
