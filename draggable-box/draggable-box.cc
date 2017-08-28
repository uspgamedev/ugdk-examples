#include <ugdk/system/engine.h>
#include <ugdk/action/scene.h>
#include <ugdk/input/events.h>
#include <ugdk/desktop/window.h>
#include <ugdk/desktop/module.h>
#include <ugdk/graphic/canvas.h>
#include <ugdk/graphic/module.h>
#include <ugdk/graphic/opengl.h>
#include <ugdk/structure/color.h>
#include <ugdk/system/compatibility.h>
#include <ugdk/ui/drawable/texturedrectangle.h>

#include <memory>
#include <iostream>

using namespace ugdk;

namespace {
const math::Vector2D box_size(50.0, 50.0);
void QuitOnEscape(const ugdk::input::KeyPressedEvent& ev) {
    if (ev.scancode == ugdk::input::Scancode::ESCAPE)
        ugdk::system::CurrentScene().Finish();
}
}

int main(int argc, char *argv[]) {
    system::Initialize();
    if (!graphic::Initialize(std::make_unique<graphic::Manager>(), desktop::manager().primary_window(), math::Vector2D(800, 600)))
        std::cout << "Deu ruim\n";
    auto scene = ugdk::MakeUnique<ugdk::action::Scene>();
    math::Vector2D box_position;

    auto rect = std::make_shared<ui::TexturedRectangle>(graphic::manager().white_texture(), box_size);
    rect->set_hotspot(ui::HookPoint::CENTER);
    system::FunctionListener<input::KeyPressedEvent> listener2(QuitOnEscape);
    system::FunctionListener<input::MouseMotionEvent> listener([rect, &box_position](const input::MouseMotionEvent& ev) {
        auto window = ev.window.lock();
        box_position.x = double(ev.position.x) / window->size().x;
        box_position.y = double(ev.position.y) / window->size().y;
    });


    scene->event_handler().AddListener(listener2);
    scene->event_handler().AddListener<input::MouseMotionEvent>(listener);
    scene->set_render_function([rect, &box_position](graphic::Canvas& canvas) {
        canvas.Clear(ugdk::structure::Color(0.2, 0.2, 0.2, 1));
        canvas.ChangeShaderProgram(graphic::manager().shaders().current_shader());
        math::Vector2D canvas_position = box_position.Scale(canvas.size());
        canvas.PushAndCompose(math::Geometry(canvas_position));
        rect->Draw(canvas);
        canvas.PopGeometry();
    });

    system::PushScene(std::move(scene));
    system::Run();
    system::Release();
    return 0;
}
