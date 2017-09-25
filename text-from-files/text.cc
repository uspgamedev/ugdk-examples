
#include <ugdk/action/scene.h>
#include <ugdk/input/events.h>
#include <ugdk/graphic/canvas.h>
#include <ugdk/graphic/module.h>
#include <ugdk/math/vector2D.h>
#include <ugdk/text/module.h>
#include <ugdk/text/label.h>
#include <ugdk/text/textbox.h>
#include <ugdk/system/engine.h>
#include <ugdk/system/compatibility.h>
#include <ugdk/filesystem/module.h>
#include <ugdk/filesystem/file.h>

#include <string>
#include <memory>

using namespace ugdk;
using math::Vector2D;

int main(int argc, char* argv[]) {

    // EXAMPLE_LOCATION is defined by CMake to be the full path to the directory
    // that contains the source code for this example
    system::Configuration config;
    config.base_path = EXAMPLE_LOCATION "/content/";
    config.canvas_size = Vector2D(1280, 720);
    config.windows_list.front().size = math::Integer2D(1280, 720);
    system::Initialize(config);

    // Load font
    text::manager().AddFont("default", "epgyosho.ttf", 30);

    // Create scene
    auto scene = ugdk::MakeUnique<ugdk::action::Scene>();

    // Exit event
    system::FunctionListener<input::KeyPressedEvent> exit_listener(
        [] (const ugdk::input::KeyPressedEvent& ev) {
            if (ev.scancode == ugdk::input::Scancode::ESCAPE)
                ugdk::system::CurrentScene().Finish();
        }
    );
    scene->event_handler().AddListener(exit_listener);

    {
        auto file = ugdk::filesystem::manager().OpenFile("hello.txt");
        auto label = std::make_shared<text::Label>(file->GetContents(),
                                                   text::manager().GetFont("default"));
        auto box = std::shared_ptr<text::TextBox>(text::manager().GetTextFromFile("touhou.txt",
                                                  "default"));

        scene->set_render_function([=](graphic::Canvas& canvas) {
            using namespace graphic;

            canvas.Clear(ugdk::structure::Color(0.2, 0.2, 0.2, 1));
            canvas.ChangeShaderProgram(graphic::manager().shaders().current_shader());

            canvas << *label;
            canvas.PushAndCompose(math::Vector2D(0, label->height() + 50));
            canvas << *box;
            canvas.PopGeometry();
        });
    }
    system::PushScene(std::move(scene));

    system::Run();
    system::Release();
    return 0;
}
