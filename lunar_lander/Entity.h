#ifndef ENTITY_H
#define ENTITY_H

#include "glm/glm.hpp"
#include "ShaderProgram.h"
enum EntityType { PLAYER, GOAL_PLATFORM, TRAP_PLATFORM, MESSAGE};


enum AnimationDirection { LEFT, RIGHT, UP, DOWN };

class Entity
{
private:
    // Removed individual animation arrays
    bool m_is_active = true;
    bool m_left_accel = false;
    bool m_right_accel = false;
    bool m_up_accel = false;

    EntityType m_entity_type;


    // ————— TRANSFORMATIONS ————— //
    glm::vec3 m_movement;
    glm::vec3 m_position;
    glm::vec3 m_scale = glm::vec3(1.0f, 1.0f, 0.0f);

    // ––––– PHYSICS (GRAVITY) ––––– //
    glm::vec3 m_velocity;
    glm::vec3 m_acceleration;

    glm::mat4 m_model_matrix;

    float     m_speed;
    float m_width = 1;
    float m_height = 1;

    // ————— ANIMATION ————— //
    int m_animation_cols;
    int m_animation_frames,
        m_animation_index,
        m_animation_rows;

    int* m_animation_indices = nullptr;
    float m_animation_time = 0.0f;

    // ––––– PHYSICS (COLLISIONS) ––––– //
    bool m_collided_top = false;
    bool m_collided_bottom = false;
    bool m_collided_left = false;
    bool m_collided_right = false;
    

public:
    // ————— STATIC VARIABLES ————— //
        // ————— TEXTURES ————— //
    GLuint    m_texture_id;
    int m_walking[4][3]; // 4x4 array for walking animations

    bool collided = false;
    bool win = false;
    bool lose = false;
    static constexpr int SECONDS_PER_FRAME = 4;


    // ————— METHODS ————— //
    Entity();
    Entity(GLuint texture_id, float speed, int walking[4][3], float animation_time,
        int animation_frames, int animation_index, int animation_cols,
        int animation_rows);
    Entity(GLuint texture_id, float speed); // Simpler constructor
    ~Entity();

    void draw_sprite_from_texture_atlas(ShaderProgram* program, GLuint texture_id,
        int index);
    bool const check_collision(Entity* other) const;
    void const check_collision_x(Entity* collidable_entities, int collidable_entity_count);
    void const check_collision_y(Entity* collidable_entities, int collidable_entity_count);

    void activate() { m_is_active = true; };
    void deactivate() { m_is_active = false; };

    void update(float delta_time, Entity* collidable_entities, int collidable_entity_count);
    void render(ShaderProgram* program);

    void normalise_movement() { m_movement = glm::normalize(m_movement); };

    void face_left() { m_animation_indices = m_walking[LEFT]; }
    void face_right() { m_animation_indices = m_walking[RIGHT]; }
    void face_up() { m_animation_indices = m_walking[UP]; }
    void face_down() { m_animation_indices = m_walking[DOWN]; }

    void move_left() { m_movement.x = -1.0f; face_left(); };
    void move_right() { m_movement.x = 1.0f;  face_right(); };
    void move_up() { m_movement.y = 1.0f;  face_up(); };
    void move_down() { m_movement.y = -1.0f; face_down(); };

    // ————— GETTERS ————— //
    glm::vec3 const get_position()   const { return m_position; }
    glm::vec3 const get_movement()   const { return m_movement; }
    glm::vec3 const get_scale()      const { return m_scale; }
    GLuint    const get_texture_id() const { return m_texture_id; }
    float     const get_speed()      const { return m_speed; }
    EntityType const get_entity_type()    const { return m_entity_type; };
    bool is_active() const { return m_is_active; };

    // ————— SETTERS ————— //
    void const set_entity_type(EntityType new_entity_type) { m_entity_type = new_entity_type; };
    void const set_position(glm::vec3 new_position) { m_position = new_position; }
    void const set_velocity(glm::vec3 new_velocity) { m_velocity = new_velocity; }
    void const set_movement(glm::vec3 new_movement) { m_movement = new_movement; }
    void const set_acceleration(glm::vec3 new_position) { m_acceleration = new_position; };
    void const set_scale(glm::vec3 new_scale) { m_scale = new_scale; }
    void const set_texture_id(GLuint new_texture_id) { m_texture_id = new_texture_id; }

    void const set_speed(float new_speed) { m_speed = new_speed; }
    void const set_animation_cols(int new_cols) { m_animation_cols = new_cols; }
    void const set_animation_rows(int new_rows) { m_animation_rows = new_rows; }
    void const set_animation_frames(int new_frames) { m_animation_frames = new_frames; }
    void const set_animation_index(int new_index) { m_animation_index = new_index; }
    void const set_animation_time(int new_time) { m_animation_time = new_time; }

    // Setter for m_walking
    void set_walking(int walking[4][3])
    {
        for (int i = 0; i < 4; ++i)
        {
            for (int j = 0; j < 3; ++j)
            {
                m_walking[i][j] = walking[i][j];
            }
        }
    }
};

#endif // ENTITY_H