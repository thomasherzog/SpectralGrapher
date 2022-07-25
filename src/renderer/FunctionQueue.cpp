#include "renderer/FunctionQueue.h"


void FunctionQueue::push_function(std::function<void()> &&function) {
    functions.push_back(function);
}

void FunctionQueue::flush() {
    for (auto it = functions.rbegin(); it != functions.rend(); it++) {
        (*it)();
    }
    functions.clear();
}