#include "vmt.hpp"

VmtHook::VmtHook( void *BaseClass )
{
	this->BaseClass = static_cast< uintptr_t ** >( BaseClass );

	while ( static_cast< uintptr_t * >( *this->BaseClass )[ this->MaxFunctions ] )
		++this->MaxFunctions;

	const std::size_t TableSize = this->MaxFunctions * sizeof( std::uintptr_t );

	this->OriginalVftable = *this->BaseClass;
	this->CurrentVftable = std::make_unique< std::uintptr_t[] >( this->MaxFunctions );

	std::memcpy( this->CurrentVftable.get(), this->OriginalVftable, TableSize );

	*this->BaseClass = this->CurrentVftable.get();
}

VmtHook::~VmtHook()
{
	*this->BaseClass = this->OriginalVftable;
}

bool VmtHook::InsertHook( void *NewFn, const std::size_t FunctionIdx )
{
	if ( FunctionIdx > this->MaxFunctions )
		return false;

	this->CurrentVftable[ FunctionIdx ] = reinterpret_cast< std::uintptr_t >( NewFn );

	return true;
}

bool VmtHook::RemoveHook( const std::size_t FunctionIdx )
{
	if ( FunctionIdx > this->MaxFunctions )
		return false;

	this->CurrentVftable[ FunctionIdx ] = this->OriginalVftable[ FunctionIdx ];

	return true;
}

std::size_t VmtHook::GetMaxFunctions()
{
	return this->MaxFunctions;
}
