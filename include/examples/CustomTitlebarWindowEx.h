#ifndef SPECTRALGRAPHER_CUSTOMTITLEBARWINDOWEX_H
#define SPECTRALGRAPHER_CUSTOMTITLEBARWINDOWEX_H

#include "windowing/BaseWindow.h"
#include "native/Win32CustomTitlebar.h"

class CustomTitlebarWindowEx  : public windowing::BaseWindow {
public:
    CustomTitlebarWindowEx();

    ~CustomTitlebarWindowEx();

    void onWindowRender() override;

protected:

private:
    static std::vector<std::tuple<int, int>> getRequiredWindowHints();

    Win32CustomTitlebar titlebar{window};
};


#endif //SPECTRALGRAPHER_CUSTOMTITLEBARWINDOWEX_H
