//
// Created by 何宇 on 2018/9/7.
//
#include "GL.hpp"

#include <glm/gtc/type_ptr.hpp>
#include "PhoneMode.hpp"
#include "Load.hpp"
#include "MeshBuffer.hpp"
#include "data_path.hpp"
#include "vertex_color_program.hpp"
#include "gl_errors.hpp"
#include "MenuMode.hpp"

#include "draw_text.hpp"

#include <iostream>



Load <Sound::Sample> sample_ringtone(LoadTagDefault, [](){
    return new Sound::Sample(data_path("ringtone.wav"));
});

Load <Sound::Sample> sample_bgm(LoadTagDefault, [](){
    return new Sound::Sample(data_path("space.wav"));
});

Load <Sound::Sample> sample_beep(LoadTagDefault, [](){
    return new Sound::Sample(data_path("beep.wav"));
});

Load <Sound::Sample> sample_alien(LoadTagDefault, [](){
    return new Sound::Sample(data_path("alien.wav"));
});


PhoneMode::PhoneMode() {
    this->scene = new Scene(
            data_path("phone.scene"),
            data_path("phone.pnc"),
            vertex_color_program.value);

    for (auto i = scene->first_object; i != nullptr; i = i->alloc_next) {
        if (i->name == "Phone") {
            phone_list.emplace_back(i);
        }
    }

    assert(phone_list.size() == 4 && "There should be 4 phones in the scene");

    phone_list[0]->set_uniforms = [this](){
        glUniform4fv(phone_list[0]->program_override_color_vec4, 1, glm::value_ptr(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)));
    };

    phone_list[1]->set_uniforms = [this](){
        glUniform4fv(phone_list[1]->program_override_color_vec4, 1, glm::value_ptr(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)));
    };

    phone_list[2]->set_uniforms = [this](){
        glUniform4fv(phone_list[2]->program_override_color_vec4, 1, glm::value_ptr(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)));
    };

    phone_list[3]->set_uniforms = [this](){
        glUniform4fv(phone_list[3]->program_override_color_vec4, 1, glm::value_ptr(glm::vec4(1.0f, 0.0f, 1.0f, 1.0f)));
    };

    camera = scene->first_camera;

    if (camera == nullptr) {
        auto transform = scene->new_transform();
        transform->position = glm::vec3(0.0f, 0.0f, 2.5f);
        transform->rotation = glm::angleAxis(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        camera = scene->new_camera(transform);
    }

    this->walk_mesh = new WalkMesh(data_path("phone.wok"));

    walk_point = this->walk_mesh->start(glm::vec3(0.0f, 0.0f, 1.5f));

    bgm = sample_bgm->play(camera->transform->position, 1.0f, Sound::Loop);
    srand((unsigned)time(nullptr));
    current_ringing_phone = rand() % 4;

    ringtone = sample_ringtone->play(phone_list[current_ringing_phone]->transform->position, 1.0f, Sound::Loop);
}

PhoneMode::~PhoneMode() {
    if (bgm) bgm->stop();
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

    if (evt.type == SDL_KEYDOWN) {
        if (evt.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
            mouse_captured = !mouse_captured;

            if (mouse_captured) {
                SDL_SetRelativeMouseMode(SDL_TRUE);
            } else {
                SDL_SetRelativeMouseMode(SDL_FALSE);
            }
        }

        if (evt.key.keysym.scancode == SDL_SCANCODE_F) {
            if (interact_available) {

                if (interact_phone == current_ringing_phone) {
                    if (ringtone) ringtone->stop();
                    if (current_phone_sound) current_phone_sound->stop();
                    current_phone_sound = sample_alien->play(phone_list[interact_phone]->transform->position, 1.0f, Sound::Once);

                    float height = 0.06f;
                    int r;
                    if (current_status == BEFORE_ANSWER) {
                        r = rand() % 2;
                        if (r == 0) current_mode = CHECK; else current_mode = REDIRECT;
                        switch(current_mode) {
                            case CHECK:
                                phone_message = "NOTHING JUST CHECKING";
                                phone_message_width = text_width(phone_message, height);
                                pass++;
                                current_status = AFTER_ANSWER;
                                break;
                            case REDIRECT:
                                int old_target = redirect_target;
                                while (redirect_target == -1 || redirect_target == current_ringing_phone || old_target == redirect_target) redirect_target = rand() % 4;
                                answer_words = rand() % 5;

                                phone_message= "GO TO " + PHONE_COLORS[redirect_target] + " PHONE AND SAY " + ANSWER_WORDS[answer_words] + "";

                                phone_message_width = text_width(phone_message, height);
                                current_status = BEFORE_REDIRECT;
                                break;
                        }
                    }
                } else {
                    if (interact_phone == redirect_target && current_status == BEFORE_REDIRECT) {
                        current_status = AFTER_REDIRECT;
                        show_answer_menu();
                    } else {
                        strike++;
                        if (current_phone_sound) current_phone_sound->stop();
                        current_phone_sound = sample_beep->play(phone_list[interact_phone]->transform->position, 1.0f, Sound::Loop);
                    }
                }
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
void PhoneMode::update(float elapsed) {

    if (game_clear || game_over) return;

    if (current_status == AFTER_ANSWER) {

        current_status = BEFORE_ANSWER;

        int old_ringing_phone = current_ringing_phone;
        while(current_ringing_phone == old_ringing_phone) current_ringing_phone = rand() % 4;

        ringtone = sample_ringtone->play(phone_list[current_ringing_phone]->transform->position, 1.0f, Sound::Loop);
    }

    if (current_status == AFTER_REDIRECT && answered) {
        std::cerr << chose_word << ":::" << ANSWER_WORDS[answer_words] << std::endl;
        if (chose_word == ANSWER_WORDS[answer_words]) {
            std::cerr << "PASS!" << std::endl;
            pass++;
        } else {
            std::cerr << "STRIKE!" << std::endl;
            strike++;
        }

        current_status = BEFORE_ANSWER;

        int old_ringing_phone = current_ringing_phone;
        while(current_ringing_phone == old_ringing_phone) current_ringing_phone = rand() % 4;

        ringtone = sample_ringtone->play(phone_list[current_ringing_phone]->transform->position, 1.0f, Sound::Loop);
    }

    if (pass >= 5) {
        game_clear = true;
    }

    if (strike > 3) {
        game_over = true;
    }

    glm::mat3 directions = glm::mat3_cast(camera->transform->rotation);
    float amt = 5.0f * elapsed;

    glm::vec3 step = glm::vec3(0.0f);

    if (controls.right) step = amt * directions[0];
    if (controls.left) step = -amt * directions[0];
    if (controls.backward) step = amt * directions[2];
    if (controls.forward) step = -amt * directions[2];

    if (step != glm::vec3(0.0f))
        walk_mesh->walk(walk_point, step);


    glm::vec3 normal = walk_mesh->world_normal(walk_point);
    glm::vec3 position = walk_mesh->world_point(walk_point) + 0.5f * normal;
    camera->transform->position.x = position.x;
    camera->transform->position.y = position.y;
    camera->transform->position.z = position.z;

    glm::vec3 player_right = directions[0];
    glm::vec3 old_player_up = glm::normalize(directions[1]);
    glm::vec3 player_forward = -directions[2];

    glm::vec3 w = glm::cross(old_player_up, normal);

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

    {
        glm::mat4 cam_to_world = camera->transform->make_local_to_world();
        Sound::listener.set_position(cam_to_world[3]);

        Sound::listener.set_right(glm::normalize(cam_to_world[0]));

        if (bgm) {
            bgm->set_position(cam_to_world[3]);
        }
    }


    interact_available = false;
    interact_phone = 0;
    for (int i = 0; i < phone_list.size(); i++) {
        auto phone = phone_list[i];
        if (glm::distance(phone->transform->position, camera->transform->position) < 1.5f) {
            interact_available = true;
            interact_phone = i;
        }
    }

    if (!interact_available) {
        if(current_phone_sound) {
            current_phone_sound->stop();
            current_phone_sound = nullptr;
        }
    }

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

    if (Mode::current.get() == this) {

        for (int i = 0; i < strike; i++) {
            std::string c = "X";
            float height = 0.04f;
            float width = text_width(c, height);
            draw_text(c, glm::vec2(0.95 - (width + 0.02) * i, 0.89f), height, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
        }

        for (int i = 0; i < pass; i++) {
            std::string c = "O";
            float height = 0.04f;
            float width = text_width(c, height);
            draw_text(c, glm::vec2(0.95 - (width + 0.02) * i, 0.80f), height, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        }

        if (game_over) {
            std::string message = "GAME OVER";
            float height = 0.18f;
            float width = text_width(message, height);
            draw_text(message, glm::vec2(-0.5f * width, 0.0f), height, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
            draw_text(message, glm::vec2(-0.5f * width, -0.01f), height, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

            message = "NOW YOU CAN FINALLY CLOSE THIS BORING GAME";
            height = 0.04f;
            width = text_width(message, height);
            draw_text(message, glm::vec2(-0.5f * width, -0.6f), height, glm::vec4(0.0f, 0.0f, 0.0f, 0.6f));
            draw_text(message, glm::vec2(-0.5f * width, -0.61f), height, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        }

        else if (game_clear) {
            std::string message = "GAME CLEAR";
            float height = 0.18f;
            float width = text_width(message, height);
            draw_text(message, glm::vec2(-0.5f * width, 0.0f), height, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
            draw_text(message, glm::vec2(-0.5f * width, -0.01f), height, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

            message = "NOW YOU CAN FINALLY CLOSE THIS BORING GAME";
            height = 0.04f;
            width = text_width(message, height);
            draw_text(message, glm::vec2(-0.5f * width, -0.6f), height, glm::vec4(0.0f, 0.0f, 0.0f, 0.6f));
            draw_text(message, glm::vec2(-0.5f * width, -0.61f), height, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        }

        else if (interact_available) {
            float height = 0.06f;
            float width = text_width("PRESS F TO INTERACT", height);
            draw_text("PRESS F TO INTERACT", glm::vec2(-0.5f * width, -0.59f), height, glm::vec4(0.0f, 0.0f, 0.0f, 0.5f));
            draw_text("PRESS F TO INTERACT", glm::vec2(-0.5f * width, -0.6f), height, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

            if (phone_message_width != 0) {
                draw_text(phone_message, glm::vec2(-0.5f * phone_message_width, -0.5f), height, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
            }
        } else {
            phone_message_width = 0;
        }
    }

    GL_ERRORS();
}

void PhoneMode::show_answer_menu() {
    std::shared_ptr< MenuMode > menu = std::make_shared<MenuMode>();

    std::shared_ptr<Mode> game = shared_from_this();
    menu->background  = game;
    answered = false;
    for (auto word : ANSWER_WORDS) {
        menu->choices.emplace_back(word, [word, game, this](){
            chose_word = word;
            Mode::set_current(game);
            answered = true;
        });
    }

    menu->selected = 1;
    Mode::set_current(menu);
}

void PhoneMode::show_pause_menu() {

}
