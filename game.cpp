#include "./game.h"


#include <random>

int window_width = 640;
int window_height = 480;


typedef struct square_t{
    int vert_indices[8];
    int my_direction;
} Square; 


static std::vector<Square> squares(1, {{-1,-1,-1,-1,-1,-1,-1,-1},-1});



enum directions {UP, DOWN, LEFT, RIGHT};

static float last_square_vert[8] = {0.0f, 0.0f,
                             0.125f, 0.0f,
                             0.125f,0.125f,
                             0.0f,0.125f };

static std::vector<float> food_verts;
static int prev_direction;

static bool hit = false;
static bool resetGameFlag = false;
static bool paused = false;

static float x_norm = 0;
static float y_norm = 0;

static int direction = UP;
static int num_squares = 0;
static int score = 0;
// static std::deque<Square> debug_squares_queue;
static int debug_squares = 0;

static uint32_t grid_size = window_width < window_height ? window_width : window_height;
static uint32_t sections = 16;



// structure of a square


/*
      
  (0,1)---(1,1)
  |         |
  |         |
  |         |
  (0,0)---(1,0)

*/

void createNewSquare(RenderInfo &r_info){

        std::vector<float> new_shape =
        {
         last_square_vert[0],last_square_vert[1],
         last_square_vert[2],last_square_vert[3],
         last_square_vert[4],last_square_vert[5],
         last_square_vert[6],last_square_vert[7]
        };

    Square new_square = {
                         {0 + (num_squares * 8),
                          1 + (num_squares * 8),
                          2 + (num_squares * 8),
                          3 + (num_squares * 8),
                          4 + (num_squares * 8),
                          5 + (num_squares * 8),
                          6 + (num_squares * 8),
                          7 + (num_squares * 8)},
                         prev_direction};
    
    squares.push_back(new_square);
    
    for(auto i : new_shape) r_info.vertices.push_back(i);

    size_t prev_indices_size = r_info.indices.size();
    for(size_t i = prev_indices_size - 6; i < prev_indices_size; ++i){
        r_info.indices.push_back(r_info.indices[i] + 4);
    }


    r_info.num_shapes += 2;

    r_info.num_indices += 6;

    num_squares += 1;

}

void generateRandomSquare(RenderInfo &r_info){
    std::random_device rd;
    std::uniform_int_distribution<int> dist((-1 * ((sections / 2) - 1)),
                                            (sections / 2) - 1);

    bool inside_snake = true;
    std::vector<float> new_shape;
    while(inside_snake){
    
        float rand_x = (float) dist(rd) * x_norm;
        float rand_y = (float) dist(rd) * y_norm;

        std::cout << "generated at " << rand_x << "," << rand_y << std::endl;        
        new_shape = {
                                        rand_x + (0 * x_norm), rand_y + (0 * y_norm),
                                        rand_x + (1 * x_norm), rand_y + (0 * y_norm),
                                        rand_x + (1 * x_norm), rand_y + (1 * y_norm),
                                        rand_x + (0 * x_norm), rand_y + (1 * y_norm)

        };

        for(int i = 1; i < squares.size(); ++i){

            Square square = squares[i];

            std::vector<float> curr_square_verts =
                {
                 r_info.vertices[square.vert_indices[0]],
                 r_info.vertices[square.vert_indices[1]],
                 r_info.vertices[square.vert_indices[2]],
                 r_info.vertices[square.vert_indices[3]],
                 r_info.vertices[square.vert_indices[4]],
                 r_info.vertices[square.vert_indices[5]],
                 r_info.vertices[square.vert_indices[6]],
                 r_info.vertices[square.vert_indices[7]]
            };

            if(curr_square_verts == new_shape) break;
            if(i == squares.size() - 1) inside_snake = false;

        }


    }

    r_info.vertices[1] = new_shape[1];
    r_info.vertices[3] = new_shape[3];
    r_info.vertices[5] = new_shape[5];
    r_info.vertices[7] = new_shape[7];        

    r_info.vertices[0] = new_shape[0];
    r_info.vertices[2] = new_shape[2];
    r_info.vertices[4] = new_shape[4];
    r_info.vertices[6] = new_shape[6];

    food_verts = new_shape;
    
    return;

}


RenderInfo gameSetup(void){
    // set up grid according to window height and width

    // find the smaller of the two dimensions

    grid_size /= sections; // divide the grid into 16 sections

    float base_x = 1.0;
    float base_y = 1.0;

    float new_x = window_width < window_height ?
                                 base_x : base_x * (float)((float)window_height / (float)window_width);
    float new_y = window_width < window_height ?
                                 base_y * (float)((float)window_width / (float)window_height) : base_y;
    float new_x_norm = new_x / (sections / 2);
    float new_y_norm = new_y / (sections / 2);

    x_norm = new_x_norm;
    y_norm = new_y_norm;


    std::vector<float> vertices(8,-1.0);
    
    std::vector<float> snake_vertices =
        {
         0 * new_x_norm, 0 * new_y_norm,
         1 * new_x_norm, 0 * new_y_norm,
         1 * new_x_norm, 1 * new_y_norm,
         0 * new_x_norm, 1 * new_y_norm,
        };

    for(auto i : snake_vertices) vertices.push_back(i);


    // Triangle Vertex positions
    
    std::vector<unsigned int> indices =
        {
         0, 1, 2,
         0, 3, 2
        };


    
    uint32_t num_shapes = 2;
    uint32_t num_verts = 4;
    uint32_t num_indices = 6;
    

    RenderInfo r_info = {nullptr, vertices, indices, num_shapes, num_verts, num_indices, 
                         new_x_norm, new_y_norm};

    r_info.num_shapes += 2;
    r_info.num_indices += 6;
    
    size_t prev_indices_size = r_info.indices.size();
    for(size_t i = prev_indices_size - 6; i < prev_indices_size; ++i){
        r_info.indices.push_back(r_info.indices[i] + 4);
    }
    
    num_squares = 2;

    std::vector<Square> new_squares(1, {{-1,-1,-1,-1,-1,-1,-1,-1},-1});
    
    squares = new_squares;
    
    Square first_square = {{8,9,10,11,12,13,14,15}, direction};
    
    squares.push_back(first_square);

    generateRandomSquare(r_info);

    score = 0;

    return r_info;
}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_UP && action == GLFW_PRESS){
        std::cout << '^' << std::endl << '|' << std::endl;
        direction = UP;
    }
    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS){
        std::cout << '|' << std::endl << 'v' << std::endl;
        direction = DOWN;
    }
    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS){
        std::cout << '<' << '-' << std::endl;
        direction = LEFT;
    }
    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS){
        std::cout << '-' << '>' << std::endl;
        direction = RIGHT;
    }

    if (key == GLFW_KEY_A && action == GLFW_PRESS){
        std::cout << "A button pressed" << std::endl;
        debug_squares += 1;
    }
    if (key == GLFW_KEY_B && action == GLFW_PRESS){
        std::cout << "B button pressed" << std::endl;
        hit = true;
    }
    if (key == GLFW_KEY_R && action == GLFW_PRESS){
        std::cout << "R button pressed, Resetting Game" << std::endl;
        resetGameFlag = true;
    }
    if (key == GLFW_KEY_P && action == GLFW_PRESS){
        std::cout << "R button pressed, Toggling Pause" << std::endl;
        paused = !paused;
    }    
}



void updateGameState(RenderInfo &r_info){


    // check things

    // check if hit food
    std::vector<float> curr_head_verts =
        {
         r_info.vertices[squares[1].vert_indices[0]],
         r_info.vertices[squares[1].vert_indices[1]],
         r_info.vertices[squares[1].vert_indices[2]],
         r_info.vertices[squares[1].vert_indices[3]],
         r_info.vertices[squares[1].vert_indices[4]],
         r_info.vertices[squares[1].vert_indices[5]],
         r_info.vertices[squares[1].vert_indices[6]],
         r_info.vertices[squares[1].vert_indices[7]]
    };
    
    if(curr_head_verts == food_verts) hit = true;    
    
    if(hit) {
        generateRandomSquare(r_info);
        
    }
    // move things
    if(!paused) {   
        prev_direction = squares[1].my_direction;
        squares[1].my_direction = direction;
        for(int i = 1; i < squares.size(); ++i){

            Square square = squares[i];

            if(i == squares.size() - 1){
                last_square_vert[0] = r_info.vertices[square.vert_indices[0]];
                last_square_vert[1] = r_info.vertices[square.vert_indices[1]];
                last_square_vert[2] = r_info.vertices[square.vert_indices[2]];
                last_square_vert[3] = r_info.vertices[square.vert_indices[3]];
                last_square_vert[4] = r_info.vertices[square.vert_indices[4]];
                last_square_vert[5] = r_info.vertices[square.vert_indices[5]];
                last_square_vert[6] = r_info.vertices[square.vert_indices[6]];
                last_square_vert[7] = r_info.vertices[square.vert_indices[7]];
            }
            switch (square.my_direction){
            case UP:
                r_info.vertices[square.vert_indices[1]] += y_norm;
                r_info.vertices[square.vert_indices[3]] += y_norm;
                r_info.vertices[square.vert_indices[5]] += y_norm;
                r_info.vertices[square.vert_indices[7]] += y_norm;
                break;
            case DOWN:
                r_info.vertices[square.vert_indices[1]] -= y_norm;
                r_info.vertices[square.vert_indices[3]] -= y_norm;
                r_info.vertices[square.vert_indices[5]] -= y_norm;
                r_info.vertices[square.vert_indices[7]] -= y_norm;        
                break;
            case LEFT:
                r_info.vertices[square.vert_indices[0]] -= x_norm;
                r_info.vertices[square.vert_indices[2]] -= x_norm;
                r_info.vertices[square.vert_indices[4]] -= x_norm;
                r_info.vertices[square.vert_indices[6]] -= x_norm;        
                break;            
            case RIGHT:
                r_info.vertices[square.vert_indices[0]] += x_norm;
                r_info.vertices[square.vert_indices[2]] += x_norm;
                r_info.vertices[square.vert_indices[4]] += x_norm;
                r_info.vertices[square.vert_indices[6]] += x_norm;        
                break;        
            }

            int temp = squares[i].my_direction;
            squares[i].my_direction = prev_direction;
            prev_direction = temp;
        
        }

        // debug stuff

        if(debug_squares > 0 || hit == true){
            if(hit == true) {
                ++score;
                std::cout << "Score: " << score << std::endl;
            }
            hit = false;
            createNewSquare(r_info);
            debug_squares--;
        
        }

    }
    
    if(resetGameFlag){
        r_info = gameSetup();
        resetGameFlag = false;
    }

}

/*
Random square generation experiment
 */
// void updateGameState(RenderInfo &r_info){

//     std::random_device rd;
//     std::uniform_real_distribution<float> dist(-1.0, 1.0);

//     float rand_x = dist(rd);
//     float rand_y = dist(rd);

//     std::cout << rand_x << "," << rand_y << std::endl;
//     std::vector<float> new_shape = {
//          rand_x + (0 * r_info.x_norm), rand_y + (0 * r_info.y_norm),
//          rand_x + (1 * r_info.x_norm), rand_y + (0 * r_info.y_norm),
//          rand_x + (1 * r_info.x_norm), rand_y + (1 * r_info.y_norm),
//          rand_x + (0 * r_info.x_norm), rand_y + (1 * r_info.y_norm)

//     };
//     for(auto i : new_shape) r_info.vertices.push_back(i);

//     size_t prev_indices_size = r_info.indices.size();
//     for(size_t i = prev_indices_size - 6; i < prev_indices_size; ++i){
//         r_info.indices.push_back(r_info.indices[i] + 4);
//     }
    
//     r_info.num_shapes += 2;
//     // r_info.num_verts += 4;
//     r_info.num_indices += 6;
//     //r_info.num_draws++;

//     return;
    
        
// }
