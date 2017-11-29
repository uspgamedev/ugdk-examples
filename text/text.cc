
#include <ugdk/system/engine.h>
#include <ugdk/action/scene.h>
#include <ugdk/input/events.h>
#include <ugdk/graphic/canvas.h>
#include <ugdk/graphic/rendertarget.h>
#include <ugdk/graphic/module.h>
#include <ugdk/text/module.h>
#include <ugdk/text/label.h>
#include <ugdk/system/compatibility.h>
#include <ugdk/math/vector2D.h>
#include <ugdk/desktop/module.h>
#include <ugdk/desktop/window.h>

#include <string>
#include <memory>

using namespace ugdk;
using math::Vector2D;

int main(int argc, char* argv[]) {

    // UGDK initialization
    // EXAMPLE_LOCATION is defined by CMake to be the full path to the directory
    // that contains the source code for this example */
    system::Configuration config;
    config.base_path = EXAMPLE_LOCATION "/content/";
    config.windows_list.front().canvas_size = math::Vector2D(1280, 720);
    config.windows_list.front().size        = math::Integer2D(1280, 720);
    system::Initialize(config);
    
    auto &graphicman = graphic::manager();

    // Create scene
    auto ourscene    = std::make_unique<action::Scene>();

    // Load font
    text::manager().AddFont("default", "epgyosho.ttf", 40);

    // This is a listener for the ESC key press event, to quit out.
    system::FunctionListener<input::KeyPressedEvent> esc_listener(
        [] (const ugdk::input::KeyPressedEvent& ev) {
            if (ev.scancode == ugdk::input::Scancode::ESCAPE)
                ugdk::system::CurrentScene().Finish();
        }
    );
    ourscene->event_handler().AddListener(esc_listener); 

    {
        auto label = std::make_shared<text::Label>("Hello World!",
                                                   text::manager().GetFont("default"));

        graphicman.target(0u)->MyRenderer()->AddStep(
            [label](graphic::Canvas& canvas) {
                using namespace graphic;

                canvas.Clear(ugdk::structure::Color(0.2, 0.2, 0.2, 1));
                canvas.ChangeShaderProgram(graphic::manager().shaders().current_shader());

                canvas.PushAndCompose(canvas.size()/2.0 - label->size()/2.0);
                canvas << *label;
                canvas.PopGeometry();
            });
    }
    system::PushScene(std::move(ourscene));

    system::Run();
    system::Release();
    return 0;
}
