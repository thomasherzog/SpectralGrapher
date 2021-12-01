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

#define GLFW_EXPOSE_NATIVE_WIN32

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

class Win32CustomTitlebar {
public:
    explicit Win32CustomTitlebar(GLFWwindow *window);

    static LRESULT CALLBACK myProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

    static LRESULT getBorderlessHitTest(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:

};


#endif //SPECTRALGRAPHER_WIN32CUSTOMTITLEBAR_H
