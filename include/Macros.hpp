#pragma once

#define EXPORT __declspec(dllexport)

template< typename T, std::size_t N >
constexpr std::size_t countof(T const (&)[N]) noexcept
{
	return N;
}
