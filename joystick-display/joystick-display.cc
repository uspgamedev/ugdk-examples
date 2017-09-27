
// General Engine headers
#include <ugdk/system/engine.h>
#include <ugdk/system/configuration.h>
#include <ugdk/system/compatibility.h>
#include <ugdk/action/scene.h>

// Input headers
#include <ugdk/input/events.h>
#include <ugdk/input/joystick.h>
#include <ugdk/action/events.h>
#include <ugdk/input/module.h>

// Graphic headers
#include <ugdk/desktop/window.h>
#include <ugdk/desktop/module.h>
#include <ugdk/graphic/canvas.h>
#include <ugdk/graphic/drawmode.h>
#include <ugdk/graphic/module.h>
#include <ugdk/graphic/vertexdata.h>

// Text headers
#include <ugdk/text/module.h>
#include <ugdk/text/label.h>

// Type headers
#include <ugdk/structure/color.h>
#include <ugdk/structure/types.h>

#include <memory>
#include <iostream>
#include <tuple>

namespace {

using namespace ugdk;
using math::Vector2D;
using std::shared_ptr;

const Vector2D  BOX_SIZE(50.0, 50.0);
const double    SPEED = 500.0;

struct VertexXYUV {
    F32 x, y, u, v;
};

struct Box {
    Vector2D pos;
    structure::Color color;
    graphic::GLTexture *tex;
    graphic::VertexData vtx;
};

graphic::Canvas& operator<<(graphic::Canvas& canvas, const Box& box) {
    auto &pos = box.pos;
    auto &texture = box.tex;
    auto &vertex_data = box.vtx;
    auto &color = box.color;

    canvas.PushAndCompose(math::Geometry(pos - BOX_SIZE/2));
    canvas.PushAndCompose(color);
    
    graphic::TextureUnit unit = graphic::manager().ReserveTextureUnit(texture);
    canvas.SendUniform("drawable_texture", unit);

    canvas.SendVertexData(vertex_data, graphic::VertexType::VERTEX, 0, 2);
    canvas.SendVertexData(vertex_data, graphic::VertexType::TEXTURE, 2 * sizeof(F32), 2);
    canvas.DrawArrays(graphic::DrawMode::TRIANGLE_STRIP(), 0, 4);

    canvas.PopVisualEffect();
    canvas.PopGeometry();
    
    return canvas;
}

struct JoystickDisplay {
    shared_ptr<input::Joystick> joystick;
    shared_ptr<text::Label> name;
};

graphic::Canvas& operator<<(graphic::Canvas& canvas, const JoystickDisplay& joy) {

    canvas << *joy.name;

    return canvas;
}

std::vector<JoystickDisplay> displays;

} // unnamed namespace

int main() {
    // UGDK initialization
    // EXAMPLE_LOCATION is defined by CMake to be the full path to the directory
    // that contains the source code for this example */
    system::Configuration config;
    config.base_path = EXAMPLE_LOCATION "/content/";
    config.canvas_size = Vector2D(1280, 720);
    config.windows_list.front().size = math::Integer2D(1280, 720);
    system::Initialize(config);

    // Load font
    text::manager().AddFont("default", "DejaVuSansMono.ttf", 18);

    // Create scene
    auto scene = std::make_unique<ugdk::action::Scene>();

    // Exit event
    system::FunctionListener<input::KeyPressedEvent> exit_listener(
        [] (const ugdk::input::KeyPressedEvent& ev) {
            if (ev.scancode == ugdk::input::Scancode::ESCAPE)
                ugdk::system::CurrentScene().Finish();
        }
    );
    scene->event_handler().AddListener(exit_listener);

    // Register joystick
    // We don't check for the already connected joysticks at the moment because there's none:
    // Even joysticks "already connected" when our application starts goes through the joystick
    // connection logic.
    // Also, it connects ALL joysticks your OS recognizes. This MIGHT include your keyboard.
    system::FunctionListener<input::JoystickConnectedEvent> joystick_connection_listener(
        [](const auto& ev) {
            shared_ptr<input::Joystick> joystick = ev.joystick.lock();
            JoystickDisplay display = {
                joystick,
                std::make_shared<text::Label>(joystick->name(),
                                              text::manager().GetFont("default"))
            };
            displays.push_back(display);
            std::cout << "registered joystick " << joystick->name() << std::endl;
        }
    );
    scene->event_handler().AddListener(joystick_connection_listener);

    // Update displays
    scene->AddTask([](double dt) {
        // TODO
    });

    scene->set_render_function([](graphic::Canvas& canvas) {
        using namespace graphic;
        const Vector2D base(40.0, 40.0);

        canvas.Clear(ugdk::structure::Color(0.2, 0.2, 0.2, 1));
        canvas.ChangeShaderProgram(graphic::manager().shaders().current_shader());

        for (size_t i = 0; i < displays.size(); i++) {
            const JoystickDisplay &display = displays[i];
            canvas.PushAndCompose(base + i * Vector2D(0.0, 80.0));
            canvas << display;
            canvas.PopGeometry();
        }
    });
    
    system::PushScene(std::move(scene));

    system::Run();
    system::Release();
    return 0;
}
