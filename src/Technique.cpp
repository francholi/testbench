#include <iostream>

#include "Technique.hpp"
#include "Base/Renderer.hpp"

Technique::Technique() {
    
}

Technique::~Technique() {
    std::cout << "destroyed technique" << std::endl;
}

void Technique::enable(Renderer* renderer) {
    // better to delegate the render state to the renderer, it can be
    // more clever about changes with current render state set.
    renderer->setRenderState(renderState);
    material->enable();
}