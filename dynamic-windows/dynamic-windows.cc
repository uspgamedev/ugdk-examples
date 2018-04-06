
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
#include <ugdk/desktop/windowsettings.h>
#include <ugdk/math/integer2D.h>

#include <iostream>
#include <memory>

using std::vector;
using std::weak_ptr;

int main(int argc, char* argv[]) {
    using namespace ugdk;
    system::Configuration config;
    config.windows_list.front().canvas_size = math::Vector2D(1280, 720);
    config.windows_list.front().size        = math::Integer2D(1280, 720);

    system::Initialize(config);
    auto &graphicman = graphic::manager();
    vector<weak_ptr<graphic::RenderScreen>> windows;
    
    auto ourscene = std::make_unique<action::Scene>();

    system::FunctionListener<input::KeyPressedEvent> quit_listener(
        [] (const ugdk::input::KeyPressedEvent &ev) {
            if (ev.scancode == ugdk::input::Scancode::ESCAPE)
                ugdk::system::CurrentScene().Finish();
        }
    );
    ourscene->event_handler().AddListener(quit_listener);

    system::FunctionListener<input::KeyReleasedEvent> close_window_listener(
        [&graphicman, &windows] (const ugdk::input::KeyReleasedEvent &ev) {
            if (!(ev.scancode == ugdk::input::Scancode::Q))
                return;
            while (windows.size() > 0u) {
                auto target = windows.back();
                windows.pop_back();
                if (!target.expired()) {
                    graphicman.UnregisterTarget(target); //this already destroys the window
                    break;
                }
            }
        }
    );
    ourscene->event_handler().AddListener(close_window_listener);
    
    system::FunctionListener<input::KeyReleasedEvent> add_window_listener(
        [&graphicman, &windows] (const ugdk::input::KeyReleasedEvent &ev) {
            if (!(ev.scancode == ugdk::input::Scancode::A))
                return;

            auto settings = desktop::WindowSettings();
                 settings.title = "New window";

            auto target = graphicman.RegisterScreen(desktop::manager().CreateWindow(settings));
            target.lock()->MyRenderer()->AddStep(
                [](graphic::Canvas& canvas){
                    canvas.Clear(ugdk::structure::Color(0.2, 0.2, 0.2, 1));
                });
        }
    );
    ourscene->event_handler().AddListener(add_window_listener);
    

    graphicman.default_target().lock()->MyRenderer()->AddStep(
        [] (graphic::Canvas& canvas_large) {
            canvas_large.Clear(ugdk::structure::Color(0.2, 0.2, 0.2, 1));
        });
    

    system::PushScene(std::move(ourscene));

    system::Run();
    system::Release();
    return 0;
}
