#include <ugdk/system/engine.h>
#include <ugdk/action/scene.h>
#include <ugdk/input/events.h>
#include <ugdk/graphic/geometry.h>
#include <ugdk/graphic/canvas.h>
#include <ugdk/text/module.h>
#include <ugdk/text/label.h>
#include <ugdk/text/textbox.h>

#include <string>
#include <memory>

using namespace ugdk;

void QuitOnEscape(const ugdk::input::KeyPressedEvent& ev) {
    if (ev.scancode == ugdk::input::Scancode::ESCAPE)
        ugdk::system::CurrentScene().Finish();
}

int main(int argc, char* argv[]) {
    system::Configuration config;
    // EXAMPLE_LOCATION is defined by CMake to be the full path to the directory
    // that contains the source code for this example
    config.base_path = EXAMPLE_LOCATION "/content/"; 
    system::Initialize(config);

    text::manager()->AddFont("default", "epgyosho.ttf", 30);

    auto scene = ugdk::MakeUnique<ugdk::action::Scene>();
    scene->event_handler().AddListener(QuitOnEscape);
    {
        auto label = std::make_shared<text::Label>(ugdk::system::GetFileContents("hello.txt"), text::manager()->GetFont("default"));
        auto box = std::shared_ptr<text::TextBox>(text::manager()->GetTextFromFile("touhou.txt", "default"));

        scene->set_render_function([=](graphic::Canvas& canvas) {
            label->Draw(canvas);
            canvas.PushAndCompose(graphic::Geometry(math::Vector2D(0, label->height() + 50)));
            box->Draw(canvas);
            canvas.PopGeometry();
        });
    }
    system::PushScene(std::move(scene));

    system::Run();
    system::Release();
    return 0;
}
