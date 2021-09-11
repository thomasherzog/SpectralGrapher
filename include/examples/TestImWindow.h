#ifndef SPECTRALGRAPHER_TESTIMWINDOW_H
#define SPECTRALGRAPHER_TESTIMWINDOW_H

#include "windowing/ImGuiWindow.h"

class TestImWindow : public windowing::ImGuiWindow {
public:
    TestImWindow();

    void onImGuiFrameRender() override;

private:



};


#endif //SPECTRALGRAPHER_TESTIMWINDOW_H
