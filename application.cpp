#include "./game.h"
#include "Renderer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"

#include <windows.h>

struct ShaderProgramSource{
    std::string VertexSource;
    std::string FragmentSource;
};

static struct ShaderProgramSource ParseShader(const std::string& filepath){
    std::ifstream stream(filepath);

    enum class ShaderType
        {
         NONE = -1,
         VERTEX = 0,
         FRAGMENT = 1
        };
    
    std::string line;
    std::stringstream ss[2];
    ShaderType type = ShaderType::NONE;
    while (getline(stream, line)){
        if(line.find("#shader") != std::string::npos){
            if(line.find("vertex") != std::string::npos)
                // set mode to vertex
                type = ShaderType::VERTEX;
            else if (line.find("fragment") != std::string::npos)
                // set mode to fragment
                type = ShaderType::FRAGMENT;
        }
        else{
            ss[(int)type] << line << '\n';
        }
    }
    return {ss[0].str(), ss[1].str()};
}

static unsigned int CompileShader(unsigned int type, const std::string& source)
{
    unsigned int id = GLCallR(glCreateShader(type));
    // Source string needs to exist 
    const char* src = source.c_str();
    GLCall(glShaderSource(id, 1, &src, nullptr));
    GLCall(glCompileShader(id));

    int result;
    GLCall(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
    if (result == GL_FALSE){

        int length;
        GLCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
        // alloca() is a function that lets you dynamically
        // create things on the stack, since you can't
        // do that normally
        char* message = (char*)alloca(length * sizeof(char));
        GLCall(glGetShaderInfoLog(id, length, &length, message));
        std::cout << "Failed to compile shader!" <<
            (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << std::endl;
        std::cout << message << std:: endl;
        GLCall(glDeleteShader(id));
        return 0;
    }
    
    

    return id;
}

static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
    unsigned int program = GLCallR(glCreateProgram());
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    GLCall(glAttachShader(program, vs));
    GLCall(glAttachShader(program, fs));

    // look up in documentation
    GLCall(glLinkProgram(program));
    GLCall(glValidateProgram(program));

    GLCall(glDeleteShader(vs));
    GLCall(glDeleteShader(fs));

    return program;
    
}


int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    // ensuring openGL core profile used
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(window_width, window_height, "Snake", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glfwSwapInterval(1);

    // Init GLEW
    if (glewInit() != GLEW_OK)
    {
        std::cout << "Error!" << std::endl;
    }

    std::cout << glGetString(GL_VERSION) << std::endl;

    {
        // SETTING UP SHAPES AND GRID
        RenderInfo info = gameSetup();
        glfwSetKeyCallback(window, key_callback);
        info.window = window;
        std::vector<float> vertices = info.vertices;
        std::vector<unsigned int> indices = info.indices;
        uint32_t num_shapes = info.num_shapes;
        uint32_t num_verts = info.num_verts;
        uint32_t num_indices = info.num_indices;
        
        // END OF SHAPES SET UP

        unsigned int vao;
        GLCall(glGenVertexArrays(1, &vao));
        GLCall(glBindVertexArray(vao));


        // docs.gl is a great openGL documentation site

        // Making a single vertex buffer     
        VertexBuffer vb(info.vertices.data(), info.num_shapes * info.num_verts * sizeof(float));


        GLCall(glEnableVertexAttribArray(0));
        GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0));

        IndexBuffer ib(info.indices.data(), info.num_indices);


        // In visual studio debugger, the path is relative to the project directory
        struct ShaderProgramSource source = ParseShader("res/shaders/basic.glsl");


        unsigned int shader = CreateShader(source.VertexSource, source.FragmentSource);
        GLCall(glUseProgram(shader));

        int color_location = GLCallR(glGetUniformLocation(shader, "u_Color"));
        ASSERT(color_location != -1);
        GLCall(glUniform4f(color_location, 0.2f, 0.3f, 0.8f, 1.0f));
        float r = 0.0f;
        float color_increment = 0.05f;


        // unbinding buffers
        GLCall(glBindVertexArray(0));
        GLCall(glUseProgram(0));
        GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
        GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0))
        /* Loop until the user closes the window */
        while (!glfwWindowShouldClose(window))
        {



            /* Render here */
            GLCall(glClear(GL_COLOR_BUFFER_BIT));

            // Draw
            // messing with draw function to demonstrate error things
            GLCall(glUseProgram(shader));
            // GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));

            /*
              8/17/22
              Notes to myself:
What am I actually drawing here? I am drawing all the squares that I made in this vertex buffer. 
Maybe I should think about how to separate this draw call in the future to when I want to 
draw different groups of shapes and stuff. This is just an idea for abstracting shapes.

              I should read more into how VAOs work.
             */
            vb.Bind();
            ib.Bind();
            GLCall(glBindVertexArray(vao));
            GLCall(glUniform4f(color_location, r, 0.3f, 0.8f, 1.0f));
            GLCall(glDrawElements(GL_TRIANGLES, info.num_shapes * info.num_verts, GL_UNSIGNED_INT, nullptr));
            //vb.Unbind();
            vb.Bind();
            vb.Update(info.vertices.data(), info.num_shapes * info.num_verts * sizeof(float));
            //ib.Unbind();
            ib.Bind();
            ib.Update(info.indices.data(),  info.num_indices);

            /* Swap front and back buffers */
            GLCall(glfwSwapBuffers(window));

            /* Poll for and process events */
            glfwPollEvents();
            // update game state
            // Sleep(75);
            updateGameState(info);
            Sleep(75);

        }

    GLCall(glDeleteProgram(shader));
    }
    glfwTerminate();
    return 0;
}
