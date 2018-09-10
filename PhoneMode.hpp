//
// Created by 何宇 on 2018/9/7.
//

#pragma once

#include "Mode.hpp"
#include "Scene.hpp"
#include "WalkMesh.hpp"
#include "Sound.hpp"

struct PhoneMode : public Mode {
    PhoneMode();
    virtual ~PhoneMode();

    virtual bool handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) override;

    virtual void update(float elapsed) override;

    virtual void draw(glm::uvec2 const &drawable_size) override;

    void show_pause_menu();
    void show_answer_menu();

    struct {
        bool forward = false;
        bool backward = false;
        bool left = false;
        bool right = false;
    } controls;

    //------- game state -------

    Scene* scene;
    WalkMesh* walk_mesh;
    WalkMesh::WalkPoint walk_point;
    Scene::Camera *camera = nullptr;

    bool menu_mode = false;

    bool mouse_captured = true;
    bool interact_available = false;
    bool answered = false;
    int interact_phone = 0;

    std::string phone_message;
    float phone_message_width;

    enum STATUS {
        BEFORE_ANSWER,
        BEFORE_REDIRECT,
        AFTER_ANSWER,
        AFTER_REDIRECT
    };

    enum RINGMODE{
        CHECK,
        REDIRECT,
    };

    std::string PHONE_COLORS[5] = {
        "GREEN",
        "RED",
        "BLUE",
        "PURPLE"
    };

    std::string ANSWER_WORDS[5] = {
        "DOG",
        "CAT",
        "RABBIT",
        "KLINGON",
        "ZELDA"
    };

    STATUS current_status = BEFORE_ANSWER;
    int current_ringing_phone = 0;
    RINGMODE current_mode;
    int redirect_target = -1;
    int answer_words;

    std::string chose_word;

    int strike = 0;
    int pass = 0;

    bool game_over = false;
    bool game_clear = false;

    std::shared_ptr< Sound::PlayingSample > bgm;
    std::shared_ptr< Sound::PlayingSample > ringtone;
    std::shared_ptr< Sound::PlayingSample > current_phone_sound;

    std::vector<Scene::Object*> phone_list;
};
