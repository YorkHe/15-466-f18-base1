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

PhoneMode::PhoneMode() {
    this->scene = new Scene(
            data_path("phone.scene"),
            data_path("phone.pnc"),
            vertex_color_program.value);

    camera = scene->first_camera;

    if (camera == nullptr) {
        auto transform = scene->new_transform();
        transform->position = glm::vec3(0.0f, -10.0f, 1.0f);
        transform->rotation = glm::angleAxis(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        camera = scene->new_camera(transform);
    }

    std::cerr << "GOOD1" << std::endl;
}

PhoneMode::~PhoneMode() {
}

bool PhoneMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
    return true;
}

void PhoneMode::update(float elapsed) {

}

void PhoneMode::draw(glm::uvec2 const &drawable_size) {
//set up basic OpenGL state:
    std::cerr << "GOOD2" << std::endl;
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

    std::cerr << "GOOD3" << std::endl;
    //fix aspect ratio of camera
    camera->aspect = drawable_size.x / float(drawable_size.y);

    std::cerr << "GOOD4" << std::endl;
    scene->draw(camera);
    std::cerr << "GOOD5" << std::endl;

    GL_ERRORS();
}

void PhoneMode::show_pause_menu() {

}
