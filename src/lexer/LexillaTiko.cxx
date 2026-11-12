// Lexilla64.dll for the Windows build of the tiko editor: the Lexilla library interface with
// one lexer, "tiko" (LexTiko.cxx, the same lexer the Linux build links statically).
// Built by 'make win-lexer' with MinGW; see the Makefile.
// License: the Scintilla/Lexilla license (License.txt in this folder).

#include <cstring>

#include "ILexer.h"

#define LEXILLA_CALL __stdcall

extern "C" Scintilla::ILexer5 *TikoCreateLexer();

namespace {
const char lexerName[] = "tiko";
}

extern "C" {

__declspec(dllexport) int LEXILLA_CALL GetLexerCount(void) {
	return 1;
}

__declspec(dllexport) void LEXILLA_CALL GetLexerName(unsigned int index, char *name, int buflength) {
	if (!name || buflength <= 0)
		return;
	*name = '\0';
	if (index == 0) {
		strncpy(name, lexerName, static_cast<size_t>(buflength) - 1);
		name[buflength - 1] = '\0';
	}
}

typedef Scintilla::ILexer5 *(*LexerFactoryFunction)(void);

__declspec(dllexport) LexerFactoryFunction LEXILLA_CALL GetLexerFactory(unsigned int index) {
	return index == 0 ? TikoCreateLexer : nullptr;
}

__declspec(dllexport) Scintilla::ILexer5 *LEXILLA_CALL CreateLexer(const char *name) {
	if (name && (strcmp(name, lexerName) == 0 || strcmp(name, "freebasic") == 0))
		return TikoCreateLexer();
	return nullptr;
}

__declspec(dllexport) const char *LEXILLA_CALL LexerNameFromID(int identifier) {
	return identifier == 75 ? lexerName : nullptr;		// SCLEX_FREEBASIC
}

__declspec(dllexport) const char *LEXILLA_CALL GetLibraryPropertyNames(void) {
	return "";
}

__declspec(dllexport) void LEXILLA_CALL SetLibraryProperty(const char *, const char *) {
}

__declspec(dllexport) const char *LEXILLA_CALL GetNameSpace(void) {
	return "lexilla";
}

}
