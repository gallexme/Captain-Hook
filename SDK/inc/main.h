#pragma once

#include <windows.h>

#define IMPORT __declspec(dllimport)

IMPORT void scriptWait(DWORD time);
IMPORT void scriptRegisterArg(HMODULE module, void(*LP_SCRIPT_MAIN)(char* arg),char* argument);
IMPORT void scriptRegister(HMODULE module, void(*LP_SCRIPT_MAIN)());
IMPORT void scriptUnregister(void(*LP_SCRIPT_MAIN)());
IMPORT void scriptUnregisterArg(void(*function)(char* arg));
IMPORT void scriptUnregisterArg(void(*function)(char* arg),char * arg);
typedef void(*KeyboardHandler)(DWORD, WORD, BYTE, BOOL, BOOL, BOOL, BOOL);

IMPORT void keyboardHandlerRegister(KeyboardHandler handler);
IMPORT void keyboardHandlerUnregister(KeyboardHandler handler);

IMPORT void nativeInit(UINT64 hash);
IMPORT void nativePush64(UINT64 val);
IMPORT PUINT64 nativeCall();

static void WAIT(DWORD time) { scriptWait(time); }
static void TERMINATE() { WAIT(MAXDWORD); }