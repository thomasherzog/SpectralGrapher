#ifndef SPECTRALGRAPHER_SETTINGSPOPUP_H
#define SPECTRALGRAPHER_SETTINGSPOPUP_H

#include "ui/editor/popups/Popup.h"

#include "graphics/vulkan/core/Context.h"
#include "imgui.h"

class SettingsPopup : public Popup {
public:
    SettingsPopup();

    ~SettingsPopup();

    void renderPopup(std::shared_ptr<vulkan::Context> &context);

};


#endif //SPECTRALGRAPHER_SETTINGSPOPUP_H
