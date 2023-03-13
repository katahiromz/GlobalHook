#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

HINSTANCE g_hDLL = NULL;

typedef BOOL (APIENTRY *FN_StartHook)(VOID);
typedef BOOL (APIENTRY *FN_EndHook)(VOID);

FN_StartHook g_pStartHook = NULL;
FN_StartHook g_pEndHook = NULL;

void DebugPrintf(const char *fmt, ...)
{
    char buf[512];
    va_list va;
    va_start(va, fmt);
    wvsprintfA(buf, fmt, va);
    OutputDebugStringA(buf);
    va_end(va);
}

BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    g_hDLL = LoadLibraryA("hookdll.dll");
    if (g_hDLL == NULL)
    {
        EndDialog(hwnd, IDABORT);
        return FALSE;
    }

    g_pStartHook = GetProcAddress(g_hDLL, "StartHook");
    g_pEndHook = GetProcAddress(g_hDLL, "EndHook");
    if (!g_pStartHook || !g_pEndHook)
    {
        FreeLibrary(g_hDLL);
        EndDialog(hwnd, IDABORT);
        return FALSE;
    }

    DebugPrintf("hookdll.dll is loaded\n");
    return TRUE;
}

void OnStartHook(HWND hwnd)
{
    DebugPrintf("OnStartHook enter\n");

    // Try to hook 1000 times
    for (INT i = 0; i < 1000; ++i)
    {
        BOOL ret = (*g_pStartHook)();
        if (!ret)
        {
            DebugPrintf("Failed to StartHook\n");
            MessageBox(hwnd, TEXT("Failed to StartHook"), NULL, MB_ICONERROR);
            continue;
        }
        (*g_pEndHook)();
    }

    (*g_pStartHook)();
    DebugPrintf("OnStartHook leave\n");
}

void OnEndHook(HWND hwnd)
{
    DebugPrintf("OnEndHook enter\n");
    (*g_pEndHook)();
    DebugPrintf("OnEndHook leave\n");
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
    if (g_hDLL)
    {
        OnEndHook(hwnd);
        FreeLibrary(g_hDLL);
        g_hDLL = NULL;
        DebugPrintf("hookdll.dll is unloaded\n");
    }
    g_pStartHook = NULL;
    g_pEndHook = NULL;
}

INT_PTR CALLBACK
DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_INITDIALOG, OnInitDialog);
        HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
        HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);
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
