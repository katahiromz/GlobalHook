#include <windows.h>

HINSTANCE g_hinstDLL = NULL;

#ifdef _MSC_VER
    #define SHARED(name)
#else
    #define SHARED(name) __attribute__((section(name), shared))
#endif

#ifdef _MSC_VER
    #pragma data_seg(".shared")
#endif

/* The following handles are shared throughout the system: */
HHOOK g_hShellHook SHARED(".shared") = NULL;
HHOOK g_hCBTHook SHARED(".shared") = NULL;

#ifdef _MSC_VER
    #pragma data_seg()
    #pragma comment(linker, "/section:.shared,rws")
#endif

void DebugPrintf(const char *fmt, ...)
{
    char buf[512];
    va_list va;
    va_start(va, fmt);
    wvsprintfA(buf, fmt, va);
    OutputDebugStringA(buf);
    va_end(va);
}

LRESULT CALLBACK
ShellProc(
    INT nCode,
    WPARAM wParam,
    LPARAM lParam)
{
    if (nCode < 0)
        return CallNextHookEx(g_hShellHook, nCode, wParam, lParam);

    switch (nCode)
    {
    case HSHELL_ACTIVATESHELLWINDOW:
        DebugPrintf("HSHELL_ACTIVATESHELLWINDOW: %p, %p\n", wParam, lParam);
        break;
    case HSHELL_WINDOWCREATED:
        DebugPrintf("HSHELL_WINDOWCREATED: %p, %p\n", wParam, lParam);
        break;
    case HSHELL_WINDOWDESTROYED:
        DebugPrintf("HSHELL_WINDOWDESTROYED: %p, %p\n", wParam, lParam);
        break;
    }

    return CallNextHookEx(g_hShellHook, nCode, wParam, lParam);
}

LRESULT CALLBACK
CBTProc(
    INT nCode,
    WPARAM wParam,
    LPARAM lParam)
{
    if (nCode < 0)
        return CallNextHookEx(g_hCBTHook, nCode, wParam, lParam);

    switch (nCode)
    {
    case HCBT_ACTIVATE:
        DebugPrintf("HCBT_ACTIVATE: %p, %p\n", wParam, lParam);
        break;
    case HCBT_CLICKSKIPPED:
        DebugPrintf("HCBT_CLICKSKIPPED: %p, %p\n", wParam, lParam);
        break;
    case HCBT_CREATEWND:
        DebugPrintf("HCBT_CREATEWND: %p, %p\n", wParam, lParam);
        break;
    case HCBT_DESTROYWND:
        DebugPrintf("HCBT_DESTROYWND: %p, %p\n", wParam, lParam);
        break;
    case HCBT_KEYSKIPPED:
        DebugPrintf("HCBT_KEYSKIPPED: %p, %p\n", wParam, lParam);
        break;
    case HCBT_MINMAX:
        DebugPrintf("HCBT_MINMAX: %p, %p\n", wParam, lParam);
        break;
    case HCBT_MOVESIZE:
        DebugPrintf("HCBT_MOVESIZE: %p, %p\n", wParam, lParam);
        break;
    case HCBT_QS:
        DebugPrintf("HCBT_QS: %p, %p\n", wParam, lParam);
        break;
    case HCBT_SETFOCUS:
        DebugPrintf("HCBT_SETFOCUS: %p, %p\n", wParam, lParam);
        break;
    case HCBT_SYSCOMMAND:
        DebugPrintf("HCBT_SYSCOMMAND: %p, %p\n", wParam, lParam);
        break;
    }

    return CallNextHookEx(g_hCBTHook, nCode, wParam, lParam);
}

BOOL APIENTRY EndHook(VOID)
{
    BOOL ret = TRUE;

    if (!UnhookWindowsHookEx(g_hShellHook))
        ret = FALSE;
    if (!UnhookWindowsHookEx(g_hCBTHook))
        ret = FALSE;

    g_hShellHook = g_hCBTHook = NULL;
    return ret;
}

BOOL APIENTRY StartHook(VOID)
{
    if (g_hShellHook || g_hCBTHook)
        EndHook();

    g_hShellHook = SetWindowsHookEx(WH_SHELL, ShellProc, g_hinstDLL, 0);
    g_hCBTHook = SetWindowsHookEx(WH_SHELL, CBTProc, g_hinstDLL, 0);
    if (!g_hShellHook || !g_hCBTHook)
    {
        EndHook();
        return FALSE;
    }

    return TRUE;
}

BOOL WINAPI
DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        g_hinstDLL = hinstDLL;
        DisableThreadLibraryCalls(hinstDLL);
        DebugPrintf("DLL_PROCESS_ATTACH: %p\n", hinstDLL);
        break;

    case DLL_PROCESS_DETACH:
        DebugPrintf("DLL_PROCESS_DETACH\n");
        break;
    }
    return TRUE;
}
