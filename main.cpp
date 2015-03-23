#include <atlimage.h>
#include <chrono>
#include <string>
#include <thread>
#include <Windows.h>
#include <Windowsx.h>

HHOOK MouseHook;
HHOOK KeyboardHook;
uint32_t imageId = 0;

void screenCap()
{
    // get the device context of the screen
    HDC hScreenDC = GetDC(NULL);

    // and a device context to put it in
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

    int x = GetDeviceCaps(hScreenDC, HORZRES);
    int y = GetDeviceCaps(hScreenDC, VERTRES);

    // maybe worth checking these are positive values
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, x, y);

    // get a new bitmap
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemoryDC, (HGDIOBJ)hBitmap);

    BitBlt(hMemoryDC, 0, 0, x, y, hScreenDC, 0, 0, SRCCOPY);
    hBitmap = (HBITMAP)SelectObject(hMemoryDC, hOldBitmap);

    // clean up
    DeleteDC(hMemoryDC);
    DeleteDC(hScreenDC);

    ++imageId;
    std::string filename = "duksnap" + std::to_string(imageId) + ".jpg";

    // Save image
    {
        CImage image;
        image.Attach(hBitmap);
        image.Save(filename.c_str());
    }

    // Load the picture and start a transfer
}

LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    PKBDLLHOOKSTRUCT k = (PKBDLLHOOKSTRUCT)(lParam);

    if (wParam == WM_MBUTTONDOWN)
    {
        screenCap();
        return 1;
    }

    return S_OK;
}

bool bCtrlDown = false;

LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    PKBDLLHOOKSTRUCT k = (PKBDLLHOOKSTRUCT)(lParam);
    tagKBDLLHOOKSTRUCT* pKey = reinterpret_cast<tagKBDLLHOOKSTRUCT*>(lParam);

    if (wParam == WM_KEYDOWN)
    {
        if (pKey->vkCode == VK_LCONTROL)
        {
            bCtrlDown = true;
        }
        if (pKey->vkCode == static_cast<decltype(lParam)>('G') &&
            bCtrlDown)
        {
            PostQuitMessage(0);
        }
    }
    else if (wParam == WM_KEYUP)
    {
        if (pKey->vkCode == VK_LCONTROL)
        {
            bCtrlDown = false;
        }
    }

    return S_OK;
}

int CALLBACK WinMain(HINSTANCE appInstance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdCount)
{
    MouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseHookProc, appInstance, 0);
    KeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHookProc, appInstance, 0);

    MSG msg = {0};
    while (true)
    {
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT)
            {
                break;
            }
        }
    }

    UnhookWindowsHookEx(MouseHook);
}
