/**
* Author: Tanzia Nur
* Assignment: Lunar Lander
* Date due: 2024-10-27, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define LOG(argument) std::cout << argument << '\n'
#define STB_IMAGE_IMPLEMENTATION
#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define PLATFORM_COUNT 10

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "Entity.h"
#include <vector>
#include <ctime>
#include "cmath"

// ————— CONSTANTS ————— //
constexpr int WINDOW_WIDTH = 640,
WINDOW_HEIGHT = 480;

constexpr float BG_RED = 0.1f,
BG_GREEN = 0.2f,
BG_BLUE = 0.4f,
BG_OPACITY = 1.0f;

constexpr int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

constexpr float MILLISECONDS_IN_SECOND = 1000.0;
constexpr char WITCH_FILEPATH[] = "assets/witch.png",
LOSE_PLATFORM_FILEPATH[] = "assets/lava.jpg",
WIN_PLATFORM_FILEPATH[] = "assets/rocky.png",
WIN_MSG_FILEPATH[] = "assets/mission_passed.png",
LOSE_MSG_FILEPATH[] = "assets/mission_failed.png";

constexpr glm::vec3 KITA_INIT_SCALE = glm::vec3(2.0f, 3.8621f, 0.0f);

constexpr GLint NUMBER_OF_TEXTURES = 1,
LEVEL_OF_DETAIL = 0,
TEXTURE_BORDER = 0;

constexpr int NUMBER_OF_NPCS = 3;

// ————— STRUCTS AND ENUMS —————//
enum AppStatus { RUNNING, TERMINATED };
enum FilterType { NEAREST, LINEAR };

struct GameState
{
    Entity* player;
    Entity* platforms;
    Entity* win_plat;
    Entity* win_msg;
    Entity* lose_msg;
};

// ————— VARIABLES ————— //
GameState g_game_state;

SDL_Window* g_display_window;
AppStatus g_app_status = RUNNING;

bool player_win = false;
bool player_lose = false;

ShaderProgram g_shader_program;
glm::mat4 g_view_matrix, g_projection_matrix;

float g_previous_ticks = 0.0f;
float g_time_accumulator = 0.0f;

void initialise();
void process_input();
void update();
void render();
void shutdown();

GLuint load_texture(const char* filepath);

// ———— GENERAL FUNCTIONS ———— //
GLuint load_texture(const char* filepath)
{
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        std::cout << filepath;
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }

    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    stbi_image_free(image);

    return textureID;
}


void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("Lunar Lander!",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

    if (g_display_window == nullptr)
    {
        std::cerr << "Error: SDL window could not be created.\n";
        shutdown();
    }

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());

    glClearColor(BG_RED, BG_GREEN, BG_BLUE, BG_OPACITY);

    // ————— PLAYER ————— //
    GLuint player_texture_id = load_texture(WITCH_FILEPATH);

    int player_flying_animation[4][3] =
    {
        { 9, 10, 11 },  // for witch to move to the left,
        { 3, 4, 5}, // for witch to move to the right,
        { 0, 1, 2 }, // for witch to move upwards,
        { 6, 7, 8 }   // for witch to move downwards
    };

    g_game_state.player = new Entity(
        player_texture_id,         // texture id
        1.0f,                      // speed
        player_flying_animation,  // animation index sets
        0.0f,                      // animation time
        4,                         // animation frame amount
        0,                         // current animation index
        3,                         // animation column amount
        4                          // animation row amount
    );

    g_game_state.player->face_down();
    g_game_state.player->set_position(glm::vec3(0.0f, 2.0f, 0.0f));

    // ————— PLATFORM ————— //
    g_game_state.platforms = new Entity[PLATFORM_COUNT];
    //GLuint lose_platform_texture_id = load_texture(LOSE_PLATFORM_FILEPATH);

    for (int i = 0; i < 4; i++)
    {
        g_game_state.platforms[i].m_texture_id = load_texture(LOSE_PLATFORM_FILEPATH);
        g_game_state.platforms[i].set_position(glm::vec3(i - 5.0f, -3.5f, 0.0f));
        g_game_state.platforms[i].update(0.0f, NULL, 0);
        g_game_state.platforms[i].set_entity_type(TRAP_PLATFORM);

    }
    for (int i = 4; i < 10; i++)
    {
        g_game_state.platforms[i].m_texture_id = load_texture(LOSE_PLATFORM_FILEPATH);
        g_game_state.platforms[i].set_position(glm::vec3(i - 4.0f, -3.5f, 0.0f));
        g_game_state.platforms[i].update(0.0f, NULL, 0);
        g_game_state.platforms[i].set_entity_type(TRAP_PLATFORM);
    }

    g_game_state.win_plat = new Entity();
    g_game_state.win_plat->m_texture_id = load_texture(WIN_PLATFORM_FILEPATH);
    g_game_state.win_plat->set_position(glm::vec3(-1.0f, -3.5f, 0.0f));
    g_game_state.win_plat->update(0.0f, NULL, 0);
    g_game_state.win_plat->set_entity_type(WIN_PLATFORM);

    g_game_state.win_msg = new Entity();
    g_game_state.win_msg->m_texture_id = load_texture(WIN_MSG_FILEPATH);
    g_game_state.win_msg->set_position(glm::vec3(0.0f, 0.0f, 0.0f));
    g_game_state.win_msg->set_scale(glm::vec3(5.0f, 1.0f, 0.0f));
    g_game_state.win_msg->update(0.0f, NULL, 0);

    
    g_game_state.lose_msg = new Entity();
    g_game_state.lose_msg->m_texture_id = load_texture(LOSE_MSG_FILEPATH);
    g_game_state.lose_msg->set_position(glm::vec3(0.0f, 0.0f, 0.0f));
    g_game_state.lose_msg->set_scale(glm::vec3(5.0f, 1.0f, 0.0f));
    g_game_state.lose_msg->update(0.0f, NULL, 0);
    

    // ————— GENERAL ————— //
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    // VERY IMPORTANT: If nothing is pressed, we don't want to go anywhere
    g_game_state.player->set_movement(glm::vec3(0.0f));

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            g_app_status = TERMINATED;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_q: g_app_status = TERMINATED;
            default:     break;
            }

        default:
            break;
        }
    }

    const Uint8* key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_LEFT])
    {
        g_game_state.player->move_left();
    }
    else if (key_state[SDL_SCANCODE_RIGHT])
    {
        g_game_state.player->move_right();
    }

    if (key_state[SDL_SCANCODE_UP])
    {
        g_game_state.player->move_up();
    }

    

    if (glm::length(g_game_state.player->get_movement()) > 1.0f)
        g_game_state.player->normalise_movement();
}


void update()
{
    // ————— DELTA TIME ————— //
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND; // get the current number of ticks
    float delta_time = ticks - g_previous_ticks; // the delta time is the difference from the last frame
    g_previous_ticks = ticks;

    // ————— FIXED TIMESTEP ————— //
    // STEP 1: Keep track of how much time has passed since last step

    delta_time += g_time_accumulator;

    // STEP 2: Accumulate the ammount of time passed while we're under our fixed timestep
    if (delta_time < FIXED_TIMESTEP)
    {
        g_time_accumulator = delta_time;
        return;
    }


    // STEP 3: Once we exceed our fixed timestep, apply that elapsed time into the objects' update function invocation
    while (delta_time >= FIXED_TIMESTEP)
    {
        // Notice that we're using FIXED_TIMESTEP as our delta time
        g_game_state.player->update(FIXED_TIMESTEP, g_game_state.platforms, PLATFORM_COUNT);
        if (g_game_state.player->check_collision(g_game_state.win_plat)) {
            player_win = true;
            //g_game_state.win_plat->activate();

        }
        for (int i = 0; i < PLATFORM_COUNT; i++) {
            if (g_game_state.platforms[i].collided) {
                player_lose = true;
                //g_game_state.platforms[i].activate();
            }
        }


        delta_time -= FIXED_TIMESTEP;
    }
    if (player_win) {
        g_game_state.player->set_movement(glm::vec3(0.0f, 0.0f, 0.0f));  // Stop movement on win
        g_game_state.player->set_velocity(glm::vec3(0.0f, 0.0f, 0.0f));  // Zero velocity
        g_game_state.player->set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));  // Zero acc
        g_game_state.player->set_position(glm::vec3(0.0f, 2.0f, 0.0f));
        // Display win message or perform other win actions
    }
    // Handle lose condition
    else if (player_lose) {
        g_game_state.player->set_movement(glm::vec3(0.0f, 0.0f, 0.0f));  // Stop movement on lose
        g_game_state.player->set_velocity(glm::vec3(0.0f, 0.0f, 0.0f));  // Zero velocity
        g_game_state.player->set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));  // Zero acc
        g_game_state.player->set_position(glm::vec3(0.0f, 2.0f, 0.0f));
        // Display lose message or perform other lose actions
    }


    g_time_accumulator = delta_time;
}


void render()
{
    // ————— GENERAL ————— //
    glClear(GL_COLOR_BUFFER_BIT);

    // ————— PLAYER ————— //
    g_game_state.player->render(&g_shader_program);

    // ————— PLATFORM ————— //
    for (int i = 0; i < PLATFORM_COUNT; i++) {

        if (g_game_state.platforms[i].is_active()) {
            g_game_state.platforms[i].render(&g_shader_program);
        }
    }

    if (g_game_state.win_plat->is_active()) {
        g_game_state.win_plat->render(&g_shader_program);
    }
   
    if (player_win) {
        g_game_state.win_msg->render(&g_shader_program);
    }
    else if (player_lose) {
        g_game_state.lose_msg->render(&g_shader_program);
    }

    // ————— GENERAL ————— //
    SDL_GL_SwapWindow(g_display_window);
}


void shutdown()
{
    SDL_Quit();
}


int main(int argc, char* argv[])
{
    initialise();

    while (g_app_status == RUNNING)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}