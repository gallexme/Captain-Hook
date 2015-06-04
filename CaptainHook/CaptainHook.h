// Folgender ifdef-Block ist die Standardmethode zum Erstellen von Makros, die das Exportieren 
// aus einer DLL vereinfachen. Alle Dateien in dieser DLL werden mit dem CAPTAINHOOK_EXPORTS-Symbol
// (in der Befehlszeile definiert) kompiliert. Dieses Symbol darf für kein Projekt definiert werden,
// das diese DLL verwendet. Alle anderen Projekte, deren Quelldateien diese Datei beinhalten, erkennen 
// CAPTAINHOOK_API-Funktionen als aus einer DLL importiert, während die DLL
// mit diesem Makro definierte Symbole als exportiert ansieht.
#ifdef CAPTAINHOOK_EXPORTS
#define CAPTAINHOOK_API __declspec(dllexport)
#else
#define CAPTAINHOOK_API __declspec(dllimport)
#endif


extern CAPTAINHOOK_API int nCaptainHook;

CAPTAINHOOK_API int fnCaptainHook(void);
