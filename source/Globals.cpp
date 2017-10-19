#include <Globals.hpp>
#include <atomic>

std::atomic<std::intmax_t> GlobalReferences = 0;

std::intmax_t Globals::ReferenceAdd()
{
	return ++GlobalReferences;
}

std::intmax_t Globals::ReferenceRelease()
{
	return --GlobalReferences;
}

std::intmax_t Globals::ReferenceGet()
{
	return GlobalReferences;
}
