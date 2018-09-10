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

    bool mouse_captured = true;
    bool interact_available = false;

    std::shared_ptr< Sound::PlayingSample > bgm;

    std::vector<Scene::Object*> phone_list;
};
