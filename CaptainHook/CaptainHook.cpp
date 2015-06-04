

#include "stdafx.h"
#include "CaptainHook.h"


CAPTAINHOOK_API int nCaptainHook=0;

// Dies ist das Beispiel einer exportierten Funktion.
CAPTAINHOOK_API int fnCaptainHook(void)
{
	return 42;
}

