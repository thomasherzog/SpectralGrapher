#include "windowing/WindowManager.h"
#include "ui/editor/EditorWindow.h"

int main() {
    auto manager = windowing::WindowManager::getInstance();
    manager->addWindow(std::make_shared<EditorWindow>());
    manager->startManager();
    return 0;
}
