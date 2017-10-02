
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
#include <vector>

namespace {

using namespace ugdk;
using math::Vector2D;
using structure::Color;
using std::shared_ptr;
using std::vector;

const double    SPEED = 500.0;

struct JoystickDisplay;

std::vector<JoystickDisplay> displays;

struct VertexXYUV {
    F32 x, y, u, v;
};

struct Box {
    Vector2D size;
    Color color;
    graphic::GLTexture *tex;
    graphic::VertexData vtx;
};

Box makeBox(const Vector2D& size, const Color& color) {
    // Box data
    Box box = {
        size,
        color,
        graphic::manager().white_texture(),
        std::move(graphic::VertexData(4u, sizeof(VertexXYUV), false))
    };
    // Map box vertices
    {
        graphic::VertexData::Mapper mapper(box.vtx, false);
        mapper.Get<VertexXYUV>(0u) = {.0f, .0f, .0f, .0f};
        mapper.Get<VertexXYUV>(1u) = {.0f, 1.f, .0f, 1.f};
        mapper.Get<VertexXYUV>(2u) = {1.f, .0f, 1.f, .0f};
        mapper.Get<VertexXYUV>(3u) = {1.f, 1.f, 1.f, 1.f};
    }
    return std::move(box);
}

graphic::Canvas& operator<<(graphic::Canvas& canvas, const Box& box) {
    auto &size = box.size;
    auto &texture = box.tex;
    auto &vertex_data = box.vtx;
    auto &color = box.color;

    canvas.PushAndCompose(math::Geometry(Vector2D(), size));
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

struct Slider {
    Box bg;
    Box slider;
    shared_ptr<text::Label> label;
    double value;
};

struct Button {
    Box bg;
    shared_ptr<text::Label> label;
};

struct JoystickDisplay {
    shared_ptr<input::Joystick> joystick;
    shared_ptr<text::Label> name;
    Box bg;
    shared_ptr<text::Label> axes_label;
    vector<Slider> axis_sliders;
    shared_ptr<text::Label> buttons_label;
    vector<Button> buttons;
};

void addJoystickDisplay(const shared_ptr<input::Joystick>& joystick) {
    auto *font = text::manager().GetFont("default");
    JoystickDisplay joy = {
        joystick,
        std::make_shared<text::Label>(joystick->name(),
                                      font),
        makeBox(Vector2D(1000.0, 120.0), Color(0.1,0.1,0.1)),
        nullptr,
        vector<Slider>(),
        nullptr,
        vector<Button>()
    };
    // Add axis sliders
    if (joystick->NumAxes() > 0) {
        joy.axes_label = std::make_shared<text::Label>("Axes", font);
        for (size_t i = 0; i < joystick->NumAxes(); ++i) {
            Slider slider = {
                makeBox(Vector2D(50.0, 10.0), Color(0.5, 0.5, 0.5)),
                makeBox(Vector2D(5.0, 10.0), Color(0.0, 1.0, 0.0)),
                std::make_shared<text::Label>(std::to_string(i),
                                              font),
                0.0
            };
            joy.axis_sliders.emplace_back(std::move(slider));
        }
    }
    // Add buttons things
    if (joystick->NumButtons() > 0) {
        joy.buttons_label = std::make_shared<text::Label>("Buttons", 
                                                          font);
        for (size_t i = 0u; i < joystick->NumButtons(); i++) {
            Button button = {
                makeBox(Vector2D(20.0, 20.0), Color(0.5, 0.5, 0.5)),
                std::make_shared<text::Label>(std::to_string(i), font)
            };
            joy.buttons.emplace_back(std::move(button));
        }
    }

    displays.emplace_back(std::move(joy));
}

graphic::Canvas& operator<<(graphic::Canvas& canvas, const JoystickDisplay& joy) {

    canvas << joy.bg;
    canvas.PushAndCompose(math::Geometry(Vector2D(5., 0.)));
    canvas << *joy.name;

    double x_offset = 0.0;
    if (joy.axes_label) {
        canvas.PushAndCompose(Vector2D(5.0, 30.0));
        canvas << *joy.axes_label;
        for (const auto& slider : joy.axis_sliders) {
            double width = slider.bg.size.x;
            double small_width = slider.slider.size.x;
            canvas.PushAndCompose(Vector2D(x_offset, 60));
            canvas << slider.bg;
            canvas.PushAndCompose(Vector2D(((1.0 + slider.value)*width - small_width)/2.0, 0));
            canvas << slider.slider;
            canvas.PopGeometry();
            canvas.PushAndCompose(Vector2D(width/2.0 - slider.label->width()/2.0, -30.0));
            canvas << *slider.label;
            canvas.PopGeometry();
            canvas.PopGeometry();
            x_offset += width + 20.0;
        }
        canvas.PopGeometry();
        x_offset += 10.0;
    }
    if (joy.buttons_label) {
        canvas.PushAndCompose(Vector2D(x_offset + 5.0, 30));
        canvas << *joy.buttons_label;
        canvas.PopGeometry();
        for (const auto& button : joy.buttons) {
            canvas.PushAndCompose(Vector2D(x_offset, 90.0));
            double width = button.bg.size.x;
            double small_width = button.label->width();
            canvas << button.bg;
            canvas.PushAndCompose(Vector2D(width / 2.0 - small_width / 2.0, -30.0));
            canvas << *button.label;
            canvas.PopGeometry();
            canvas.PopGeometry();
            x_offset += width + 10;
        }
        x_offset += 10.0;
    }

    canvas.PopGeometry();
    
    return canvas;
}

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
            addJoystickDisplay(ev.joystick.lock());
        }
    );
    scene->event_handler().AddListener(joystick_connection_listener);

    // Update displays
    scene->AddTask([](double dt) {
        for (auto& joy : displays) {
            for (size_t i = 0u; i < joy.joystick->NumAxes(); i++) {
                joy.axis_sliders[i].value = joy.joystick->GetAxisStatus(i).Percentage();
            }
            for (size_t i = 0u; i< joy.joystick->NumButtons(); i++)
                if (joy.joystick->IsDown(i))
                    joy.buttons[i].bg.color = Color(0.0, 1.0, 0.0);
                else 
                    joy.buttons[i].bg.color = Color(0.5, 0.5, 0.5);
        }
    });

    scene->set_render_function([](graphic::Canvas& canvas) {
        using namespace graphic;
        const Vector2D base(10.0, 10.0);

        canvas.Clear(ugdk::structure::Color(0.2, 0.2, 0.2, 1));
        canvas.ChangeShaderProgram(graphic::manager().shaders().current_shader());

        for (size_t i = 0; i < displays.size(); i++) {
            const JoystickDisplay &display = displays[i];
            canvas.PushAndCompose(base + i * Vector2D(0.0, display.bg.size.y + 15.0));
            canvas << display;
            canvas.PopGeometry();
        }
    });
    
    system::PushScene(std::move(scene));

    system::Run();
    system::Release();
    return 0;
}
