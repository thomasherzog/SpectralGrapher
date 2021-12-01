#include "examples/CustomTitlebarWindowEx.h"


CustomTitlebarWindowEx::CustomTitlebarWindowEx() : BaseWindow("This is a window title", getRequiredWindowHints()) {

}

CustomTitlebarWindowEx::~CustomTitlebarWindowEx() {

}

void CustomTitlebarWindowEx::onWindowRender() {

}

std::vector<std::tuple<int, int>> CustomTitlebarWindowEx::getRequiredWindowHints() {
    return {};
}
