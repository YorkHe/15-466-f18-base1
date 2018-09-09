//
// Created by 何宇 on 2018/9/7.
//

#include <glm/gtc/type_ptr.hpp>
#include "PhoneMode.hpp"
#include "Load.hpp"
#include "MeshBuffer.hpp"
#include "data_path.hpp"
#include "vertex_color_program.hpp"
#include "gl_errors.hpp"

#include <iostream>

PhoneMode::PhoneMode() {
    this->scene = new Scene(
            data_path("phone.scene"),
            data_path("phone.pnc"),
            vertex_color_program.value);

    camera = scene->first_camera;

    if (camera == nullptr) {
        auto transform = scene->new_transform();
        transform->position = glm::vec3(0.0f, 0.0f, 2.5f);
        transform->rotation = glm::angleAxis(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        camera = scene->new_camera(transform);
    }

    this->walk_mesh = new WalkMesh(data_path("phone.wok"));

//    std::cerr << walk_mesh->vertices.size() << std::endl;
//    for (int i =0; i < walk_mesh->vertices.size(); i++) {
//        std::cerr << walk_mesh->vertices[i].x << "," << walk_mesh->vertices[i].y << "," << walk_mesh->vertices[i].z << std::endl;
//    }
//
//    std::cerr << walk_mesh->triangles.size() << std::endl;
//    for (int i =0; i < walk_mesh->vertices.size(); i++) {
//        std::cerr << walk_mesh->triangles[i].x << "," << walk_mesh->triangles[i].y << "," << walk_mesh->triangles[i].z << std::endl;
//    }

    walk_point = this->walk_mesh->start(glm::vec3(0.0f, 0.0f, 1.5f));

}

PhoneMode::~PhoneMode() {
}

bool PhoneMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

    if (evt.type == SDL_KEYDOWN && evt.key.repeat) {
        return false;
    }

    if (evt.type == SDL_KEYDOWN || evt.type == SDL_KEYUP) {
        if (evt.key.keysym.scancode == SDL_SCANCODE_W) {
            controls.forward = (evt.type == SDL_KEYDOWN);
            return true;
        } else if (evt.key.keysym.scancode == SDL_SCANCODE_S) {
            controls.backward = (evt.type == SDL_KEYDOWN);
            return true;
        } else if (evt.key.keysym.scancode == SDL_SCANCODE_A) {
            controls.left = (evt.type == SDL_KEYDOWN);
            return true;
        } else if (evt.key.keysym.scancode == SDL_SCANCODE_D) {
            controls.right = (evt.type == SDL_KEYDOWN);
            return true;
        }
    }

//    SDL_SetRelativeMouseMode(SDL_TRUE);

    if (evt.type == SDL_KEYDOWN) {
        if (evt.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
            mouse_captured = !mouse_captured;

            if (mouse_captured) {
                SDL_SetRelativeMouseMode(SDL_TRUE);
            } else {
                SDL_SetRelativeMouseMode(SDL_FALSE);
            }
        }
    }

    if (evt.type == SDL_MOUSEMOTION) {
        //Note: float(window_size.y) * camera->fovy is a pixels-to-radians conversion factor
        float yaw = evt.motion.xrel / float(window_size.y) * camera->fovy;
        float pitch = evt.motion.yrel / float(window_size.y) * camera->fovy;
        yaw = -yaw;
        pitch = -pitch;
        camera->transform->rotation = glm::normalize(
                camera->transform->rotation
                * glm::angleAxis(yaw, glm::vec3(0.0f, 1.0f, 0.0f))
                * glm::angleAxis(pitch, glm::vec3(1.0f, 0.0f, 0.0f))
        );
        return true;
    }

    return false;
}
void print2_vec3(const std::string name, glm::vec3 vec) {
	std::cerr << name << ":" << vec.x << "," << vec.y << "," << vec.z << std::endl;
}
void PhoneMode::update(float elapsed) {
    glm::mat3 directions = glm::mat3_cast(camera->transform->rotation);
    float amt = 5.0f * elapsed;

    glm::vec3 step = glm::vec3(0.0f);

    if (controls.right) step = amt * directions[0];
    if (controls.left) step = -amt * directions[0];
    if (controls.backward) step = amt * directions[2];
    if (controls.forward) step = -amt * directions[2];

//    std::cerr << directions[0].x << "," << directions[0].y << "," << directions[0].z << std::endl;

    if (step != glm::vec3(0.0f))
        walk_mesh->walk(walk_point, step);


//    camera->transform->position = walk_mesh->world_point(walk_point);
    glm::vec3 normal = walk_mesh->world_normal(walk_point);
    glm::vec3 position = walk_mesh->world_point(walk_point) + 0.5f * normal;
    camera->transform->position.x = position.x;
    camera->transform->position.y = position.y;
    camera->transform->position.z = position.z;

//
    glm::vec3 player_right = directions[0];
    glm::vec3 old_player_up = glm::normalize(directions[1]);
    glm::vec3 player_forward = -directions[2];

//    print2_vec3("RIGHT", player_right);
//    print2_vec3("UP", old_player_up);
//    print2_vec3("FORWARD", player_forward);


//
    glm::vec3 w = glm::cross(old_player_up, normal);
//
    glm::quat orientation_change= glm::normalize(
            glm::quat(
                   glm::dot(old_player_up, normal), w.x, w.y, w.z
            )
    );

    player_forward = orientation_change * player_forward ;

    player_forward = glm::normalize(player_forward - normal * glm::dot(normal, player_forward));
    player_right = glm::cross(player_forward, normal);

    auto q = glm::quat_cast(glm::mat3(
            player_right, normal, -player_forward
    ));

    camera->transform->rotation = q;

//    std::cerr << "Q :" << q.x << "," << q.y << "," << q.z << "," << q.w << std::endl;
//
//    auto q2 = camera->transform->rotation;
////
//    std::cerr << "Q2:" << q2.x << "," << q2.y << "," << q2.z << "," << q2.w << std::endl;


//    std::cerr << "WORLD" << camera->transform->position.x << "," << camera->transform->position.y << "," << camera->transform->position.z << std::endl;
//    glm::vec3 position = walk_mesh->world_point(walk_point);
//    auto p = this->walk_mesh->start(camera->transform->position);
//
//    const glm::vec3 &vertex_a = walk_mesh->vertices[p.triangle[0]];
//    const glm::vec3 &vertex_b = walk_mesh->vertices[p.triangle[1]];
//    const glm::vec3 &vertex_c = walk_mesh->vertices[p.triangle[2]];
//

//    std::cerr << "WALK" << position.x << "," << position.y << "," << position.z << std::endl;
}

void PhoneMode::draw(glm::uvec2 const &drawable_size) {
    //set up basic OpenGL state:
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //set up light position + color:
    glUseProgram(vertex_color_program->program);
    glUniform3fv(vertex_color_program->sun_color_vec3, 1, glm::value_ptr(glm::vec3(0.81f, 0.81f, 0.76f)));
    glUniform3fv(vertex_color_program->sun_direction_vec3, 1, glm::value_ptr(glm::normalize(glm::vec3(-0.2f, 0.2f, 1.0f))));
    glUniform3fv(vertex_color_program->sky_color_vec3, 1, glm::value_ptr(glm::vec3(0.4f, 0.4f, 0.45f)));
    glUniform3fv(vertex_color_program->sky_direction_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 1.0f, 0.0f)));
    glUseProgram(0);

    //fix aspect ratio of camera
    camera->aspect = drawable_size.x / float(drawable_size.y);

    scene->draw(camera);

    GL_ERRORS();
}

void PhoneMode::show_pause_menu() {

}
