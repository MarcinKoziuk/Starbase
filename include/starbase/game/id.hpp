#pragma once

// Extracted and adapted from Turbo Badger's TBID
// (C) 2011-2014, Emil Segerås

#include <cstdint>

namespace Starbase {

// FNV constants
static constexpr std::uint32_t basis = 2166136261U;
static constexpr std::uint32_t prime = 16777619U;

// compile-time hash helper function
static constexpr std::uint32_t IDC_one(char c, const char* remain, std::uint32_t value)
{
	return c == 0 ? value : IDC_one(remain[0], remain + 1, (value ^ c) * prime);
}

// compile-time hash
static constexpr std::uint32_t IDC(const char* str)
{
	return (str && *str) ? IDC_one(str[0], str + 1, basis) : 0;
}

#ifndef NDEBUG

// Debug (non compile-time constant) version of ID, which checks for
// hash collisions.
class IDD {
public:
	IDD(std::uint32_t id = 0) : id(id) {}
	IDD(const IDD &id) : id(id) {}
	IDD(const char *string) { Set(string); }

	void Set(const char *string);

	operator std::uint32_t () const { return id; }
	const IDD& operator= (const IDD &id) { this->id = id; return *this; }

private:
	std::uint32_t id;
};

static std::uint32_t ID(const char* str)
{
	return IDD(str);
}

#else

static constexpr std::uint32_t ID(const char* str)
{
	return IDC(str);
}

#endif; // NDEBUG

} // namespace Starbase
