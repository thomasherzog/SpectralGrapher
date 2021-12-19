#include <iostream>
#include "windowing/WindowManager.h"
#include "graphics/vulkan/core/SharedContext.h"

#include "examples/SimpleRasterizerWindowEx.h"

int main() {
    auto manager = windowing::WindowManager::getInstance();
    manager->addWindow(std::make_shared<SimpleRasterizerWindowEx>());
    manager->startManager();
    return 0;
}
