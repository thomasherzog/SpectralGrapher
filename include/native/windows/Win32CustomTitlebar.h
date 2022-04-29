#ifndef SPECTRALGRAPHER_WIN32CUSTOMTITLEBAR_H
#define SPECTRALGRAPHER_WIN32CUSTOMTITLEBAR_H


#include <windows.h>
#include <CommCtrl.h>
#include <dwmapi.h>
#include <iostream>
#include <WinUser.h>
#include <Uxtheme.h>
#include <vssym32.h>
#include <wingdi.h>
#include <windowsx.h>

#define GLFW_EXPOSE_NATIVE_WIN32

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

struct TitlebarProperties {
    int height = 27;
    int controlBoxWidth = 150;
    int iconWidth = 30;
    int extraLeftReservedWidth = 150;
    int extraRightReservedWidth = 30;
    int frameBorderThickness = 8;
};

class Win32CustomTitlebar {
public:
    explicit Win32CustomTitlebar(GLFWwindow *window, TitlebarProperties properties);

    ~Win32CustomTitlebar();

private:

    static LRESULT
    CALLBACK staticCustomSubclass(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass,
                                  DWORD_PTR dwRefData);

    LRESULT customSubclass(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass);

    LRESULT getBorderlessHitTest(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) const;

    LRESULT getWmNcCalcSize(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    void sendMessageToClientArea(HWND hwnd, int uMsg, LPARAM lParam);

    static LRESULT getScreenToWindowCoordinates(HWND hwnd, LPARAM lParam);

    GLFWwindow *window;

    TitlebarProperties properties;

    HBRUSH brush;

    UINT resizeTimer;

    bool isResizing{false};

};


#endif //SPECTRALGRAPHER_WIN32CUSTOMTITLEBAR_H
