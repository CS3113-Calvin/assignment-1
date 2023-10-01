/**
* Author: Calvin Tian
* Assignment: Simple 2D Scene
* Date due: 2023-09-30, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"

enum Coordinate
{
    x_coordinate,
    y_coordinate
};

#define LOG(argument) std::cout << argument << '\n'

const int WINDOW_WIDTH = 640,
WINDOW_HEIGHT = 480;

const float BG_RED = 0.1922f,
BG_BLUE = 0.549f,
BG_GREEN = 0.9059f,
BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const float MILLISECONDS_IN_SECOND = 1000.0;

const int NUMBER_OF_TEXTURES = 1; // to be generated, that is
const GLint LEVEL_OF_DETAIL = 0;  // base image level; Level n is the nth mipmap reduction image
const GLint TEXTURE_BORDER = 0;   // this value MUST be zero

SDL_Window* g_display_window;
bool g_game_is_running = true;

// Req 1: At least 2 objects
// object 1
const char OBJECT_1_FILEPATH[] = "C:\\Users\\calvi\\source\\repos\\Project_1\\Assets\\calvin.png";
GLuint g_obj1_texture_id;
// object 2
const char OBJECT_2_FILEPATH[] = "C:\\Users\\calvi\\source\\repos\\Project_1\\Assets\\hobbes.png";
GLuint g_obj2_texture_id;

// Req 2: Movement
// overall position
glm::vec3 g_obj1_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_obj2_position = glm::vec3(0.0f, 0.0f, 0.0f);
// movement tracker
glm::vec3 g_obj1_movement = glm::vec3(0.3f, 0.3f, 0.0f);    // up and right
glm::vec3 g_obj2_movement = glm::vec3(-1.0f, -1.0f, 0.0f);  // down and left

// Req 3: Rotation
const float DEGREES_PER_SECOND = 360.0f;
float g_obj2_rotation = 0.0f; // obj2 rotation

// EC: Scaling
float g_obj1_scale = 1.0f;
float g_obj2_scale = 1.0f;

const float GROWTH_FACTOR = 0.5f; // .5% growth per second
const float SHRINK_FACTOR = 0.5f; // .5% shrinkage per second
const float MAX_SCALE = 1.1f;     // grow image up to 110% of original size
const float MIN_SCALE = 0.9f;     // shrink image down to 90% of original size
bool g_is_growing = true;

ShaderProgram g_shader_program;
glm::mat4 view_matrix, m_projection_matrix, m_trans_matrix;
glm::mat4 g_obj1_model_matrix, g_obj2_model_matrix;

float m_previous_ticks = 0.0f;

GLuint load_texture(const char* filepath) {
    // STEP 1: Loading the image file
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL) {
        LOG("Unable to load image. Make sure the path is correct.");
        LOG(filepath);
        assert(false);
    }

    // STEP 2: Generating and binding a texture ID to our image
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(
        GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA,
        width, height,
        TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE,
        image
    );

    // STEP 3: Setting our texture filter parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // STEP 4: Releasing our file from memory and returning our texture id
    stbi_image_free(image);

    return textureID;
}

void initialise() {
    // Initialise video
    SDL_Init(SDL_INIT_VIDEO);

    g_display_window = SDL_CreateWindow("Assignment 1 - Calvin Tian",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_obj1_model_matrix = glm::mat4(1.0f);
    g_obj2_model_matrix = glm::mat4(1.0f);
    view_matrix = glm::mat4(1.0f);  // Defines the position (location and orientation) of the camera
    m_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);  // Defines the characteristics of your camera, such as clip planes, field of view, projection method etc.

    g_shader_program.set_projection_matrix(m_projection_matrix);
    g_shader_program.set_view_matrix(view_matrix);
    // Notice we haven't set our model matrix yet!

    glUseProgram(g_shader_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    g_obj1_texture_id = load_texture(OBJECT_1_FILEPATH);
    g_obj2_texture_id = load_texture(OBJECT_2_FILEPATH);

    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
            g_game_is_running = false;
        }
    }
}

void update() {
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND; // get the current number of ticks
    float delta_time = ticks - m_previous_ticks; // the delta time is the difference from the last frame
    m_previous_ticks = ticks;

    // Reset the model matrix
    g_obj1_model_matrix = glm::mat4(1.0f);
    //g_obj2_model_matrix = glm::mat4(1.0f);

    // Req 2: Movement
    // add             direction       * elapsed time * units per second
    g_obj1_position += g_obj1_movement * delta_time * 1.0f;
    g_obj2_position += g_obj2_movement * delta_time * 1.0f;

    // translate the model matrix by the updated position
    g_obj1_model_matrix = glm::translate(g_obj1_model_matrix, g_obj1_position);
    g_obj2_model_matrix = glm::translate(g_obj1_model_matrix, g_obj2_position);  // move obj2 relative to obj1

    // Req 3: Rotation
    g_obj2_rotation += DEGREES_PER_SECOND * delta_time;
    g_obj2_model_matrix = glm::rotate(g_obj2_model_matrix, glm::radians(g_obj2_rotation), glm::vec3(0.0f, 0.0f, 1.0f));

    // EC: Scaling
    if (g_obj1_scale >= MAX_SCALE) {
        g_is_growing = false;
    }
    else if (g_obj1_scale <= MIN_SCALE) {
        g_is_growing = true;
    }

    if (g_is_growing) {
        g_obj1_scale += GROWTH_FACTOR * delta_time;
    }
    else {
        g_obj1_scale -= SHRINK_FACTOR * delta_time;
    }
    g_obj1_model_matrix = glm::scale(g_obj1_model_matrix, glm::vec3(g_obj1_scale, g_obj1_scale, 1.0f));
}

void draw_object(glm::mat4& object_model_matrix, GLuint& object_texture_id) {
    g_shader_program.set_model_matrix(object_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6); // we are now drawing 2 triangles, so we use 6 instead of 3
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Vertices
    float vertices[] = {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  // triangle 1
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f   // triangle 2
    };

    // Textures
    float texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
    };

    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());

    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    // Bind texture
    draw_object(g_obj1_model_matrix, g_obj1_texture_id);
    draw_object(g_obj2_model_matrix, g_obj2_texture_id);

    // We disable two attribute arrays now
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    SDL_GL_SwapWindow(g_display_window);
}

void shutdown() {
    SDL_Quit();
}

/**
 Start here—we can see the general structure of a game loop without worrying too much about the details yet.
 */
int main(int argc, char* argv[]) {
    initialise();

    while (g_game_is_running) {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}
