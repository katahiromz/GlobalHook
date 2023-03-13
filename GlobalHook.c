#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

HINSTANCE g_hDLL = NULL;

typedef BOOL (APIENTRY *FN_StartHook)(VOID);
typedef BOOL (APIENTRY *FN_EndHook)(VOID);

FN_StartHook g_pStartHook = NULL;
FN_StartHook g_pEndHook = NULL;
BOOL bHooked = FALSE;

void DebugPrintf(const char *fmt, ...)
{
    char buf[512];
    va_list va;
    va_start(va, fmt);
    wvsprintfA(buf, fmt, va);
    OutputDebugStringA(buf);
    va_end(va);
}
#define DPRINT(fmt, ...) DebugPrintf("Line %d: " fmt, __LINE__, ## __VA_ARGS__)

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
        (*g_pEndHook)();
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
    return TRUE;
}

void OnStartHook(HWND hwnd)
{
    DPRINT("OnStartHook enter\n");

    // Try to hook 20 times
    for (INT i = 0; i < 20; ++i)
    {
        BOOL ret = (*g_pStartHook)();
        if (!ret)
        {
            DPRINT("Failed to StartHook\n");
            MessageBox(hwnd, TEXT("Failed to StartHook"), NULL, MB_ICONERROR);
            continue;
        }
        (*g_pEndHook)();

        MyUnloadDll(hwnd);

        CopyFileA("hookdll.dll", "copyed.dll", FALSE);
        if (!DeleteFileA("hookdll.dll"))
        {
            DPRINT("Failed to DeleteFile\n");
            MessageBox(hwnd, TEXT("Failed to DeleteFile"), NULL, MB_ICONERROR);
        }
        CopyFileA("copyed.dll", "hookdll.dll", FALSE);
        DeleteFileA("copyed.dll");

        MyLoadDLL(hwnd);
    }

    (*g_pStartHook)();
    DPRINT("OnStartHook leave\n");
}

void OnEndHook(HWND hwnd)
{
    DPRINT("OnEndHook enter\n");
    (*g_pEndHook)();
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
