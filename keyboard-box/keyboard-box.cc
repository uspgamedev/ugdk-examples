#include <ugdk/system/engine.h>
#include <ugdk/action/scene.h>
#include <ugdk/input/module.h>
#include <ugdk/input/events.h>
#include <ugdk/input/scancode.h>
#include <ugdk/graphic/canvas.h>
#include <ugdk/graphic/vertexdata.h>
#include <ugdk/graphic/module.h>
#include <ugdk/structure/vertex.h>
#include <ugdk/ui/drawable/texturedrectangle.h>

using namespace ugdk;

namespace {
    const static float RECTANGLE_SIZE = 50.0f;
}

void QuitOnEscape(const ugdk::input::KeyPressedEvent& ev) {
    if (ev.scancode == ugdk::input::Scancode::ESCAPE)
        ugdk::system::CurrentScene().Finish();
}

class Rectangle {
  public:
    Rectangle() 
      : velocity_(500.0)
      , drawable_(new ui::TexturedRectangle(graphic::manager()->white_texture(), math::Vector2D(RECTANGLE_SIZE, RECTANGLE_SIZE)))
    {}

    void Update(double dt) {
        auto manager = input::manager();
        if(manager->keyboard().IsDown(input::Scancode::A))
            MoveLeft(dt);
        if(manager->keyboard().IsDown(input::Scancode::D))
            MoveRight(dt);
        if(manager->keyboard().IsDown(input::Scancode::W))
            MoveUp(dt);
        if(manager->keyboard().IsDown(input::Scancode::S))
            MoveDown(dt);
    }
    
    void MoveLeft(double dt) {
        position_.x -= dt*velocity_;
    }
    void MoveRight(double dt) {
        position_.x += dt*velocity_;
    }
    void MoveUp(double dt) {
        position_.y -= dt*velocity_;
    }
    void MoveDown(double dt) {
        position_.y += dt*velocity_;
    }

    void Render(graphic::Canvas& canvas) const {
        canvas.PushAndCompose(graphic::Geometry(position_));
        drawable_->Draw(canvas);
        canvas.PopGeometry();
    }

  private:
    double velocity_;
    math::Vector2D position_;
    std::unique_ptr<ui::TexturedRectangle> drawable_;
};

int main(int argc, char* argv[]) {
    system::Initialize();

    auto scene = ugdk::MakeUnique<ugdk::action::Scene>();
    scene->event_handler().AddListener(QuitOnEscape);

    {
        auto r = std::make_shared<Rectangle>();
        scene->AddTask([r](double dt) {
            r->Update(dt);
        });
        scene->set_render_function([r](graphic::Canvas& canvas) {
            r->Render(canvas);
        });
    }
    system::PushScene(std::move(scene));

    system::Run();
    system::Release();
    return 0;
}