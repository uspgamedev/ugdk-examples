
#include <ugdk/system/engine.h>
#include <ugdk/action/scene.h>
#include <ugdk/input/events.h>
#include <ugdk/input/module.h>
#include <ugdk/desktop/window.h>
#include <ugdk/desktop/module.h>
#include <ugdk/graphic/canvas.h>
#include <ugdk/graphic/drawmode.h>
#include <ugdk/graphic/module.h>
#include <ugdk/graphic/vertexdata.h>
#include <ugdk/structure/color.h>
#include <ugdk/structure/types.h>
#include <ugdk/system/compatibility.h>

#include <memory>
#include <iostream>
#include <tuple>

using namespace ugdk;
using math::Vector2D;

namespace {

const Vector2D  BOX_SIZE(50.0, 50.0);
const double    SPEED = 500.0;

struct VertexXYUV {
    F32 x, y, u, v;
};

struct Box {
    Vector2D pos;
    graphic::GLTexture *tex;
    graphic::VertexData vtx;
};

}

int main(int argc, char *argv[]) {

    // UGDK initialization
    system::Configuration config;
    config.windows_list.front().canvas_size = math::Vector2D(1280, 720);
    config.windows_list.front().size        = math::Integer2D(1280, 720);
    system::Initialize(config);

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

    // Box data
    Box box = {
        math::Vector2D(1280.0, 720.0)/2.0,
        graphic::manager().white_texture(),
        std::move(graphic::VertexData(4u, sizeof(VertexXYUV), false))
    };

    // Map box vertices
    {
        graphic::VertexData::Mapper mapper(box.vtx, false);
        F32 x = static_cast<F32>(BOX_SIZE.x);
        F32 y = static_cast<F32>(BOX_SIZE.x);
        mapper.Get<VertexXYUV>(0u) = {.0f, .0f, .0f, .0f};
        mapper.Get<VertexXYUV>(1u) = {.0f, y,   .0f, .1f};
        mapper.Get<VertexXYUV>(2u) = {x,   .0f, .1f, .0f};
        mapper.Get<VertexXYUV>(3u) = {x,   y,   .1f, .1f};
    }

    // Box move task
    scene->AddTask([&box](double dt) {
        auto &manager = input::manager();
        if(manager.keyboard().IsDown(input::Scancode::A))
            box.pos += Vector2D(-1.0, 0.0)*SPEED*dt;
        if(manager.keyboard().IsDown(input::Scancode::D))
            box.pos += Vector2D(1.0, 0.0)*SPEED*dt;
        if(manager.keyboard().IsDown(input::Scancode::W))
            box.pos += Vector2D(0.0, -1.0)*SPEED*dt;
        if(manager.keyboard().IsDown(input::Scancode::S))
            box.pos += Vector2D(0.0, 1.0)*SPEED*dt;
    });

    // Rendering
    scene->set_render_function(0u,
        [&box](graphic::Canvas& canvas) {
            auto &pos = box.pos;
            auto &texture = box.tex;
            auto &vertex_data = box.vtx;

            canvas.Clear(ugdk::structure::Color(0.2, 0.2, 0.2, 1));
            canvas.ChangeShaderProgram(graphic::manager().shaders().current_shader());
            canvas.PushAndCompose(math::Geometry(pos - BOX_SIZE/2));

            graphic::TextureUnit unit = graphic::manager().ReserveTextureUnit(texture);
            canvas.SendUniform("drawable_texture", unit);

            canvas.SendVertexData(vertex_data, graphic::VertexType::VERTEX, 0, 2);
            canvas.SendVertexData(vertex_data, graphic::VertexType::TEXTURE, 2 * sizeof(F32), 2);
            canvas.DrawArrays(graphic::DrawMode::TRIANGLE_STRIP(), 0, 4);

            canvas.PopGeometry();
        });

    system::PushScene(std::move(scene));
    system::Run();
    system::Release();
    return 0;
}
