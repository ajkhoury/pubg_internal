#pragma once

#include <cstdint>
#include <cstddef>
#include <memory>

class VmtHook
{
private:
	uintptr_t **BaseClass = nullptr;
	uintptr_t *OriginalVftable = nullptr;
	std::unique_ptr< std::uintptr_t[] > CurrentVftable = nullptr;

	std::size_t MaxFunctions = 0;

public:
	VmtHook() = delete;

	explicit VmtHook( void *BaseClass );
	~VmtHook();

	template < typename Fn = void * > inline const Fn GetOriginalFunction( std::size_t FunctionIdx )
	{
		return reinterpret_cast< Fn >( this->OriginalVftable[ FunctionIdx ] );
	}

	bool InsertHook( void *NewFn, const std::size_t FunctionIdx );
	bool RemoveHook( const std::size_t FunctionIdx );

	std::size_t GetMaxFunctions();
};