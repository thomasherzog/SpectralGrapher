#include <windowing/VulkanWindow.h>
#include "native/windows/Win32CustomTitlebar.h"

#include "windowing/BaseWindow.h"

Win32CustomTitlebar::Win32CustomTitlebar(GLFWwindow *window, TitlebarProperties properties) : window(window),
                                                                                              properties(properties) {
    HWND hwnd = glfwGetWin32Window(window);

    brush = ::CreateSolidBrush(RGB(36, 36, 36));

    SetWindowSubclass(hwnd, &Win32CustomTitlebar::staticCustomSubclass, 1, (DWORD_PTR) this);

    SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR) brush);

    SetWindowPos(hwnd, hwnd, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

Win32CustomTitlebar::~Win32CustomTitlebar() {
    ::DeleteObject(brush);
    ::RemoveWindowSubclass(glfwGetWin32Window(window), &Win32CustomTitlebar::staticCustomSubclass, 1);
}


LRESULT
Win32CustomTitlebar::staticCustomSubclass(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass,
                                          DWORD_PTR dwRefData) {
    auto *pThis = reinterpret_cast<Win32CustomTitlebar *>(dwRefData);
    return pThis->customSubclass(hwnd, uMsg, wParam, lParam, uIdSubclass);
}


LRESULT
CALLBACK Win32CustomTitlebar::customSubclass(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass) {
    switch (uMsg) {
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
        case WM_NCLBUTTONDOWN:
        case WM_NCLBUTTONUP:
            if (wParam == HTMINBUTTON || wParam == HTMAXBUTTON || wParam == HTCLOSE) {
                int uClientMsg = (uMsg == WM_NCLBUTTONDOWN) ? WM_LBUTTONDOWN : WM_LBUTTONUP;
                sendMessageToClientArea(hwnd, uClientMsg, lParam);
                return 0;
            }
            return DefSubclassProc(hwnd, uMsg, wParam, lParam);
        case WM_SIZE:
        case WM_SIZING:
            isResizing = true;
            return DefSubclassProc(hwnd, uMsg, wParam, lParam);
        case WM_ENTERSIZEMOVE: {
            SetTimer(hwnd, resizeTimer, USER_TIMER_MINIMUM, nullptr);
            isResizing = false;
            return 0;
        }
        case WM_EXITSIZEMOVE: {
            KillTimer(hwnd, resizeTimer);
            isResizing = false;
            return 0;
        }
        case WM_TIMER: {
            auto *base = static_cast<windowing::VulkanWindow *>(glfwGetWindowUserPointer(window));
            if(isResizing) {
                printf("Test");
                base->recreateSwapchain();
            }
            base->onWindowRender();
            return DefSubclassProc(hwnd, uMsg, wParam, lParam);
        }
        default: {
            return DefSubclassProc(hwnd, uMsg, wParam, lParam);
        }
    }
}

LRESULT Win32CustomTitlebar::getBorderlessHitTest(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) const {
    POINT cursorPos{};
    GetCursorPos(&cursorPos);

    RECT windowRect{};
    GetWindowRect(hwnd, &windowRect);

    // Control Elements (box, space, etc.)
    // Order: control box > app icon > extra left reserved space > extra right reserved space
    if (cursorPos.y >= windowRect.top && cursorPos.y < windowRect.top + properties.height) {
        if (cursorPos.x > windowRect.right - 8 - properties.controlBoxWidth && cursorPos.x <= windowRect.right - 8) {
            // Control Box
            if (cursorPos.x > windowRect.right - 8 - (properties.controlBoxWidth / 3)) {
                return HTCLOSE;
            } else if (cursorPos.x > windowRect.right - 8 - (properties.controlBoxWidth / 3) * 2) {
                return HTMAXBUTTON;
            } else if (cursorPos.x > windowRect.right - 8 - properties.controlBoxWidth) {
                return HTMINBUTTON;
            }
            return HTNOWHERE;
        } else if (cursorPos.x >= windowRect.left + 8 && cursorPos.x < windowRect.left + properties.iconWidth + 8) {
            // App Icon
            return HTNOWHERE;
        } else if (cursorPos.x >= windowRect.left + 8 + properties.iconWidth &&
                   cursorPos.x < windowRect.left + properties.iconWidth + 8 + properties.extraLeftReservedWidth) {
            // Extra left reserved space
            return HTNOWHERE;
        } else if (cursorPos.x > windowRect.right - 8 - properties.controlBoxWidth - properties.extraRightReservedWidth
                   && cursorPos.x <= windowRect.right - 8 - properties.controlBoxWidth) {
            // Extra right reserved space
            return HTNOWHERE;
        }
    }

    if (cursorPos.y >= windowRect.top && cursorPos.y < windowRect.top + properties.frameBorderThickness
        && cursorPos.x >= windowRect.left + 8 && cursorPos.x <= windowRect.right - 8) {
        return HTTOP;
    }

    if (cursorPos.y >= windowRect.top && cursorPos.y < windowRect.top + properties.height
        && cursorPos.x >= windowRect.left + 8 && cursorPos.x <= windowRect.right - 8) {
        return HTCAPTION;
    }
    return HTNOWHERE;
}

LRESULT Win32CustomTitlebar::getWmNcCalcSize(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (wParam) {
        auto *ncParams = reinterpret_cast<NCCALCSIZE_PARAMS *>(lParam);
        ncParams->rgrc[0].left += properties.frameBorderThickness;
        ncParams->rgrc[0].bottom -= properties.frameBorderThickness;
        ncParams->rgrc[0].right -= properties.frameBorderThickness;

        if (IsZoomed(hwnd)) {
            ncParams->rgrc[0].top += properties.frameBorderThickness;
        }
    }
    return 0;
}

void Win32CustomTitlebar::sendMessageToClientArea(HWND hwnd, int uMsg, LPARAM lParam) {
    SendMessage(hwnd, uMsg, 0, getScreenToWindowCoordinates(hwnd, lParam));
}

LRESULT Win32CustomTitlebar::getScreenToWindowCoordinates(HWND hwnd, LPARAM lParam) {
    RECT rcWindow;
    GetWindowRect(hwnd, &rcWindow);
    int x = GET_X_LPARAM(lParam) - rcWindow.left;
    int y = GET_Y_LPARAM(lParam) - rcWindow.top;
    return MAKELONG(x, y);
}


