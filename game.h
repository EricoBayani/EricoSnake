#pragma once
#include <iostream>
#include <string>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <vector>


#include "Renderer.h"

// window characteristics
extern int window_width;
extern int window_height;


// struct containing vertices to render
typedef struct RenderInfo{

    GLFWwindow* window;
    
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    uint32_t num_shapes;
    uint32_t num_verts;
    uint32_t num_indices;
    // uint32_t num_draws;
    float x_norm;
    float y_norm;
    
} RenderInfo;

// returns render_struct that graphics functions in main() will render
RenderInfo gameSetup(void);

// updates positions based on directions
void updateGameState(RenderInfo &r_info);

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
