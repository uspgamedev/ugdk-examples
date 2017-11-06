
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
    
    auto ourscene = std::make_unique<action::Scene>();

    system::FunctionListener<input::KeyPressedEvent> quit_listener(
        [] (const ugdk::input::KeyPressedEvent &ev) {
            if (ev.scancode == ugdk::input::Scancode::ESCAPE)
                ugdk::system::CurrentScene().Finish();
        }
    );

    system::FunctionListener<input::KeyReleasedEvent> close_window_listener(
        [] (const ugdk::input::KeyReleasedEvent &ev) {
            if (!(ev.scancode == ugdk::input::Scancode::Q))
                return;
            auto &dmanager = desktop::manager();
            auto &gmanager = graphic::manager();

            gmanager.DeregisterScreen(gmanager.num_screens()-1);
            dmanager.DestroyWindow(dmanager.num_windows()-1);

            if (!dmanager.num_windows())
                ugdk::system::CurrentScene().Finish();
        }
    );
    
    system::FunctionListener<input::KeyReleasedEvent> add_window_listener(
        [] (const ugdk::input::KeyReleasedEvent &ev) {
            if (!(ev.scancode == ugdk::input::Scancode::A))
                return;

            auto &scene  = ugdk::system::CurrentScene();           
            auto index   = desktop::manager().num_windows();
            auto &deskmanager = desktop::manager();            
            auto &gphcmanager = graphic::manager();
            
            auto  settings = desktop::WindowSettings();
            settings.title = "Window no. " + std::to_string(deskmanager.num_windows()); 

            deskmanager.CreateWindow( settings );
            deskmanager.window( deskmanager.num_windows()-1 );
            gphcmanager.RegisterScreen( deskmanager.window(index) );

            scene.set_render_function(index,
                                        [](graphic::Canvas& canvas){
                                            canvas.Clear(ugdk::structure::Color(0.2, 0.2, 0.2, 1));
                                        }
                                     );
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
