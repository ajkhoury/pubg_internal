#pragma once

#include "Objects.h"
#include "UnrealTypes.h"

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
static std::string MakeUniqueCppNameImpl(const T* t)
{
    std::string NameString;
    if (ObjectsProxy().CountObjects<T>(t->GetName()) > 1) {
        const UObject* OuterObj = t->GetOuter();
        if (OuterObj != nullptr) {
            NameString = MakeValidName(OuterObj->GetName()) + '_';
        }
    }
    return NameString + MakeValidName(t->GetName());
}

static std::string MakeUniqueCppName(const UEnum* e)
{
    std::string NameString = MakeUniqueCppNameImpl(e);
    if (!NameString.empty() && NameString[0] != 'E') {
        NameString = 'E' + NameString;
    }
    return NameString;
}

static std::string MakeUniqueCppName(const UStruct* s)
{
    std::string NameString;
    if (ObjectsProxy().CountObjects<UStruct>(s->GetName()) > 1) {
        const UObject* OuterObj = s->GetOuter();
        if (OuterObj != nullptr) {
            NameString = MakeValidName(OuterObj->GetNameCPP()) + '_';
        }
    }
    return NameString + MakeValidName(s->GetNameCPP());
}