
#include <ugdk/system/engine.h>
#include <ugdk/action/scene.h>
#include <ugdk/input/events.h>
#include <ugdk/system/compatibility.h>
#include <ugdk/graphic/canvas.h>
#include <ugdk/graphic/module.h>
#include <ugdk/graphic/opengl.h>
#include <ugdk/desktop/module.h>
#include <ugdk/desktop/window.h>
#include <ugdk/math/integer2D.h>

#include <vector>
#include <iostream>
#include <memory>

int main(int argc, char* argv[]) {
    using namespace ugdk;
    system::Configuration config;
    config.windows_list.front().canvas_size = math::Vector2D(1280, 720);
    config.windows_list.front().size        = math::Integer2D(1280, 720);
    system::Initialize(config);

    system::FunctionListener<input::KeyPressedEvent> listener(
        [] (const ugdk::input::KeyPressedEvent& ev) {
            if (ev.scancode == ugdk::input::Scancode::ESCAPE)
                ugdk::system::CurrentScene().Finish();
        }
    );

    auto ourscene = std::make_unique<action::Scene>();

    ourscene->event_handler().AddListener(listener);
    ourscene->set_render_function([] (const std::vector<graphic::Canvas*>& canvases) {
        
        // Get the primary window
        std::shared_ptr<desktop::Window> window = desktop::manager().window(0).lock();
        
        // Get a reference to the primary-window canvas
        auto &canvas = *canvases[0];

        // Set the current screen we are drawing on
        graphic::manager().SetActiveScreen(0);

        //Tell the engine we are using our canvas
        graphic::manager().UseCanvas(canvas);  
        
        canvas.Clear(ugdk::structure::Color(0.2, 0.2, 0.2, 1));

        window->Present();
    });

    system::PushScene(std::move(ourscene));

    system::Run();
    system::Release();
    return 0;
}
