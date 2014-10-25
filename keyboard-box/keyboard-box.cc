#include <ugdk/system/engine.h>
#include <ugdk/action/scene.h>
#include <ugdk/input/module.h>
#include <ugdk/input/events.h>
#include <ugdk/input/scancode.h>
#include <ugdk/graphic/canvas.h>
#include <ugdk/graphic/vertexdata.h>
#include <ugdk/graphic/module.h>
#include <ugdk/structure/vertex.h>

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
      , vertexdata_(4, sizeof(structure::VertexXYUV), true)
    {
        graphic::VertexData::Mapper mapper(vertexdata_);
        mapper.Get<structure::VertexXYUV>(0)->set_xyuv(          0.0f,           0.0f, 0.0f, 0.0f);
        mapper.Get<structure::VertexXYUV>(1)->set_xyuv(          0.0f, RECTANGLE_SIZE, 0.0f, 1.0f);
        mapper.Get<structure::VertexXYUV>(2)->set_xyuv(RECTANGLE_SIZE,           0.0f, 1.0f, 0.0f);
        mapper.Get<structure::VertexXYUV>(3)->set_xyuv(RECTANGLE_SIZE, RECTANGLE_SIZE, 1.0f, 1.0f);
    }

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
        canvas.SendVertexData(vertexdata_, graphic::VertexType::VERTEX, offsetof(structure::VertexXYUV, x));
        canvas.SendVertexData(vertexdata_, graphic::VertexType::TEXTURE, offsetof(structure::VertexXYUV, u));
        auto unit = graphic::manager()->ReserveTextureUnit(graphic::manager()->white_texture());
        canvas.SendUniform("drawable_texture", unit);
        canvas.DrawArrays(graphic::DrawMode::TRIANGLE_STRIP(), 0, 4);
        canvas.PopGeometry();
    }

  private:
    double velocity_;
    math::Vector2D position_;
    graphic::VertexData vertexdata_;
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