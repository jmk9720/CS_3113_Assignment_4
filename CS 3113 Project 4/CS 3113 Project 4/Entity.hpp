/**
* Author: Jonathan Kim
* Assignment: Rise of the AI
* Date due: 2023-11-18, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "Map.hpp"

enum EntityType { PLATFORM, PLAYER, ENEMY };
enum AIType { GUARD, ASSASSIN, JUMPER };
enum AIState { WALKING, IDLE, ATTACKING, RESET };

class Entity
{
private:
    bool m_is_active = true;
    EntityType m_entity_type;
    AIType m_ai_type;
    AIState m_ai_state;
    
    int *m_animation_right = NULL;
    int *m_animation_left = NULL;
    int *m_animation_up = NULL;
    int *m_animation_down = NULL;
    
    glm::vec3 m_position;
    glm::vec3 m_velocity;
    glm::vec3 m_acceleration;
    
    float m_width = 0.8f;
    float m_height = 0.8f;
    
    bool dead = false;
    int jump_counter = 0;
public:
    static const int SECONDS_PER_FRAME = 4;
    static const int LEFT  = 0,
                     RIGHT = 1,
                     UP    = 2,
                     DOWN  = 3;
    
    GLuint m_texture_id;
    glm::mat4 m_model_matrix;
    
    float m_speed;
    glm::vec3 m_movement;
    
    int **m_walking = new int*[4] { m_animation_left, m_animation_right, m_animation_up, m_animation_down };
    int *m_animation_indices = NULL;
    int m_animation_frames = 0;
    int m_animation_index = 0;
    float m_animation_time = 0.0f;
    int m_animation_cols = 0;
    int m_animation_rows = 0;
    
    bool m_is_jumping = false;
    float m_jumping_power = 0;
    
    bool m_map_top = false;
    bool m_map_bottom = false;
    bool m_map_left = false;
    bool m_map_right = false;
    
    bool m_enemy_top = false;
    bool m_enemy_bottom = false;
    bool m_enemy_left = false;
    bool m_enemy_right = false;

    bool game_over = false;
    Entity();
    ~Entity();

    void draw_sprite_from_texture_atlas(ShaderProgram *program, GLuint texture_id, int index);
    void update(float delta_time, Entity *player, Entity *objects, int object_count, Map *map);
    void render(ShaderProgram *program);
    void activate_ai(Entity *player);
    void ai_guard(Entity *player);
    void ai_jump(Entity *player);
    void ai_assassin(Entity *player);
    
    void const check_collision_y(Entity *collidable_entities, int collidable_entity_count);
    void const check_collision_x(Entity *collidable_entities, int collidable_entity_count);
    void const check_collision_y(Map *map);
    void const check_collision_x(Map *map);
    
    bool const check_collision(Entity *other) const;
    
    void activate() { m_is_active = true; };
    void deactivate() { m_is_active = false; };
    
    EntityType const get_entity_type() const { return m_entity_type; };
    AIType     const get_ai_type() const { return m_ai_type; };
    AIState    const get_ai_state() const { return m_ai_state; };
    glm::vec3  const get_position() const { return m_position; };
    glm::vec3  const get_movement() const { return m_movement; };
    glm::vec3  const get_velocity() const { return m_velocity; };
    glm::vec3  const get_acceleration() const { return m_acceleration; };
    float      const get_jumping_power () const { return m_jumping_power; };
    float      const get_speed() const { return m_speed; };
    int        const get_width() const { return m_width; };
    int        const get_height() const { return m_height; };
    bool const get_dead() const { return dead; }
    
    void const set_entity_type(EntityType new_entity_type) { m_entity_type = new_entity_type; };
    void const set_ai_type(AIType new_ai_type) { m_ai_type = new_ai_type; };
    void const set_ai_state(AIState new_state) { m_ai_state = new_state; };
    void const set_position(glm::vec3 new_position) { m_position = new_position; };
    void const set_movement(glm::vec3 new_movement) { m_movement = new_movement; };
    void const set_velocity(glm::vec3 new_velocity) { m_velocity = new_velocity; };
    void const set_speed(float new_speed) { m_speed = new_speed; };
    void const set_jumping_power(float new_jumping_power) { m_jumping_power = new_jumping_power; };
    void const set_acceleration(glm::vec3 new_acceleration) { m_acceleration = new_acceleration; };
    void const set_width(float new_width) { m_width = new_width; };
    void const set_height(float new_height) { m_height = new_height; };
};