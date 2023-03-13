#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

HINSTANCE g_hDLL = NULL;

typedef BOOL (APIENTRY *FN_StartHook)(HWND hwnd);
typedef BOOL (APIENTRY *FN_EndHook)(HWND hwnd);

FN_StartHook g_pStartHook = NULL;
FN_StartHook g_pEndHook = NULL;
BOOL bHooked = FALSE;
UINT g_WM_SHELLHOOK = 0;

void DebugPrintf(const char *fmt, ...)
{
    char buf[512];
    va_list va;
    va_start(va, fmt);
    wvsprintfA(buf, fmt, va);
    OutputDebugStringA(buf);
    va_end(va);
}
#define DPRINT(fmt, ...) DebugPrintf("%s: Line %d: " fmt, __FILE__, __LINE__, ## __VA_ARGS__)

BOOL MyLoadDLL(HWND hwnd)
{
    g_hDLL = LoadLibraryA("hookdll.dll");
    if (g_hDLL == NULL)
    {
        DPRINT("Failed to load hookdll.dll\n");
        EndDialog(hwnd, IDABORT);
        return FALSE;
    }

    g_pStartHook = GetProcAddress(g_hDLL, "StartHook");
    g_pEndHook = GetProcAddress(g_hDLL, "EndHook");
    if (!g_pStartHook || !g_pEndHook)
    {
        DPRINT("Failed to load hookdll.dll\n");
        FreeLibrary(g_hDLL);
        EndDialog(hwnd, IDABORT);
        return FALSE;
    }

    DPRINT("hookdll.dll is loaded\n");
    return TRUE;
}

VOID MyUnloadDll(HWND hwnd)
{
    if (g_hDLL)
    {
        g_pEndHook(hwnd);
        FreeLibrary(g_hDLL);
        g_hDLL = NULL;
        DPRINT("hookdll.dll is unloaded\n");
    }
    g_pStartHook = NULL;
    g_pEndHook = NULL;
}

BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    MyLoadDLL(hwnd);

    RegisterShellHookWindow(hwnd);

    g_WM_SHELLHOOK = RegisterWindowMessage(TEXT("SHELLHOOK"));

    return TRUE;
}

void OnStartHook(HWND hwnd)
{
    DPRINT("OnStartHook enter\n");

    if (!g_pStartHook(hwnd))
    {
        DPRINT("Failed to StartHook\n");
        MessageBox(hwnd, TEXT("Failed to StartHook"), NULL, MB_ICONERROR);
    }

    DPRINT("OnStartHook leave\n");
}

void OnEndHook(HWND hwnd)
{
    DPRINT("OnEndHook enter\n");
    g_pEndHook(hwnd);
    DPRINT("OnEndHook leave\n");
}

void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id)
    {
    case IDOK:
    case IDCANCEL:
        EndDialog(hwnd, id);
        break;
    case psh1: // StartHook
        OnStartHook(hwnd);
        break;
    case psh2: // EndHook
        OnEndHook(hwnd);
        break;
    }
}

void OnDestroy(HWND hwnd)
{
    DeregisterShellHookWindow(hwnd);
    MyUnloadDll(hwnd);
}

INT_PTR CALLBACK
DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_INITDIALOG, OnInitDialog);
        HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
        HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);
    default:
        if (g_WM_SHELLHOOK && g_WM_SHELLHOOK == uMsg)
        {
            DPRINT("WM_SHELLHOOKMESSAGE: %p, %p\n", wParam, lParam);
        }
        break;
    }
    return 0;
}

INT WINAPI
WinMain(HINSTANCE   hInstance,
        HINSTANCE   hPrevInstance,
        LPSTR       lpCmdLine,
        INT         nCmdShow)
{
    InitCommonControls();
    if (DialogBox(hInstance, MAKEINTRESOURCE(1), NULL, DialogProc) == IDABORT)
    {
        MessageBox(NULL, TEXT("Failed to load 'hookdll.dll'"), NULL, MB_ICONERROR);
    }
    return 0;
}
