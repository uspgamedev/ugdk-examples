
#include <ugdk/system/engine.h>
#include <ugdk/action/scene.h>
#include <ugdk/input/events.h>
#include <ugdk/desktop/window.h>
#include <ugdk/desktop/module.h>
#include <ugdk/graphic/canvas.h>
#include <ugdk/graphic/module.h>
#include <ugdk/graphic/vertexdata.h>
#include <ugdk/graphic/opengl.h>
#include <ugdk/structure/color.h>
#include <ugdk/structure/types.h>
#include <ugdk/system/compatibility.h>

#include <memory>
#include <iostream>
#include <tuple>

using namespace ugdk;

namespace {

const math::Vector2D BOX_SIZE(50.0, 50.0);

struct VertexXYUV {
    F32 x, y, u, v;
};

struct Box {
    math::Vector2D pos;
    graphic::GLTexture *tex;
    graphic::VertexData vtx;
};

}

int main(int argc, char *argv[]) {

    // UGDK initialization
    system::Configuration config;
    config.canvas_size = math::Vector2D(1280, 720);
    config.windows_list.front().size = math::Integer2D(1280, 720);
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
        math::Vector2D(0,0),
        graphic::manager().white_texture(),
        std::move(graphic::VertexData(4u, sizeof(VertexXYUV), false))
    };

    system::FunctionListener<input::MouseMotionEvent> box_listener([&box](const input::MouseMotionEvent& ev) {
        auto window = ev.window.lock();
        box.pos.x = double(ev.position.x) / window->size().x;
        box.pos.y = double(ev.position.y) / window->size().y;
    });
    scene->event_handler().AddListener<input::MouseMotionEvent>(box_listener);

    // Rendering
    scene->set_render_function([&box](graphic::Canvas& canvas) {
        auto &pos = box.pos;
        auto &texture = box.tex;
        auto &vertex_data = box.vtx;

        canvas.Clear(ugdk::structure::Color(0.2, 0.2, 0.2, 1));
        canvas.ChangeShaderProgram(graphic::manager().shaders().current_shader());
        math::Vector2D canvas_position = pos.Scale(canvas.size());
        canvas.PushAndCompose(math::Geometry(canvas_position));

        graphic::TextureUnit unit = graphic::manager().ReserveTextureUnit(texture);
        canvas.SendUniform("drawable_texture", unit);

        canvas.SendVertexData(vertex_data, graphic::VertexType::VERTEX, 0, 2);
        canvas.SendVertexData(vertex_data, graphic::VertexType::TEXTURE, 2 * sizeof(F32), 2);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        canvas.PopGeometry();
    });

    system::PushScene(std::move(scene));
    system::Run();
    system::Release();
    return 0;
}
