
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

const dvec2             BOX_SIZE(50.0, 50.0);
const F64               SPEED = 500.0;
const math::Integer2D   SCREEN_SIZE(1280, 720)

}

int main(int argc, char *argv[]) {

    // UGDK initialization
    system::Configuration config;
    config.windows_list.front().canvas_size = dvec2(1280, 720);
    config.windows_list.front().size        = SCREEN_SIZE;
    system::Initialize(config);

    auto &graphic = graphic::manager();
    auto weak_tex = graphic.RegisterTexture(SCREEN_SIZE);

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

    // Rendering
    auto texture_renderer = weak_tex.lock()->MyRenderer();
    auto texture_function = [&box, &box_position](graphic::Canvas& canvas) {
        canvas.Clear(ugdk::structure::Color(0.2, 0.2, 0.2, 1));
        canvas.ChangeShaderProgram(graphic::manager().shaders().current_shader());
        canvas.PushAndCompose(math::Geometry(box_position - BOX_SIZE * 0.5));

        canvas << box;
        canvas.PopGeometry(texture_function);
    };
    texture_renderer->AddStep(texture_function);
    


    auto renderer_ptr = graphicman.default_target().lock()->MyRenderer();
    renderer_ptr->AddStep(
        );
    renderer_ptr->AddStep(

    );

    system::PushScene(std::move(ourscene));
    system::Run();
    system::Release();
    return 0;
}
