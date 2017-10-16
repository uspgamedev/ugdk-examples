
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

#include <iostream>
#include <memory>

int main(int argc, char* argv[]) {
    using namespace ugdk;
    system::Configuration config;
    config.windows_list.front().canvas_size = math::Vector2D(1280, 720);
    config.windows_list.front().size        = math::Integer2D(1280, 720);

    config.windows_list.emplace_back();
    config.windows_list.back().canvas_size = math::Vector2D (1280/4, 720/4);
    config.windows_list.back().size        = math::Integer2D(1280/4, 720/4);
    config.windows_list.back().title       = "I'm its little brother";
    
    system::Initialize(config);

    system::FunctionListener<input::KeyPressedEvent> listener(
        [] (const ugdk::input::KeyPressedEvent& ev) {
            if (ev.scancode == ugdk::input::Scancode::ESCAPE)
                ugdk::system::CurrentScene().Finish();
        }
    );

    auto ourscene = std::make_unique<action::Scene>();

    ourscene->event_handler().AddListener(listener);
    ourscene->set_render_function([] (graphic::Canvas& canvas) {

        std::shared_ptr<desktop::Window> win_large = desktop::manager().window(0);
        std::shared_ptr<desktop::Window> win_small = desktop::manager().window(1);

        graphic::manager().AttachTo(win_small);
        graphic::Canvas canvas_small = graphic::Canvas(graphic::manager().screen());
        canvas_small.Clear(ugdk::structure::Color(0.2, 0.2, 0.2, 1));

        graphic::manager().AttachTo(win_large);
        graphic::Canvas canvas_large = graphic::Canvas(graphic::manager().screen());
        canvas_large.Clear(ugdk::structure::Color(0.2, 0.2, 0.2, 1));
        
    });

    system::PushScene(std::move(ourscene));

    system::Run();
    system::Release();
    return 0;
}
