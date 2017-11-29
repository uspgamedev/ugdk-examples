
#include <ugdk/system/engine.h>
#include <ugdk/action/scene.h>
#include <ugdk/input/events.h>
#include <ugdk/system/compatibility.h>

#include <ugdk/graphic/canvas.h>
#include <ugdk/graphic/rendertarget.h>
#include <ugdk/graphic/module.h>
#include <ugdk/graphic/opengl.h>

#include <ugdk/desktop/module.h>
#include <ugdk/desktop/window.h>
#include <ugdk/math/integer2D.h>

#include <vector>
#include <iostream>
#include <memory>

using namespace ugdk;

int main(int argc, char* argv[]) {
    using namespace ugdk;
    
    system::Configuration config;
    auto window_conf = config.windows_list.front();
    
    window_conf.canvas_size = math::Vector2D(1280, 720);
    window_conf.size        = math::Integer2D(1280, 720);
    
    system::Initialize(config);
    
    auto &desktopman = desktop::manager();
    auto &graphicman = graphic::manager();
    auto ourscene    = std::make_unique<action::Scene>();

    // And we access the RenderTarget's Renderer and add a step, which is a lambda
    graphicman.target(0u)->MyRenderer()->AddStep(
        [](graphic::Canvas& canvas) {
            canvas.Clear(ugdk::structure::Color(0.2, 0.2, 0.2, 1));
        });

    // This is a listener for the ESC key press event, to quit out.
    system::FunctionListener<input::KeyPressedEvent> listener(
        [] (const ugdk::input::KeyPressedEvent& ev) {
            if (ev.scancode == ugdk::input::Scancode::ESCAPE)
                ugdk::system::CurrentScene().Finish();
        }
    );
    ourscene->event_handler().AddListener(listener); 

    system::PushScene(std::move(ourscene));

    system::Run();
    system::Release();
    return 0;
}
