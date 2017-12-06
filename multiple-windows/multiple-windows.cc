
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

    config.windows_list.emplace_back();
    config.windows_list.back().canvas_size = math::Vector2D (1280/4, 720/4);
    config.windows_list.back().size        = math::Integer2D(1280/4, 720/4);
    config.windows_list.back().title       = "I'm its little brother";
    
    system::Initialize(config);
    auto &graphicman = graphic::manager();

    auto ourscene = std::make_unique<action::Scene>();

    system::FunctionListener<input::KeyPressedEvent> listener(
        [] (const ugdk::input::KeyPressedEvent& ev) {
            if (ev.scancode == ugdk::input::Scancode::ESCAPE)
                ugdk::system::CurrentScene().Finish();
        }
    );
    ourscene->event_handler().AddListener(listener);

    for (auto target : graphicman.targets()) {
        target->MyRenderer()->AddStep(
            [] (graphic::Canvas& canvas_large) {
            canvas_large.Clear(ugdk::structure::Color(0.2, 0.2, 0.2, 1));
        });
    }
    
    system::PushScene(std::move(ourscene));

    system::Run();
    system::Release();
    return 0;
}
