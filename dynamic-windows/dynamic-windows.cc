
#include <ugdk/system/engine.h>
#include <ugdk/action/scene.h>
#include <ugdk/input/events.h>
#include <ugdk/system/compatibility.h>
#include <ugdk/graphic/canvas.h>
#include <ugdk/graphic/module.h>
#include <ugdk/graphic/rendertarget.h>
#include <ugdk/graphic/opengl.h>
#include <ugdk/desktop/module.h>
#include <ugdk/desktop/window.h>
#include <ugdk/desktop/windowsettings.h>
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
    
    auto ourscene = std::make_unique<action::Scene>();

    system::FunctionListener<input::KeyPressedEvent> quit_listener(
        [] (const ugdk::input::KeyPressedEvent &ev) {
            if (ev.scancode == ugdk::input::Scancode::ESCAPE)
                ugdk::system::CurrentScene().Finish();
        }
    );
    ourscene->event_handler().AddListener(quit_listener);

    system::FunctionListener<input::KeyReleasedEvent> close_window_listener(
        [&graphicman] (const ugdk::input::KeyReleasedEvent &ev) {
            if (!(ev.scancode == ugdk::input::Scancode::Q))
                return;
            uint32_t index = graphicman.num_targets();

            graphicman.UnregisterTarget(index); //this already destroys the window

            if (index == 1)
                ugdk::system::CurrentScene().Finish();
        }
    );
    ourscene->event_handler().AddListener(close_window_listener);
    
    system::FunctionListener<input::KeyReleasedEvent> add_window_listener(
        [&graphicman] (const ugdk::input::KeyReleasedEvent &ev) {
            if (!(ev.scancode == ugdk::input::Scancode::A))
                return;

            auto settings = desktop::WindowSettings();
                 settings.title = "New window"; 

            auto target = graphicman.target(graphicman.RegisterScreen(
                desktop::manager().CreateWindow(settings)));

            //auto screen = std::dynamic_pointer_cast<graphic::RenderScreen>(target);
            target->MyRenderer()->AddStep(
                [](graphic::Canvas& canvas){
                    canvas.Clear(ugdk::structure::Color(0.2, 0.2, 0.2, 1));
                });
        }
    );
    ourscene->event_handler().AddListener(add_window_listener);
    

    graphicman.target(0u)->MyRenderer()->AddStep(
        [] (graphic::Canvas& canvas_large) {
            canvas_large.Clear(ugdk::structure::Color(0.2, 0.2, 0.2, 1));
        });
    

    system::PushScene(std::move(ourscene));

    system::Run();
    system::Release();
    return 0;
}
