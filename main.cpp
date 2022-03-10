#include "FunEngine2D/core/include/Globals.h"
#include "FunEngine2D/core/include/render/WindowManager.h"
#include "FunEngine2D/core/include/Resources.h"
#include "FunEngine2D/core/include/_Time.h"
#include "FunEngine2D/core/include/Input.h"
#include "FunEngine2D/core/include/tools/Debugger.h"
#include "FunEngine2D/core/include/render/shapes/Particler.h"

#include "FunEngine2D/core/include/networking/Client.h"
#include "FunEngine2D/core/include/networking/Server.h"
#include "FunEngine2D/core/include/ecs/ECS.h"
#include "FunEngine2D/core/include/physics/Physics.h"
#include "FunEngine2D/core/include/interactable/Interaction.h"
#include "FunEngine2D/core/include/interactable/Interactable.h"








struct Body {
    fun::ecs::Entity entity = fun::ecs::NULLENTITY;

    b2Body* b2body = nullptr;

    Body(const b2BodyDef& body_def, const b2FixtureDef& fixture) {
        b2body = fun::physics::create_body(body_def);

        b2body->CreateFixture(&fixture);
    }

    static void OnCreate(Body& body) {
        body.entity = fun::ecs::get_entity <Body> (body);
    }
    
    static void OnDestroy(Body& body) {
        fun::physics::destroy_body(body.b2body);
    }

    sf::Vector2f GetForwardVector() {
        float radians = b2body->GetAngle();

        return sf::Vector2f(sin(radians), cos(radians));
    }

    void SetLinearVelocity(sf::Vector2f velocity) {
        b2body->SetLinearVelocity({ velocity.x, velocity.y });
    }

    void SetAngularVelocity(float delta) {
        b2body->SetAngularVelocity(delta);
    }

    void BeforeResolve() {
        auto& transform = fun::ecs::get_component <fun::Transform> (entity);

        float simulation_scale = fun::physics::get_simulation_scale();
        b2Vec2 b2position = { transform.position.x * simulation_scale, transform.position.y * simulation_scale };

        // ! check if positions or angles are distant enough

        b2body->SetTransform(b2position, transform.rotation);
    }

    void AfterResolve() {
        auto& transform = fun::ecs::get_component <fun::Transform> (entity);

        float simulation_scale = fun::physics::get_simulation_scale();
        b2Vec2 b2position = b2body->GetPosition();
        sf::Vector2f position = { b2position.x / simulation_scale, b2position.y / simulation_scale };

        transform.position = position;
        transform.rotation = b2body->GetAngle();
    }
};

struct Controller {
    fun::ecs::Entity entity = fun::ecs::NULLENTITY;

    float speed;
    float torque;

    explicit Controller(float s, float t) {
        speed = s;
        torque = t;
    }

    static void OnCreate(Controller& controller) {
        controller.entity = fun::ecs::get_entity <Controller> (controller);
    }

    void Update() {
        if (!fun::ecs::has_component <Body> (entity)) return;

        auto& body = fun::ecs::get_component <Body> (entity);

        float vertical = fun::input::vertical(sf::Keyboard::Down, sf::Keyboard::Up);
        float horizontal = fun::input::horizontal(sf::Keyboard::Left, sf::Keyboard::Right);

        body.SetAngularVelocity(horizontal * torque);
        body.SetLinearVelocity(vertical * body.GetForwardVector() * speed);
    }
};







void load_resources() {
    fun::resources::load_texture("spaceship", "spaceship.png");
    fun::resources::load_shader("pixelate", "pixelate");
    fun::resources::load_font("default", "lato_light.ttf");
}

void create_callbacks() {
    fun::ecs::oncreate_callback <Controller> (Controller::OnCreate);
    fun::ecs::oncreate_callback <Body> (Body::OnCreate);

    fun::ecs::ondestroy_callback <Body> (Body::OnDestroy);
}

void make_spaceship(sf::Vector2f position) {
    auto spaceship = fun::ecs::new_entity();

    auto& texture = fun::resources::get_texture("spaceship");

    float radius = texture.getSize().x * .5f;

    b2BodyDef body;
    body.position.Set(position.x * fun::physics::get_simulation_scale(), position.y * fun::physics::get_simulation_scale());
    body.type = b2_kinematicBody;
    body.linearDamping = 0;
    body.angularDamping = 0;

    b2CircleShape shape;
    shape.m_radius = radius * fun::physics::get_simulation_scale();

    b2FixtureDef fixture;
    fixture.shape = &shape;

    fun::ecs::add_component <Body> (spaceship, body, fixture);
    fun::ecs::add_component <fun::Transform> (spaceship, position, 0);
    fun::ecs::add_component <Controller> (spaceship, 30.f, 3.f);
    fun::ecs::add_component <sf::Sprite> (spaceship, texture);
    fun::ecs::add_component <fun::Particler> (spaceship, fun::Particler::RenderType::Quads, 1000000, 1);

    auto& sprite = fun::ecs::get_component <sf::Sprite> (spaceship);
    sprite.setOrigin(radius, radius);
}

void make_barrier(sf::Vector2f position) {
    auto barrier = fun::ecs::new_entity();

    sf::Vector2f half_scale = { 10.f, 50.f };

    b2BodyDef body;
    body.position.Set(position.x * fun::physics::get_simulation_scale(), position.y * fun::physics::get_simulation_scale());
    body.type = b2_dynamicBody;
    body.linearDamping = INT_MAX;
    body.angularDamping = INT_MAX;

    b2PolygonShape shape;
    shape.SetAsBox(half_scale.x * fun::physics::get_simulation_scale(), half_scale.y * fun::physics::get_simulation_scale());

    b2FixtureDef fixture;
    fixture.shape = &shape;
    fixture.density = 1.0f;
    fixture.friction = 0.3f;

    fun::ecs::add_component <Body> (barrier, body, fixture);
    fun::ecs::add_component <fun::Transform> (barrier, position, 0);
    fun::ecs::add_component <sf::RectangleShape> (barrier, half_scale * 2.f);

    auto& rect = fun::ecs::get_component <sf::RectangleShape> (barrier);
    const sf::Vector2f scale = rect.getScale();

    rect.setOrigin(half_scale);
    rect.setFillColor(sf::Color::Black);
}

struct DebugData {
    std::string name;
};







int main () {
    fun::wndmgr::init(fun::wndmgr::WindowData("Spaceships"));
    auto* window = fun::wndmgr::main_window;

    fun::physics::set_simulation_scale(.1f);
    // fun::physics::set_gravity({ 0, -10.f });

    load_resources();
    create_callbacks();
    make_spaceship({ 0, 0 });
    make_barrier({ -130.f, 110.f });
    make_barrier({ 100.f, 80.f });
    make_barrier({ -50.f, -70.f });

    sf::Shader* shader = fun::resources::get_shader("pixelate");
    
    while (window->render.isOpen()) {
        fun::time::recalculate();
        fun::input::listen();
        fun::interaction::update();
        fun::wndmgr::update();

        for (auto& controller : fun::ecs::iterate_component <Controller> ()) {
            controller.Update();
        }

        {
            for (auto& body : fun::ecs::iterate_component <Body> ()) {
                body.BeforeResolve();
            }
            
            fun::physics::simulate();
            
            for (auto& body : fun::ecs::iterate_component <Body> ()) {
                body.AfterResolve();
            }
        }

        window->world_view.move(fun::input::keyboard_2d() * sf::Vector2f(1, -1) * window->zoom * 200.f * fun::time::delta_time());

        for (auto& sprite : fun::ecs::iterate_component <sf::Sprite> ()) {
            auto& transform = fun::ecs::get_component <fun::Transform> (fun::ecs::get_entity <sf::Sprite> (sprite));

            sprite.setPosition(transform.position * sf::Vector2f(1, -1));
            sprite.setRotation(fun::math::degrees(transform.rotation));
            
            window->DrawWorld(sprite, 10);
        }

        for (auto& rect : fun::ecs::iterate_component <sf::RectangleShape> ()) {
            auto& transform = fun::ecs::get_component <fun::Transform> (fun::ecs::get_entity <sf::RectangleShape> (rect));

            rect.setPosition(transform.position * sf::Vector2f(1, -1));
            rect.setRotation(-fun::math::degrees(transform.rotation)); // ! ? -
            
            window->DrawWorld(rect, 10);
        }

        for (auto& particler : fun::ecs::iterate_component <fun::Particler> ()) {
            auto& transform = fun::ecs::get_component <fun::Transform> (fun::ecs::get_entity (particler));
            auto& body = fun::ecs::get_component <Body> (fun::ecs::get_entity (particler));

            particler.transform = transform;

            if (abs(body.b2body->GetLinearVelocity().x) + abs(body.b2body->GetLinearVelocity().y) > 0) {
                fun::Particler::EmitData emit_data;

                emit_data.min_lifetime = .5f;
                emit_data.min_lifetime = 1.f;
                emit_data.min_direction_angle = fun::math::radians(160);
                emit_data.max_direction_angle = fun::math::radians(200);
                emit_data.min_velocity_start = 1.f;
                emit_data.max_velocity_start = 1000.f;
                emit_data.min_velocity_end = 0.f;
                emit_data.max_velocity_end = .1f;
                emit_data.max_size_start = 5.f;

                particler.Emit(20, emit_data);
            }

            particler.Update();
            
            window->DrawWorld(particler, 10);
        }

        fun::debugger::display_debug_menu();
        fun::debugger::display_unit_coord(window->GetMouseWorldPosition(), 16, 999, sf::Color::Black, "default");
        fun::debugger::display_debug_log();

        {
            const sf::Vector2u win_size = window->render.getSize();

            shader->setUniform("width", (float)win_size.x);
            shader->setUniform("height", (float)win_size.y);
            shader->setUniform("pixelate", .4f * window->zoom);
        }

        window->Display(sf::Color::White, shader);
    }

    fun::wndmgr::close();

    return 0;
};