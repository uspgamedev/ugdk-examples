
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
#include <ugdk/math/integer2D.h>

#include <iostream>
#include <memory>

int main(int argc, char* argv[]) {
    using namespace ugdk;
    system::Configuration config;
    config.windows_list.front().canvas_size = math::Vector2D(1280, 720);
    config.windows_list.front().size        = math::Integer2D(1280, 720);

    system::Initialize(config);
    
    auto ourscene = std::make_unique<action::Scene>();
    auto ptr      = ourscene.get();

    system::FunctionListener<input::KeyPressedEvent> quit_listener(
        [] (const ugdk::input::KeyPressedEvent& ev) {
            if (ev.scancode == ugdk::input::Scancode::ESCAPE)
                ugdk::system::CurrentScene().Finish();
        }
    );

    system::FunctionListener<input::KeyPressedEvent> add_window_listener(
        [] (const ugdk::input::KeyPressedEvent& ev) {
            auto &scene   = ugdk::system::CurrentScene();
            auto &manager = desktop::manager();

            std::string title = std::string("window "+std::to_string(desktop::manager().num_windows()));
            using desktop::WindowSettings;
            if (ev.keycode == ugdk::input::Keycode::a) {
                desktop::WindowSettings settings (
                    WindowSettings(title
                        , ""
                        , math::Integer2D (1280/4, 720/4)
                        , math::Vector2D (1280/4, 720/4)
                        , false
                        , true
                ));
                manager.CreateWindow(settings);
                uint32_t index = manager.num_windows() - 1;
                scene.set_render_function(index,
                    [] (graphic::Canvas& canvas) {
                        canvas.Clear(ugdk::structure::Color(0.2, 0.2, 0.2, 1));
                    });
            }
        }
    );

    system::FunctionListener<input::KeyPressedEvent> close_window_listener(
        [] (const ugdk::input::KeyPressedEvent& ev) {
            auto &manager = desktop::manager();
            auto &scene   = ugdk::system::CurrentScene();
            if (!manager.num_windows())
                ugdk::system::CurrentScene().Finish();

            if (ev.keycode == ugdk::input::Keycode::q) {
                manager.DestroyWindow(manager.num_windows()-1);
                scene.remove_render_function(scene.num_functions()-1);
            }
        }
    );

    ourscene->set_render_function(0u,
        [] (graphic::Canvas& canvas_large) {
            canvas_large.Clear(ugdk::structure::Color(0.2, 0.2, 0.2, 1));
        });
    
    ourscene->event_handler().AddListener(quit_listener);
    ourscene->event_handler().AddListener(add_window_listener);
    ourscene->event_handler().AddListener(close_window_listener);

    system::PushScene(std::move(ourscene));

    system::Run();
    system::Release();
    return 0;
}
