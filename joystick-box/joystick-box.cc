
// General Engine headers
#include <ugdk/system/engine.h>
#include <ugdk/system/configuration.h>
#include <ugdk/system/compatibility.h>
#include <ugdk/action/scene.h>

// Input headers
#include <ugdk/input/events.h>
#include <ugdk/input/joystick.h>
#include <ugdk/action/events.h>
#include <ugdk/input/module.h>

// Graphic headers
#include <ugdk/desktop/window.h>
#include <ugdk/desktop/module.h>
#include <ugdk/graphic/canvas.h>
#include <ugdk/graphic/drawmode.h>
#include <ugdk/graphic/module.h>
#include <ugdk/graphic/vertexdata.h>
#include <ugdk/graphic/mesh.h>

// Type headers
#include <ugdk/structure/color.h>
#include <ugdk/structure/types.h>

// GLM headers
#include <glm/vec2.hpp>

#include <memory>
#include <iostream>
#include <tuple>

namespace {

using namespace ugdk;
using namespace glm;
using std::shared_ptr;

const glm::vec2 BOX_SIZE = { 50.0f, 50.0f };
const F32       SPEED = 500.0f;

} // unnamed namespace

int main(int argc, char *argv[]) {

    // UGDK initialization
    system::Configuration config;
    config.windows_list.front().canvas_size = math::Vector2D(1280, 720);
    config.windows_list.front().size        = math::Integer2D(1280, 720);
    system::Initialize(config);

    // Create scene
    auto scene = std::make_unique<ugdk::action::Scene>();

    // Exit event
    system::FunctionListener<input::KeyPressedEvent> exit_listener(
        [] (const ugdk::input::KeyPressedEvent& ev) {
            if (ev.scancode == ugdk::input::Scancode::ESCAPE)
                ugdk::system::CurrentScene().Finish();
        }
    );
    scene->event_handler().AddListener(exit_listener);

    // Box data
    F32 x = static_cast<F32>(BOX_SIZE.x);
    F32 y = static_cast<F32>(BOX_SIZE.x);
    vec2 box_position = {1280/2, 720/2};
    graphic::Mesh2D box(graphic::DrawMode::TRIANGLE_STRIP(), graphic::manager().white_texture());
    box.Fill({
        {.0f, .0f, .0f, .0f},
        {.0f,   y, .0f, 1.f},
        {  x, .0f, 1.f, 0.f},
        {  x,   y, 1.f, 1.f}
    });

    action::Scene *scene_ptr = scene.get();

    // Register joystick
    // We don't check for the already connected joysticks at the moment because there's none:
    // Even joysticks "already connected" when our application starts goes through the joystick
    // connection logic.
    // Also, it connects ALL joysticks your OS recognizes. This MIGHT include your keyboard.
    system::FunctionListener<input::JoystickConnectedEvent> joystick_connection_listener(
        [scene_ptr,&box_position](const auto& ev) {
            shared_ptr<input::Joystick> joystick = ev.joystick.lock();
            std::cout << "registered joystick " << joystick->name() << std::endl;
            // Box move task
            scene_ptr->AddTask([&box_position,joystick](double dt) {
                double move_x = joystick->GetAxisStatus(0).Percentage();
                double move_y = joystick->GetAxisStatus(1).Percentage();
                box_position += vec2(move_x, move_y)*SPEED*static_cast<F32>(dt);
            });
        }
    );
    scene->event_handler().AddListener(joystick_connection_listener);

    // Rendering
    scene->set_render_function(0u,
        [&box, &box_position](graphic::Canvas& canvas) {

            canvas.Clear(ugdk::structure::Color(0.2, 0.2, 0.2, 1));
            canvas.ChangeShaderProgram(graphic::manager().shaders().current_shader());
            canvas.PushAndCompose(math::Geometry(box_position - BOX_SIZE * 0.5f));

            canvas << box;

            canvas.PopGeometry();
        });

    system::PushScene(std::move(scene));
    system::Run();
    system::Release();
    return 0;
}
