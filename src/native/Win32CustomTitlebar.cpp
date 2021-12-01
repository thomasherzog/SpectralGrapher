#include "native/Win32CustomTitlebar.h"



#define RECTWIDTH(rc)            (rc.right - rc.left)
#define RECTHEIGHT(rc)            (rc.bottom - rc.top)

Win32CustomTitlebar::Win32CustomTitlebar(GLFWwindow *window) {
    HWND hwnd = glfwGetWin32Window(window);


    SetWindowSubclass(hwnd, &Win32CustomTitlebar::myProc, 1, (DWORD_PTR) this);

    const MARGINS shadow_on = {0, 0, 1, 0};
    DwmExtendFrameIntoClientArea(hwnd, &shadow_on);
     //
    //UpdateWindow(hwnd);
}

LRESULT CALLBACK Win32CustomTitlebar::myProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass,
                                             DWORD_PTR dwRefData) {

    static HBRUSH brush;
    switch (uMsg) {
        case WM_ACTIVATE: {
            SetWindowPos(hwnd, hwnd, 0,0,0,0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

            brush = CreateSolidBrush(RGB(36,36,36));
        }
        case WM_SIZE: {
            printf("T");
        }
        case WM_ERASEBKGND: {
            HDC hdc = (HDC)(wParam);
            RECT rc; GetClientRect(hwnd, &rc);
            FillRect(hdc, &rc, brush);
            return TRUE;
        }
        case WM_NCCALCSIZE: {
            if(lParam) {
                auto *ncParams = reinterpret_cast<NCCALCSIZE_PARAMS *>(lParam);
                ncParams->rgrc[0].left += 8;
                ncParams->rgrc[0].bottom -= 8;
                ncParams->rgrc[0].right -= 8;
            }
            return LRESULT(0);
        }
        case WM_NCHITTEST: {
            auto hit = getBorderlessHitTest(hwnd, uMsg, wParam, lParam);
            if (hit == 0) {
                return DefSubclassProc(hwnd, uMsg, wParam, lParam);
            }
            return hit;
        }
        case WM_DESTROY: {
            DeleteObject(brush); // Free the created brush: see note below!
        }
        default: {
            return DefSubclassProc(hwnd, uMsg, wParam, lParam);
        }
    }
}

LRESULT Win32CustomTitlebar::getBorderlessHitTest(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    struct TitlebarSettings {
        int height = 27;
        int controlBoxWidth = 150;
        int iconWidth = 40;
        int extraLeftReservedWidth = 150;
        int extraRightReservedWidth = 0;

        int maximizedWindowFrameThickness = 30;
        int frameResizeBorderThickness = 10;
        int frameBorderThickness = 5;
    } settings;

    POINT cursorPos{};
    GetCursorPos(&cursorPos);

    RECT windowRect{};
    GetWindowRect(hwnd, &windowRect);

    int uRow = 1;
    int uCol = 1;
    bool onResizeBorder = false;
    bool onFrameDrag = false;

    int offsetTop = settings.height == 0 ? settings.frameBorderThickness : settings.height;

    if (cursorPos.y >= windowRect.top &&
        cursorPos.y < windowRect.top + offsetTop + settings.maximizedWindowFrameThickness) {
        onResizeBorder = (cursorPos.y < (windowRect.top + settings.frameBorderThickness));
        if (!onResizeBorder) {
            onFrameDrag = (cursorPos.y <= windowRect.top + settings.height + settings.maximizedWindowFrameThickness)
                          && (cursorPos.x < (windowRect.right - (settings.controlBoxWidth
                                                                 + settings.maximizedWindowFrameThickness +
                                                                 settings.extraRightReservedWidth)))
                          && (cursorPos.x > (windowRect.left + settings.iconWidth
                                             + settings.maximizedWindowFrameThickness +
                                             settings.extraLeftReservedWidth));
        }
        uRow = 0;
    } else if (cursorPos.y < windowRect.bottom + settings.frameBorderThickness && cursorPos.y >= windowRect.bottom) {
        uRow = 2;
    }

    if (cursorPos.x >= windowRect.left && cursorPos.x < windowRect.left + settings.frameBorderThickness) {
        uCol = 0;
    } else if (cursorPos.x < windowRect.right && cursorPos.x >= windowRect.right - settings.frameBorderThickness) {
        uCol = 2;
    }

    int hitTests[3][3] = {
            {HTTOPLEFT, onResizeBorder ? HTTOP : onFrameDrag ? HTCAPTION : HTNOWHERE, HTTOPRIGHT},
            {HTLEFT,       HTNOWHERE,                                                 HTRIGHT},
            {HTBOTTOMLEFT, HTBOTTOM,                                                  HTBOTTOMRIGHT}
    };

    return LRESULT(hitTests[uRow][uCol]);
}
