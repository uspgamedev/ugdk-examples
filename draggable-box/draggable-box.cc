
#include <ugdk/system/engine.h>
#include <ugdk/action/scene.h>
#include <ugdk/input/events.h>
#include <ugdk/desktop/window.h>
#include <ugdk/desktop/module.h>
#include <ugdk/graphic/canvas.h>
#include <ugdk/graphic/drawmode.h>
#include <ugdk/graphic/module.h>
#include <ugdk/graphic/vertexdata.h>
#include <ugdk/structure/color.h>
#include <ugdk/structure/types.h>
#include <ugdk/system/compatibility.h>

#include <memory>
#include <vector>
#include <iostream>
#include <tuple>

using namespace ugdk;

namespace {

const math::Vector2D BOX_SIZE(50.0, 50.0);

}

int main(int argc, char *argv[]) {

    // UGDK initialization
    system::Configuration config;
    config.windows_list.front().canvas_size = math::Vector2D(1280, 720);
    config.windows_list.front().size        = math::Integer2D(1280, 720);
    system::Initialize(config);

    // Create scene
    auto scene = ugdk::MakeUnique<ugdk::action::Scene>();

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
    Vector2D box_position;
    graphic::Mesh box = (
        graphic::DrawMode::TRIANGLE_STRIP(),
        {
            .0f, .0f,
            .0f,   y,
              x, .0f,
              x,   y
        }
    );

    // Box drag event
    system::FunctionListener<input::MouseMotionEvent> box_listener(
        [&box_position](const input::MouseMotionEvent& ev) {
            box_position.x = static_cast<double>(ev.position.x);
            box_position.y = static_cast<double>(ev.position.y);
        }
    );
    scene->event_handler().AddListener<input::MouseMotionEvent>(box_listener);

    // Rendering
    scene->set_render_function(0u,
        [&box,&box_position](graphic::Canvas& canvas) {
            auto &pos = box_position;

            canvas.Clear(ugdk::structure::Color(0.2, 0.2, 0.2, 1));
            //canvas.ChangeShaderProgram(graphic::manager().shaders().current_shader());
            canvas.PushAndCompose(math::Geometry(pos - BOX_SIZE/2));

            canvas << box;

            canvas.PopGeometry();
        });
    
    system::PushScene(std::move(scene));
    system::Run();
    system::Release();
    return 0;
}
