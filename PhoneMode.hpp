//
// Created by 何宇 on 2018/9/7.
//

#pragma once

#include "Mode.hpp"
#include "Scene.hpp"

struct PhoneMode : public Mode {
    PhoneMode();
    virtual ~PhoneMode();

    virtual bool handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) override;

    virtual void update(float elapsed) override;

    virtual void draw(glm::uvec2 const &drawable_size) override;

    void show_pause_menu();

    struct {
        bool forward = false;
        bool backward = false;
        bool left = false;
        bool right = false;
    } controls;

    //------- game state -------

    Scene* scene;
    Scene::Camera *camera = nullptr;
};
