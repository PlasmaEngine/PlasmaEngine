// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

Lightning::Function* GetMemberOverloadedFunction(Lightning::Type* type, StringParam fnName);
Lightning::Function* GetMemberOverloadedFunction(Lightning::Type* type, StringParam fnName, StringParam p0);
Lightning::Function* GetMemberOverloadedFunction(Lightning::Type* type, StringParam fnName, StringParam p0, StringParam p1);
Lightning::Function*
GetMemberOverloadedFunction(Lightning::Type* type, StringParam fnName, StringParam p0, StringParam p1, StringParam p2);
Lightning::Function* GetMemberOverloadedFunction(
    Lightning::Type* type, StringParam fnName, StringParam p0, StringParam p1, StringParam p2, StringParam p3);

Lightning::Function* GetStaticFunction(Lightning::Type* type, StringParam fnName);
Lightning::Function* GetStaticFunction(Lightning::Type* type, StringParam fnName, StringParam p0);
Lightning::Function* GetStaticFunction(Lightning::Type* type, StringParam fnName, StringParam p0, StringParam p1);
Lightning::Function*
GetStaticFunction(Lightning::Type* type, StringParam fnName, StringParam p0, StringParam p1, StringParam p2);
Lightning::Function* GetStaticFunction(
    Lightning::Type* type, StringParam fnName, StringParam p0, StringParam p1, StringParam p2, StringParam p3);
Lightning::Function* GetStaticFunction(Lightning::Type* type,
                                   StringParam fnName,
                                   StringParam p0,
                                   StringParam p1,
                                   StringParam p2,
                                   StringParam p3,
                                   StringParam p4);

Lightning::Function* GetConstructor(Lightning::Type* type, StringParam p0);
Lightning::Function* GetConstructor(Lightning::Type* type, Array<String>& params);

Lightning::Field* GetStaticMember(Lightning::Type* type, StringParam memberName);
Lightning::Property* GetInstanceProperty(Lightning::Type* type, StringParam propName);
Lightning::Property* GetStaticProperty(Lightning::Type* type, StringParam propName);

Array<String> BuildParams(StringParam p0);
Array<String> BuildParams(StringParam p0, StringParam p1);
Array<String> BuildParams(StringParam p0, StringParam p1, StringParam p2);
Array<String> BuildParams(StringParam p0, StringParam p1, StringParam p2, StringParam p3);

String JoinRepeatedString(StringParam str, StringParam separator, size_t count);

} // namespace Plasma
