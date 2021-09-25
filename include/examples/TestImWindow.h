#ifndef SPECTRALGRAPHER_TESTIMWINDOW_H
#define SPECTRALGRAPHER_TESTIMWINDOW_H

#include "windowing/ImGuiWindow.h"

#include <implot.h>

class TestImWindow : public windowing::ImGuiWindow {
public:
    TestImWindow();

    ~TestImWindow();

    void onImGuiFrameRender() override;

private:
  ImPlotContext* implotContext;


};


#endif //SPECTRALGRAPHER_TESTIMWINDOW_H
