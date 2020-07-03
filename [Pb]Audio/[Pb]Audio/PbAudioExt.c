
#include "../[Pb]Audio.h"

/*
void pbaudio_wasapi_ext_init()
{
	HMODULE avrt_dll;
	//HMODULE dxgi_debug_dll;
		
	//HMODULE d3d12_dll;
	//HMODULE d3d12_sdk_layer_dll;
	
	HRESULT hr;

	//load dxgi dll to create the device that will give us the desired swap chain features
	avrt_dll = LoadLibraryEx(_T("Avrt.dll"), NULL, 0);
	

	//HINSTANCE dll;
	//typedef PROC WINAPI wglGetProcAddressproc(LPCSTR lpszProc);
	//wglGetProcAddressproc* wglGetProcAddress;

	//use xmacros to iterate over each function definition and expand it
	//use xmacros to iterate over each function definition and expand it
    #define PBAXE(ret, name, dllname, ...) \
            name = (name##proc *)GetProcAddress(avrt_dll, #dllname); \
            if (!name) { \
                printf("\nFunction " #dllname " couldn't be loaded from Avrt.dll\n"); \
            }
	        PBA_WSAPI_FUNC_LIST
    #undef PBAXE

}


*/