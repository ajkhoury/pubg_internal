#pragma once

#include "Types.h"

#include <string>

class UEnum;
class UStruct;

namespace fmt {

std::string MakeValidName(const std::string&& Name);
std::string MakeUniqueEnumCppName(UEnum const* e);
std::string MakeUniqueStructCppName(UStruct const* s);

std::string StringifyFunctionFlags(const uint32_t Flags);
std::string StringifyPropertyFlags(const uint64_t Flags);

}

