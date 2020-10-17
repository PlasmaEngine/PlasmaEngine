// MIT Licensed (see LICENSE.md).

#pragma once
#ifndef LIGHTNING_STRING_CONSTANTS_HPP
#  define LIGHTNING_STRING_CONSTANTS_HPP

namespace Lightning
{
// Constants
PlasmaShared extern const String ThisKeyword;
PlasmaShared extern const String ValueKeyword;
PlasmaShared extern const String PreConstructorName;
PlasmaShared extern const String ConstructorName;
PlasmaShared extern const String DestructorName;
PlasmaShared extern const String FieldInitializerName;
PlasmaShared extern const String ExpressionLibrary;
PlasmaShared extern const String ExpressionProgram;
PlasmaShared extern const String ExpressionMain;
PlasmaShared extern const String PropertyDelegateName;
PlasmaShared extern const String StaticAttribute;
PlasmaShared extern const String OverrideAttribute;
PlasmaShared extern const String VirtualAttribute;
PlasmaShared extern const String HiddenAttribute;
PlasmaShared extern const String ExtensionAttribute;
PlasmaShared extern const String PropertyAttribute;
PlasmaShared extern const String InternalAttribute;
PlasmaShared extern const String DeprecatedAttribute;
PlasmaShared extern const String ExportDocumentation;
PlasmaShared extern const String ImportDocumentation;
PlasmaShared extern const String CodeString;
PlasmaShared extern const String ExpressionInitializerLocal;
PlasmaShared extern const String OperatorInsert;
PlasmaShared extern const String OperatorGet;
PlasmaShared extern const String OperatorSet;
PlasmaShared extern const String UnknownOrigin;
PlasmaShared extern const String EmptyLowerIdentifier;
PlasmaShared extern const String EmptyUpperIdentifier;
PlasmaShared extern const String DefaultLibraryName;

// Helper functions
PlasmaShared String BuildGetterName(StringParam name);
PlasmaShared String BuildSetterName(StringParam name);

// Perform string escape replacements
PlasmaShared String ReplaceStringEscapes(StringRange input);

// Strip outlining quotes (directly used for string literals)
PlasmaShared StringRange StripStringQuotes(StringRange input);

// Adds quotes to the string
PlasmaShared StringRange AddStringQuotes(StringRange input);

// Perform string escape replacements and outlining quotes (directly used for
// string literals)
PlasmaShared String ReplaceStringEscapesAndStripQuotes(StringRange input);

// Replaces all ascii escapable characters with escapes and adds quotes to the
// string
PlasmaShared String EscapeStringAndAddQuotes(StringRange input);

// Change an identifier between lower and upper camel cases (just modifies the
// first letter)
PlasmaShared String ToLowerCamelCase(StringRange input);
PlasmaShared String ToUpperCamelCase(StringRange input);
} // namespace Lightning

#endif
