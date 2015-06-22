#include "ScriptManager.h"
#include "..\Utility\Log.h"

using namespace Utility;

#pragma comment(lib, "winmm.lib")
#define DLL_EXPORT __declspec( dllexport )

ScriptManagerThread g_ScriptManagerThread;
std::vector<ScriptManagerThread> g_ScriptManagers;

static HANDLE		mainFiber;
static Script *		currentScript;

void Script::Tick() {

	if ( mainFiber == nullptr ) {
		mainFiber = ConvertThreadToFiber( nullptr );
	}

	if ( timeGetTime() < wakteAt ) {
		return;
	}

	if ( scriptFiber ) {

		currentScript = this;
		SwitchToFiber( scriptFiber );
		currentScript = nullptr;
	} else {

		scriptFiber = CreateFiber(NULL, [](LPVOID handler) {

			__try {

				reinterpret_cast<Script*>(handler)->Run();

			}
			__except (EXCEPTION_EXECUTE_HANDLER) {

				LOG_ERROR("Error in script->Run");
			}
		}, this );
	}
}

void Script::Run() {
	if (callbackFunction != nullptr)
		callbackFunction();
	else if (callbackArgFunction != nullptr)
		callbackArgFunction(arg);
}

void Script::Yield( uint32_t time ) {

	wakteAt = timeGetTime() + time;
	SwitchToFiber( mainFiber );
}

void ScriptManagerThread::DoRun() {

	scriptVec thisIterScripts( m_scripts );

	for ( auto & script : thisIterScripts ) {
		
		script.script->Tick();
	}
}

eThreadState ScriptManagerThread::Reset( uint32_t scriptHash, void * pArgs, uint32_t argCount ) {

	// Collect all script functions
	std::vector<void( *)( )> scriptFunctions;
	std::vector <scriptFunction> scriptArgFunctions;
	for ( auto&& script : m_scripts ) {
		if (script.script->isArgScript) {
			scriptArgFunctions.push_back(script);
		}
		else {
			scriptFunctions.push_back(script.script->GetCallbackFunction());
		}
	}

	// Clear the script list
	m_scripts.clear();

	// Start all script functions
	for ( auto && fn : scriptFunctions ) {
		AddScript( fn );
	}
	for (auto && fn : scriptArgFunctions) {
		AddArgScript(fn.script->GetCallbackArgFunction(),fn.arg);
	}


	return ScriptThread::Reset( scriptHash, pArgs, argCount );
}

void ScriptManagerThread::AddScript( void( *fn )( ) ) {

	scriptFunction fnc = scriptFunction();
	fnc.script = std::make_shared<Script>(fn);
	
	m_scripts.push_back(fnc);
}
void ScriptManagerThread::AddArgScript(void(*fn)(char* arg),char* argument) {
	scriptFunction fnc =  scriptFunction();
	fnc.script = std::make_shared<Script>(fn,argument);
	fnc.arg = argument;
	m_scripts.push_back(fnc);
}
void ScriptManagerThread::RemoveScript( void( *fn )( ) ) {

	for ( auto it = m_scripts.begin(); it != m_scripts.end(); it++ ) {

		if ( ( *it ).script->GetCallbackFunction() == fn ) {

			m_scripts.erase( it );
			return;
		}
	}
}
void ScriptManagerThread::RemoveArgScript(void(*fn)(char* arg)) {

	for (auto it = m_scripts.begin(); it != m_scripts.end(); it++) {

		if ((*it).script->GetCallbackArgFunction() == fn) {

			m_scripts.erase(it);
		}
	}
}
void ScriptManagerThread::RemoveArgScript(void(*fn)(char* arg),char* argument) {

	for (auto it = m_scripts.begin(); it != m_scripts.end(); it++) {

		if ((*it).script->GetCallbackArgFunction() == fn && (*it).arg == argument) {

			m_scripts.erase(it);
		}
	}
}
void DLL_EXPORT scriptWait( unsigned long waitTime ) {

	currentScript->Yield( waitTime );
}
void DLL_EXPORT  scriptRegisterArg(HMODULE module, void(*function)(char* arg),char* argument) {
	g_ScriptManagerThread.AddArgScript(function,argument);
}
void DLL_EXPORT scriptRegister( HMODULE hMod, void( *function )( ) ) {

	LOG_DEBUG( "Registering script: 0x%p", hMod );
	g_ScriptManagerThread.AddScript( function );
}

void DLL_EXPORT scriptUnregister( void( *function )( ) ) {

	LOG_DEBUG( "Unregistering script fn: 0x%p", function );
	g_ScriptManagerThread.RemoveScript( function );
}
void DLL_EXPORT scriptUnregisterArg(void(*function)(char* arg)) {
	LOG_DEBUG("Unregistering script fn: 0x%p", function);
	g_ScriptManagerThread.RemoveArgScript(function);
}

void DLL_EXPORT scriptUnregisterArg(void(*function)(char* arg),char* argument) {
	LOG_DEBUG("Unregistering script fn: 0x%p", function);
	g_ScriptManagerThread.RemoveArgScript(function,argument);
}

static ScriptManagerContext g_context;
static uint64_t g_hash;

void DLL_EXPORT nativeInit( uint64_t hash ) {

	g_context.Reset();
	g_hash = hash;
}

void DLL_EXPORT nativePush64( uint64_t value ) {

	g_context.Push( value );
}

DLL_EXPORT uint64_t* nativeCall() {

	auto fn = ScriptEngine::GetNativeHandler( g_hash );

	if ( fn != 0 ) {

		__try {

			fn( &g_context );
		} __except ( EXCEPTION_EXECUTE_HANDLER ) {

			LOG_ERROR( "Error in nativeCall" );
		}
	}

	return reinterpret_cast<uint64_t*>( g_context.GetResultPointer() );
}

typedef void( *TKeyboardFn )( DWORD key, WORD repeats, BYTE scanCode, BOOL isExtended, BOOL isWithAlt, BOOL wasDownBefore, BOOL isUpNow );

static std::set<TKeyboardFn> g_keyboardFunctions;

void DLL_EXPORT keyboardHandlerRegister( TKeyboardFn function ) {

	g_keyboardFunctions.insert( function );
}

void DLL_EXPORT keyboardHandlerUnregister( TKeyboardFn function ) {

	g_keyboardFunctions.erase( function );
}

void ScriptManager::WndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) {

	if ( uMsg == WM_KEYDOWN || uMsg == WM_KEYUP || uMsg == WM_SYSKEYDOWN || uMsg == WM_SYSKEYUP ) {

		auto functions = g_keyboardFunctions;

		for ( auto & function : functions ) {
			function( (DWORD)wParam, lParam & 0xFFFF, ( lParam >> 16 ) & 0xFF, ( lParam >> 24 ) & 1, ( uMsg == WM_SYSKEYDOWN || uMsg == WM_SYSKEYUP ), ( lParam >> 30 ) & 1, ( uMsg == WM_SYSKEYUP || uMsg == WM_KEYUP ) );
		}
	}
}
