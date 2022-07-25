#ifndef SPECTRALGRAPHER_FUNCTIONQUEUE_H
#define SPECTRALGRAPHER_FUNCTIONQUEUE_H

#include <deque>
#include <functional>

class FunctionQueue {
public:
    void push_function(std::function<void()>&& function);

    void flush();

private:
    std::deque<std::function<void()>> functions;

};


#endif //SPECTRALGRAPHER_FUNCTIONQUEUE_H
