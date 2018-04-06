
#include <ugdk/system/engine.h>
#include <ugdk/action/scene.h>
#include <ugdk/input/events.h>
#include <ugdk/system/compatibility.h>
#include <ugdk/graphic/canvas.h>
#include <ugdk/graphic/module.h>
#include <ugdk/graphic/rendertarget.h>
#include <ugdk/graphic/renderscreen.h>
#include <ugdk/graphic/opengl.h>
#include <ugdk/desktop/module.h>
#include <ugdk/desktop/window.h>
#include <ugdk/math/integer2D.h>

#include <iostream>
#include <memory>

int main(int argc, char* argv[]) {
    using namespace ugdk;
    system::Configuration config;
    config.windows_list.front().canvas_size = math::Vector2D(1280, 720);
    config.windows_list.front().size        = math::Integer2D(1280, 720);
    
    system::Initialize(config);
    auto &graphicman = graphic::manager();

    auto second_window_settings = desktop::WindowSettings();
    second_window_settings.canvas_size = math::Vector2D (1280/4, 720/4);
    second_window_settings.size        = math::Integer2D(1280/4, 720/4);
    second_window_settings.title       = "I'm its little brother";
    auto second_target = graphicman.RegisterScreen(
        desktop::manager().CreateWindow(second_window_settings)
    );

    auto ourscene = std::make_unique<action::Scene>();

    system::FunctionListener<input::KeyPressedEvent> listener(
        [] (const ugdk::input::KeyPressedEvent& ev) {
            if (ev.scancode == ugdk::input::Scancode::ESCAPE)
                ugdk::system::CurrentScene().Finish();
        }
    );
    ourscene->event_handler().AddListener(listener);

<<<<<<< 091edda7f113318e205b0fa6f55d964ebf75a968
    for (auto target : graphicman.targets()) {
        target->MyRenderer()->AddStep(
            [] (graphic::Canvas& canvas_large) {
            canvas_large.Clear(ugdk::structure::Color(0.2, 0.2, 0.2, 1));
        });
    }
    
=======
    
    graphicman.default_target().lock()->MyRenderer()->AddStep(
        [] (graphic::Canvas& canvas_large) {
            canvas_large.Clear(ugdk::structure::Color(0.2, 0.2, 0.2, 1));
        });
    second_target.lock()->MyRenderer()->AddStep(
        [] (graphic::Canvas& canvas_small) {
            canvas_small.Clear(ugdk::structure::Color(0.3, 0.3, 0.3, 1));
        });

>>>>>>> Update examples to non-indexed render targets
    system::PushScene(std::move(ourscene));

    system::Run();
    system::Release();
    return 0;
}
