#include "FunEngine2D/core/include/globals.h"
#include "FunEngine2D/core/include/render/WindowManager.h"
#include "FunEngine2D/core/include/Resources.h"
#include "FunEngine2D/core/include/_Time.h"
#include "FunEngine2D/core/include/Input.h"
#include "FunEngine2D/core/include/tools/Debugger.h"
#include "FunEngine2D/experimental/include/networking/Client.h"
#include "FunEngine2D/experimental/include/networking/Server.h"
#include "FunEngine2D/experimental/include/ecs/ECS.h"
#include "FunEngine2D/experimental/include/physics/Physics.h"

struct Controller {
    fun::ecs::Entity entity = fun::ecs::NULLENTITY;

    float speed;
    float torque;

    explicit Controller(float s, float t) {
        speed = s;
        torque = t;
    }

    void Update() {
        if (entity == fun::ecs::NULLENTITY) {
            entity = fun::ecs::get_entity <Controller> (this);
        }

        const float delta_time = fun::time::delta_time();

        auto& shape = fun::ecs::get_component <sf::Sprite> (entity);

        float vertical = fun::input::vertical(sf::Keyboard::Down, sf::Keyboard::Up);
        float horizontal = fun::input::horizontal(sf::Keyboard::Left, sf::Keyboard::Right);

        shape.rotate(horizontal * torque * delta_time);

        float rotation = fun::math::radians(shape.getRotation());

        shape.move(vertical * sf::Vector2(sin(rotation), -cos(rotation)) * speed * delta_time);
    }
};

void load_resources() {
    fun::resources::load_texture("spaceship", "spaceship.png");
    fun::resources::load_shader("pixelate", "pixelate");
}

void make_spaceship() {
    auto spaceship = fun::ecs::new_entity();

    fun::ecs::add_component <Controller> (spaceship, 200, 200);
    fun::ecs::add_component <sf::Sprite> (spaceship, fun::resources::get_texture("spaceship"));

    auto& sprite = fun::ecs::get_component <sf::Sprite> (spaceship);
    sf::Vector2u texture_size = sprite.getTexture()->getSize();

    sprite.setOrigin(texture_size.x * .5f, texture_size.y * .5f);
}

int main () {
    fun::wndmgr::init(fun::wndmgr::WindowData("Spaceships"));
    auto* window = fun::wndmgr::main_window;

    load_resources();
    make_spaceship();
    
    while (window->render.isOpen()) {
        fun::time::recalculate();
        fun::wndmgr::update();
        fun::input::listen();
        fun::physics::simulate();

        window->world_view.move(fun::input::keyboard_2d() * sf::Vector2f(1, -1) * 300.f * fun::time::delta_time());

        for (auto& controller : fun::ecs::get_component_array <Controller> ()) {
            controller.Update();
        }

        for (auto& sprite : fun::ecs::get_component_array <sf::Sprite> ()) {
            window->DrawWorld(sprite, 0);
        }

        window->Display(sf::Color::White, fun::resources::get_shader("pixelate"));
    }

    fun::wndmgr::close();

    return 0;
};