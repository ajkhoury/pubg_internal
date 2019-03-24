#pragma once

#include "Objects.h"

static std::string MakeValidName(std::string&& Name)
{
    std::string ValidString(Name);

    for (size_t i = 0; i < Name.length(); ++i) {
        if (ValidString[i] == ' ' ||
            ValidString[i] == '?' ||
            ValidString[i] == '+' ||
            ValidString[i] == '-' ||
            ValidString[i] == ':' ||
            ValidString[i] == '/' ||
            ValidString[i] == '^' ||
            ValidString[i] == '(' ||
            ValidString[i] == ')' ||
            ValidString[i] == '[' ||
            ValidString[i] == ']' ||
            ValidString[i] == '<' ||
            ValidString[i] == '>' ||
            ValidString[i] == '&' ||
            ValidString[i] == '.' ||
            ValidString[i] == '#' ||
            ValidString[i] == '\'' ||
            ValidString[i] == '"' ||
            ValidString[i] == '%') {
            ValidString[i] = '_';
        }
    }

    if (!ValidString.empty()) {
        if (isdigit(ValidString[0])) {
            ValidString = '_' + ValidString;
        }
    }

    return ValidString;
}

template<typename T>
static std::string MakeUniqueCppNameImpl(const T& t)
{
    std::string NameString;
    if (ObjectsProxy().CountObjects<T>(t.GetName()) > 1) {
        ObjectProxy OuterObj = t.GetOuter();
        NameString = MakeValidName(OuterObj.GetName()) + "_";
    }
    return NameString + MakeValidName(t.GetName());
}

static std::string MakeUniqueCppName(const UEnum* e)
{
    EnumProxy Enum(e);
    std::string NameString = MakeUniqueCppNameImpl(Enum);
    if (!NameString.empty() && NameString[0] != 'E') {
        NameString = 'E' + NameString;
    }
    return NameString;
}

static std::string MakeUniqueCppName(const UStruct* s)
{
    StructProxy Struct(s);
    std::string NameString;
    if (ObjectsProxy().CountObjects<StructProxy>(Struct.GetName()) > 1) {
        NameString = MakeValidName(ObjectProxy(Struct.GetOuter()).GetNameCPP()) + "_";
    }
    return NameString + MakeValidName(Struct.GetNameCPP());
}