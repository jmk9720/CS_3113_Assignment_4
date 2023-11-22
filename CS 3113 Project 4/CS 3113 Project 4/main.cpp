/**
* Author: Jonathan Kim
* Assignment: Rise of the AI
* Date due: 2023-11-18, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

/*
 There are three AI anemies: guard, jumper, assassin. A guard will chase
 the player once it is in range. The jumper will follow the player until
 it reaches a certain time and crashes down. It will reset for a few
 seconds and will follow the player again. The assassin will move towards
 the player at a faster speed and go back to its original position.
 
 Players can kill each enemy by jumping on top of the enemies. If hit
 anywhere else, the player dies and the game is over. Once all enemies
 are dead, mission is accomplished.
 */

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define ENEMY_COUNT 3
#define LEVEL1_WIDTH 25
#define LEVEL1_HEIGHT 5

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "cmath"
#include <ctime>
#include <vector>
#include "Entity.hpp"
#include "Map.hpp"
using namespace std;

struct GameState
{
    Entity *player;
    Entity *enemies;
    Entity *bullet;
    
    Map *map;
};

const int WINDOW_WIDTH  = 640,
          WINDOW_HEIGHT = 480;

const float BG_RED     = 0.1922f,
            BG_BLUE    = 0.549f,
            BG_GREEN   = 0.9059f,
            BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH  = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char GAME_WINDOW_NAME[] = "Rise of the AI";

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const float MILLISECONDS_IN_SECOND = 1000.0;

const int FONTBANK_SIZE = 16;

const char SPRITESHEET_FILEPATH[] = "assets/images/player.png",
           MAP_TILESET_FILEPATH[] = "assets/images/tile_spritesheet.png",
           ENEMY_FILEPATH[] = "assets/images/enemy.png",
           TEXT_SPRITE_FILEPATH[] = "assets/fonts/font1.png";

const int NUMBER_OF_TEXTURES = 1;
const GLint LEVEL_OF_DETAIL = 0;
const GLint TEXTURE_BORDER = 0;

unsigned int LEVEL_1_DATA[] =
{
      0,   0,   0,   0,   0,   0, 103, 103, 103, 103, 103, 103, 103,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0, 103, 103,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    103, 103,   0,   0,   0,   0,   0,   0, 103, 103, 103, 103, 103, 103,   0,   0,   0,   0,   0,     0,   0,   0,   0,   0,   0,
    152, 152, 103, 103,   0,   0, 103, 103, 152, 152, 152, 152, 152, 152, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103,
    152, 152, 152, 152,   0,   0, 152, 152, 152, 152, 152, 152, 152, 152, 152, 152, 152, 152, 152, 152, 152, 152, 152, 152, 152
};

GameState g_state;

SDL_Window* m_display_window;
bool m_game_is_running = true;

ShaderProgram m_program;
glm::mat4 m_view_matrix, m_projection_matrix, g_text_matrix;

GLuint text_texture_id;

float m_previous_ticks = 0.0f;
float m_accumulator    = 0.0f;

bool double_jump = false;
int death_count = 0;
bool mission = false;

GLuint load_texture(const char* filepath)
{
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);
    
    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }
    
    GLuint texture_id;
    glGenTextures(NUMBER_OF_TEXTURES, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    stbi_image_free(image);
    
    return texture_id;
}

void initialise()
{
    // ————— GENERAL ————— //
    SDL_Init(SDL_INIT_VIDEO);
    m_display_window = SDL_CreateWindow(GAME_WINDOW_NAME,
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL);
    
    SDL_GLContext context = SDL_GL_CreateContext(m_display_window);
    SDL_GL_MakeCurrent(m_display_window, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    // ————— VIDEO SETUP ————— //
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    m_program.Load(V_SHADER_PATH, F_SHADER_PATH);
    
    m_view_matrix = glm::mat4(1.0f);
    m_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    m_program.SetProjectionMatrix(m_projection_matrix);
    m_program.SetViewMatrix(m_view_matrix);
    
    g_text_matrix = glm::mat4(1.0f);
    g_text_matrix = glm::translate(m_view_matrix, glm::vec3(-3.5f, 0.0f, 0.0f));
    
    glUseProgram(m_program.programID);
    
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    // ————— MAP SET-UP ————— //
    GLuint map_texture_id = load_texture(MAP_TILESET_FILEPATH);
    g_state.map = new Map(LEVEL1_WIDTH, LEVEL1_HEIGHT, LEVEL_1_DATA, map_texture_id, 1.0f, 12, 13);
    
    // ————— GEORGE SET-UP ————— //
    // Existing
    g_state.player = new Entity();
    g_state.player->set_entity_type(PLAYER);
    g_state.player->set_position(glm::vec3(0.0f, 0.0f, 0.0f));
    g_state.player->set_movement(glm::vec3(0.0f));
    g_state.player->set_speed(2.5f);
    g_state.player->set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    g_state.player->m_texture_id = load_texture(SPRITESHEET_FILEPATH);
    
    // Walking
//    g_state.player->m_walking[g_state.player->LEFT]  = new int[4] { 1, 5, 9,  13 };
//    g_state.player->m_walking[g_state.player->RIGHT] = new int[4] { 3, 7, 11, 15 };
//    g_state.player->m_walking[g_state.player->UP]    = new int[4] { 2, 6, 10, 14 };
//    g_state.player->m_walking[g_state.player->DOWN]  = new int[4] { 0, 4, 8,  12 };
//
//    g_state.player->m_animation_indices = g_state.player->m_walking[g_state.player->RIGHT];  // start George looking left
//    g_state.player->m_animation_frames = 4;
//    g_state.player->m_animation_index  = 0;
//    g_state.player->m_animation_time   = 0.0f;
//    g_state.player->m_animation_cols   = 4;
//    g_state.player->m_animation_rows   = 4;
    g_state.player->set_height(1.0f);
    g_state.player->set_width(1.0f);
    
    // Jumping
    g_state.player->m_jumping_power = 5.0f;
    
    GLuint enemy_texture_id = load_texture(ENEMY_FILEPATH);
    g_state.enemies = new Entity[ENEMY_COUNT];
    g_state.enemies[ENEMY_COUNT - 3].set_entity_type(ENEMY);
    g_state.enemies[ENEMY_COUNT - 3].set_ai_type(JUMPER);
    g_state.enemies[ENEMY_COUNT - 3].set_ai_state(RESET);
    g_state.enemies[ENEMY_COUNT - 3].m_texture_id = enemy_texture_id;
    g_state.enemies[ENEMY_COUNT - 3].set_position(glm::vec3(2.5f, 3.0f, 0.0f));
    g_state.enemies[ENEMY_COUNT - 3].set_movement(glm::vec3(1.0f));
    g_state.enemies[ENEMY_COUNT - 3].set_speed(0.5f);
    g_state.enemies[ENEMY_COUNT - 3].set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));
    
    g_state.enemies[ENEMY_COUNT - 3].set_height(1.0f);
    g_state.enemies[ENEMY_COUNT - 3].set_width(1.0f);
    
    g_state.enemies[ENEMY_COUNT - 2].set_entity_type(ENEMY);
    g_state.enemies[ENEMY_COUNT - 2].set_ai_type(ASSASSIN);
    g_state.enemies[ENEMY_COUNT - 2].set_ai_state(IDLE);
    g_state.enemies[ENEMY_COUNT - 2].m_texture_id = enemy_texture_id;
    g_state.enemies[ENEMY_COUNT - 2].set_position(glm::vec3(20.0f, 0.0f, 0.0f));
    g_state.enemies[ENEMY_COUNT - 2].set_movement(glm::vec3(0.0f));
    g_state.enemies[ENEMY_COUNT - 2].set_speed(1.0f);
    g_state.enemies[ENEMY_COUNT - 2].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    
    g_state.enemies[ENEMY_COUNT - 2].set_height(1.0f);
    g_state.enemies[ENEMY_COUNT - 2].set_width(1.0f);
    
    g_state.enemies[ENEMY_COUNT - 1].set_entity_type(ENEMY);
    g_state.enemies[ENEMY_COUNT - 1].set_ai_type(GUARD);
    g_state.enemies[ENEMY_COUNT - 1].set_ai_state(IDLE);
    g_state.enemies[ENEMY_COUNT - 1].m_texture_id = enemy_texture_id;
    g_state.enemies[ENEMY_COUNT - 1].set_position(glm::vec3(12.0f, 0.0f, 0.0f));
    g_state.enemies[ENEMY_COUNT - 1].set_movement(glm::vec3(0.0f));
    g_state.enemies[ENEMY_COUNT - 1].set_speed(0.5f);
    g_state.enemies[ENEMY_COUNT - 1].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    
    g_state.enemies[ENEMY_COUNT - 1].set_height(1.0f);
    g_state.enemies[ENEMY_COUNT - 1].set_width(1.0f);
    
    text_texture_id = load_texture(TEXT_SPRITE_FILEPATH);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    g_state.player->set_movement(glm::vec3(0.0f));
    
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                m_game_is_running = false;
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_q:
                        // Quit the game with a keystroke
                        m_game_is_running = false;
                        break;
                        
                    case SDLK_SPACE:
                        // Jump
                        if (g_state.player->m_map_bottom)
                        {
                            g_state.player->m_is_jumping = true;
                            double_jump = true;
                        }
                        else if (double_jump == true) {
                            double_jump = false;
                            g_state.player->m_is_jumping = true;
                        }
                        break;
                        
                    default:
                        break;
                }
                
            default:
                break;
        }
    }
    
    const Uint8 *key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_LEFT])
    {
        g_state.player->m_movement.x = -1.0f;
        g_state.player->m_animation_indices = g_state.player->m_walking[g_state.player->LEFT];
    }
    else if (key_state[SDL_SCANCODE_RIGHT])
    {
        g_state.player->m_movement.x = 1.0f;
        g_state.player->m_animation_indices = g_state.player->m_walking[g_state.player->RIGHT];
    }
    
    // This makes sure that the player can't move faster diagonally
    if (glm::length(g_state.player->m_movement) > 1.0f)
    {
        g_state.player->m_movement = glm::normalize(g_state.player->m_movement);
    }
}

void update()
{
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - m_previous_ticks;
    m_previous_ticks = ticks;
    
    delta_time += m_accumulator;
    
    if (delta_time < FIXED_TIMESTEP)
    {
        m_accumulator = delta_time;
        return;
    }
    if (g_state.player->game_over == false) {
        while (delta_time >= FIXED_TIMESTEP)
        {
            g_state.player->update(FIXED_TIMESTEP, g_state.player, g_state.enemies, ENEMY_COUNT, g_state.map);
            
            for (int i = 0; i < ENEMY_COUNT; i++) {
                g_state.enemies[i].update(FIXED_TIMESTEP, g_state.player, NULL, 0, g_state.map);
                if (g_state.enemies[i].get_dead() == true) {
                    death_count += 1;
                }
                if (death_count == 3) {
                    g_state.player->game_over = true;
                    mission = true;
                }
            }
            if (mission == true) {
                std::cout << "MISSION SUCCESS" << std::endl;
            }
            if (death_count != 3) {
                death_count = 0;
            }
            if (g_state.player->m_enemy_top) {
                std::cout << "TOP" << std::endl;
            }
            if (g_state.player->m_enemy_bottom) {
                std::cout << "BOTTOM" << std::endl;
            }
            delta_time -= FIXED_TIMESTEP;
        }
        m_accumulator = delta_time;
        
        m_view_matrix = glm::mat4(1.0f);
        m_view_matrix = glm::translate(m_view_matrix, glm::vec3(-g_state.player->get_position().x, 0.0f, 0.0f));
        g_text_matrix = glm::mat4(1.0f);
        g_text_matrix = glm::translate(g_text_matrix, glm::vec3(g_state.player->get_position().x - 3.5, 0.0f, 0.0f));
        
    }
    
}

void DrawText(ShaderProgram *program, GLuint font_texture_id, string text, float screen_size, float spacing, glm::vec3 position)
{
    // Scale the size of the fontbank in the UV-plane
    // We will use this for spacing and positioning
    float width = 1.0f / FONTBANK_SIZE;
    float height = 1.0f / FONTBANK_SIZE;

    // Instead of having a single pair of arrays, we'll have a series of pairs—one for each character
    // Don't forget to include <vector>!
    vector<float> vertices;
    vector<float> texture_coordinates;

    // For every character...
    for (int i = 0; i < text.size(); i++) {
        // 1. Get their index in the spritesheet, as well as their offset (i.e. their position
        //    relative to the whole sentence)
        int spritesheet_index = (int) text[i];  // ascii value of character
        float offset = (screen_size + spacing) * i;
        
        // 2. Using the spritesheet index, we can calculate our U- and V-coordinates
        float u_coordinate = (float) (spritesheet_index % FONTBANK_SIZE) / FONTBANK_SIZE;
        float v_coordinate = (float) (spritesheet_index / FONTBANK_SIZE) / FONTBANK_SIZE;

        // 3. Inset the current pair in both vectors
        vertices.insert(vertices.end(), {
            offset + (-0.5f * screen_size), 0.5f * screen_size,
            offset + (-0.5f * screen_size), -0.5f * screen_size,
            offset + (0.5f * screen_size), 0.5f * screen_size,
            offset + (0.5f * screen_size), -0.5f * screen_size,
            offset + (0.5f * screen_size), 0.5f * screen_size,
            offset + (-0.5f * screen_size), -0.5f * screen_size,
        });

        texture_coordinates.insert(texture_coordinates.end(), {
            u_coordinate, v_coordinate,
            u_coordinate, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate + width, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate, v_coordinate + height,
        });
    }

    // 4. And render all of them using the pairs
    glm::mat4 model_matrix = glm::mat4(1.0f);
    g_text_matrix = glm::translate(g_text_matrix, position);
    
    program->SetModelMatrix(g_text_matrix);
    glUseProgram(program->programID);
    
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices.data());
    glEnableVertexAttribArray(program->positionAttribute);
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coordinates.data());
    glEnableVertexAttribArray(program->texCoordAttribute);
    
    glBindTexture(GL_TEXTURE_2D, font_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, (int) (text.size() * 6));
    
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

void render()
{
    m_program.SetViewMatrix(m_view_matrix);
    
    glClear(GL_COLOR_BUFFER_BIT);
    
    g_state.player->render(&m_program);
    g_state.map->render(&m_program);
    for (int i = 0; i < ENEMY_COUNT; i++) {
        g_state.enemies[i].render(&m_program);
    }
    
    if (g_state.player->game_over == true) {
        if (mission == true) {
            DrawText(&m_program, text_texture_id, "MISSION SUCCESS!", 0.5f, 0.0f, glm::vec3(0.0f, 0.0f, 0.0f));
        }
        else {
            DrawText(&m_program, text_texture_id, "MISSION FAILED!", 0.5f, 0.0f, glm::vec3(0.0f, 0.0f, 0.0f));
        }
    }
    
    SDL_GL_SwapWindow(m_display_window);
}

void shutdown()
{
    SDL_Quit();
    
    delete [] g_state.enemies;
    delete    g_state.player;
    delete    g_state.map;
}

// ————— GAME LOOP ————— //
int main(int argc, char* argv[])
{
    initialise();
    
    while (m_game_is_running)
    {
        process_input();
        update();
        render();
    }
    
    shutdown();
    return 0;
}
