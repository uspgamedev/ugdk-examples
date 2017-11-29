
#include <ugdk/system/engine.h>
#include <ugdk/action/scene.h>
#include <ugdk/input/events.h>
#include <ugdk/input/module.h>
#include <ugdk/desktop/window.h>
#include <ugdk/desktop/module.h>
#include <ugdk/graphic/canvas.h>
#include <ugdk/graphic/drawmode.h>
#include <ugdk/graphic/module.h>
#include <ugdk/graphic/mesh.h>
#include <ugdk/graphic/rendertarget.h>
#include <ugdk/structure/color.h>
#include <ugdk/structure/types.h>
#include <ugdk/structure/vertex.h>
#include <ugdk/system/compatibility.h>

#include <glm/vec2.hpp>

#include <memory>
#include <iostream>
#include <tuple>

using namespace ugdk;
using namespace glm;

namespace {

const dvec2     BOX_SIZE(50.0, 50.0);
const F64       SPEED = 500.0;

}

int main(int argc, char *argv[]) {

    // UGDK initialization
    system::Configuration config;
    config.windows_list.front().canvas_size = dvec2(1280, 720);
    config.windows_list.front().size        = math::Integer2D(1280, 720);
    system::Initialize(config);

    auto &graphicman = graphic::manager();

    // Create scene
    auto ourscene    = std::make_unique<action::Scene>();

    // This is a listener for the ESC key press event, to quit out.
    system::FunctionListener<input::KeyPressedEvent> esc_listener(
        [] (const ugdk::input::KeyPressedEvent& ev) {
            if (ev.scancode == ugdk::input::Scancode::ESCAPE)
                ugdk::system::CurrentScene().Finish();
        }
    );
    ourscene->event_handler().AddListener(esc_listener); 

    // Box data
    F32 x = static_cast<F32>(BOX_SIZE.x);
    F32 y = static_cast<F32>(BOX_SIZE.y);
    dvec2 box_position;
    graphic::Mesh2D box(graphic::DrawMode::TRIANGLE_STRIP(), graphic::manager().white_texture());
    box.Fill({
        {.0f, .0f, .0f, .0f},
        {.0f,   y, .0f, 1.f},
        {  x, .0f, 1.f, 0.f},
        {  x,   y, 1.f, 1.f}
    });

    // Box move task
    ourscene->AddTask([&box_position](double dt) {
        auto &manager = input::manager();
        dvec2 dir;
        if(manager.keyboard().IsDown(input::Scancode::A))
            dir += dvec2(-1.0, 0.0);
        if(manager.keyboard().IsDown(input::Scancode::D))
            dir += dvec2(1.0, 0.0);
        if(manager.keyboard().IsDown(input::Scancode::W))
            dir += dvec2(0.0, -1.0);
        if(manager.keyboard().IsDown(input::Scancode::S))
            dir += dvec2(0.0, 1.0);
        box_position += dir * SPEED * dt;
    });

    // Rendering
    graphicman.target(0u)->MyRenderer()->AddStep(
        [&box, &box_position](graphic::Canvas& canvas) {

            canvas.Clear(ugdk::structure::Color(0.2, 0.2, 0.2, 1));
            canvas.ChangeShaderProgram(graphic::manager().shaders().current_shader());
            canvas.PushAndCompose(math::Geometry(box_position - BOX_SIZE * 0.5));

            canvas << box;
            canvas.PopGeometry();
        });

    system::PushScene(std::move(ourscene));
    system::Run();
    system::Release();
    return 0;
}
