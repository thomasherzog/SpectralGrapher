#include "native/windows/Win32CustomTitlebar.h"

#include "windowing/BaseWindow.h"

#define RECTWIDTH(rc)            (rc.right - rc.left)
#define RECTHEIGHT(rc)            (rc.bottom - rc.top)

Win32CustomTitlebar::Win32CustomTitlebar(GLFWwindow *window) : window(window) {
    HWND hwnd = glfwGetWin32Window(window);

    SetWindowSubclass(hwnd, &Win32CustomTitlebar::myProc, 1, (DWORD_PTR) this);

    const MARGINS shadow_on = {0, 0, 1, 0};
    DwmExtendFrameIntoClientArea(hwnd, &shadow_on);
}

LRESULT CALLBACK Win32CustomTitlebar::myProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass,
                                             DWORD_PTR dwRefData) {
    switch (uMsg) {
        case WM_ACTIVATE: {
            SetWindowPos(hwnd, hwnd, 0, 0, 0, 0,
                         SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
            return DefSubclassProc(hwnd, uMsg, wParam, lParam);
        }
        case WM_NCCALCSIZE: {
            return getWmNcCalcSize(hwnd, uMsg, wParam, lParam);
        }
        case WM_NCHITTEST: {
            auto hit = getBorderlessHitTest(hwnd, uMsg, wParam, lParam);
            if (hit == 0) {
                return DefSubclassProc(hwnd, uMsg, wParam, lParam);
            }
            return hit;
        }
        default: {
            return DefSubclassProc(hwnd, uMsg, wParam, lParam);
        }
    }
}

LRESULT Win32CustomTitlebar::getBorderlessHitTest(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static struct TitlebarSettings {
        int height = 27;
        int controlBoxWidth = 150;
        int iconWidth = 30;
        int extraLeftReservedWidth = 150;
        int extraRightReservedWidth = 30;
        int frameBorderThickness = 8;
    } settings;

    POINT cursorPos{};
    GetCursorPos(&cursorPos);

    RECT windowRect{};
    GetWindowRect(hwnd, &windowRect);

    // Control Elements (box, space, etc.)
    // Order: control box > app icon > extra left reserved space > extra right reserved space
    if (cursorPos.y >= windowRect.top && cursorPos.y < windowRect.top + settings.height) {
        if (cursorPos.x > windowRect.right - 8 - settings.controlBoxWidth && cursorPos.x <= windowRect.right - 8) {
            // Control Box
            if (cursorPos.x > windowRect.right - 8 - (settings.controlBoxWidth / 3)) {
                return HTNOWHERE;
            } else if (cursorPos.x > windowRect.right - 8 - (settings.controlBoxWidth / 3) * 2) {
                return HTNOWHERE;
            } else if (cursorPos.x > windowRect.right - 8 - settings.controlBoxWidth) {
                return HTNOWHERE;
            }
            return HTNOWHERE;
        } else if (cursorPos.x >= windowRect.left + 8 && cursorPos.x < windowRect.left + settings.iconWidth + 8) {
            // App Icon
            return HTNOWHERE;
        } else if (cursorPos.x >= windowRect.left + 8 + settings.iconWidth &&
                   cursorPos.x < windowRect.left + settings.iconWidth + 8 + settings.extraLeftReservedWidth) {
            // Extra left reserved space
            return HTNOWHERE;
        } else if (cursorPos.x > windowRect.right - 8 - settings.controlBoxWidth - settings.extraRightReservedWidth
                   && cursorPos.x <= windowRect.right - 8 - settings.controlBoxWidth) {
            // Extra right reserved space
            return HTNOWHERE;
        }
    }

    if (cursorPos.y >= windowRect.top && cursorPos.y < windowRect.top + settings.frameBorderThickness
        && cursorPos.x >= windowRect.left + 8 && cursorPos.x <= windowRect.right - 8) {
        return HTTOP;
    }

    if (cursorPos.y >= windowRect.top && cursorPos.y < windowRect.top + settings.height
        && cursorPos.x >= windowRect.left + 8 && cursorPos.x <= windowRect.right - 8) {
        return HTCAPTION;
    }

    return HTNOWHERE;
}

LRESULT Win32CustomTitlebar::getWmNcCalcSize(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static struct TitlebarSettings {
        int height = 27;
        int controlBoxWidth = 150;
        int iconWidth = 30;
        int extraLeftReservedWidth = 150;
        int extraRightReservedWidth = 30;
        int frameBorderThickness = 8;
    } settings;

    if (wParam) {
        auto *ncParams = reinterpret_cast<NCCALCSIZE_PARAMS *>(lParam);
        ncParams->rgrc[0].left += settings.frameBorderThickness;
        ncParams->rgrc[0].bottom -= settings.frameBorderThickness;
        ncParams->rgrc[0].right -= settings.frameBorderThickness;

        if (IsZoomed(hwnd)) {
            ncParams->rgrc[0].top += settings.frameBorderThickness;
        }
    }
    return LRESULT(0);
}
