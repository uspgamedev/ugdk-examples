#include <ugdk/system/engine.h>
#include <ugdk/action/scene.h>
#include <ugdk/input/events.h>
#include <ugdk/desktop/window.h>
#include <ugdk/graphic/canvas.h>
#include <ugdk/graphic/module.h>
#include <ugdk/ui/drawable/texturedrectangle.h>

#include <memory>

using namespace ugdk;

namespace {
    const static math::Vector2D box_size(50.0, 50.0);
}

int main(int argc, char *argv[]) {
    system::Initialize();

    auto scene = ugdk::MakeUnique<ugdk::action::Scene>();
    math::Vector2D box_position;
    {
        auto rect = std::make_shared<ui::TexturedRectangle>(graphic::manager()->white_texture(), box_size);
        rect->set_hotspot(ui::HookPoint::CENTER);

        scene->event_handler().AddListener<input::MouseMotionEvent>([rect, &box_position](const input::MouseMotionEvent& ev) {
            auto window = ev.window.lock();
            box_position.x = double(ev.position.x) / window->size().x;
            box_position.y = double(ev.position.y) / window->size().y;
        });
        scene->set_render_function([rect, &box_position](graphic::Canvas& canvas) {
            math::Vector2D canvas_position = box_position.Scale(canvas.size());
            canvas.PushAndCompose(graphic::Geometry(canvas_position));
            rect->Draw(canvas);
            canvas.PopGeometry();
        });

    }
    system::PushScene(std::move(scene));

    system::Run();    
    system::Release();
    return 0;
}