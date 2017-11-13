
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
#include <ugdk/graphic/mesh.h>

// Text headers
#include <ugdk/text/module.h>
#include <ugdk/text/label.h>

// Type headers
#include <ugdk/structure/color.h>
#include <ugdk/structure/types.h>
#include <ugdk/structure/vertex.h>

#include <memory>
#include <iostream>
#include <tuple>
#include <vector>

namespace {

using namespace ugdk;
using namespace glm;
using structure::Color;
using std::shared_ptr;
using std::vector;

const double    SPEED = 500.0;

struct JoystickDisplay;

std::vector<JoystickDisplay> displays;
graphic::Mesh2D mesh;

struct Box {
    dvec2 size;
    Color color;
    graphic::Mesh2D mesh;
};

struct Slider {
    Box bg;
    Box slider;
    shared_ptr<text::Label> label;
};

struct Button {
    Box bg;
    shared_ptr<text::Label> label;
};

struct Hat {
    Box bg;
    Box slider;
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
    shared_ptr<text::Label> hats_label;
    vector<Hat> hats;
};

Box makeBox(const dvec2& size, const Color& color) {
    // Box data
    Box box = {
        size,
        color,
        mesh
    };
    return std::move(box);
}

graphic::Canvas& operator<<(graphic::Canvas& canvas, const Box& box) {
    auto &size = box.size;
    auto &mesh = box.mesh;
    auto &color = box.color;

    canvas.PushAndCompose(math::Geometry(dvec2(), size));
    canvas.PushAndCompose(color);

    canvas << mesh;

    canvas.PopVisualEffect();
    canvas.PopGeometry();
    
    return canvas;
}

void addJoystickDisplay(const shared_ptr<input::Joystick>& joystick) {
    auto *font = text::manager().GetFont("default");
    JoystickDisplay joy = {
        joystick,
        std::make_shared<text::Label>(joystick->name(),
                                      font),
        makeBox(dvec2(1000.0, 130.0), Color(0.1,0.1,0.1)),
        nullptr,
        vector<Slider>(),
        nullptr,
        vector<Button>(),
        nullptr,
        vector<Hat>()
    };
    // Add axis sliders
    if (joystick->NumAxes() > 0) {
        joy.axes_label = std::make_shared<text::Label>("Axes", font);
        for (size_t i = 0; i < joystick->NumAxes(); ++i) {
            Slider slider = {
                makeBox(dvec2(50.0, 10.0), Color(0.5, 0.5, 0.5)),
                makeBox(dvec2(5.0, 10.0), Color(0.0, 1.0, 0.0)),
                std::make_shared<text::Label>(std::to_string(i),
                                              font)
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
                makeBox(dvec2(20.0, 20.0), Color(1.0, 1.0, 1.0)),
                std::make_shared<text::Label>(std::to_string(i), font)
            };
            joy.buttons.emplace_back(std::move(button));
        }
    }
    // Add hat slider
    if (joystick->NumHats() > 0) {
        joy.hats_label = std::make_shared<text::Label> ("Hats", font);
        for (size_t i = 0u; i < joystick->NumHats(); i++) {
            Hat hat = {
                makeBox(dvec2(30.0, 30.0), Color(0.5, 0.5, 0.5)),
                makeBox(dvec2(10.0, 10.0), Color(0.0, 1.0, 0.0)),
                std::make_shared<text::Label>(std::to_string(i), font)
            };
            joy.hats.emplace_back(std::move(hat));
        }
    }

    displays.emplace_back(std::move(joy));
}

graphic::Canvas& operator<<(graphic::Canvas& canvas, const JoystickDisplay& joy) {

    canvas << joy.bg;
    canvas.PushAndCompose(math::Geometry(dvec2(5., 0.)));
    canvas << *joy.name;

    double x_offset = 0.0;
    if (joy.axes_label) {
        canvas.PushAndCompose(dvec2(5.0, 30.0));
        canvas << *joy.axes_label;
        for (size_t i = 0u; i < joy.axis_sliders.size(); i++) {
            const auto& slider = joy.axis_sliders[i];
            double width = slider.bg.size.x;
            double small_width = slider.slider.size.x;
            double value = joy.joystick->GetAxisStatus(i).Percentage();
            canvas.PushAndCompose(dvec2(x_offset, 60));
            canvas << slider.bg;
            canvas.PushAndCompose(dvec2(((1.0 + value)*width - small_width)/2.0, 0));
            canvas << slider.slider;
            canvas.PopGeometry();
            canvas.PushAndCompose(dvec2(width/2.0 - slider.label->width()/2.0, -30.0));
            canvas << *slider.label;
            canvas.PopGeometry();
            canvas.PopGeometry();
            x_offset += width + 20.0;
        }
        canvas.PopGeometry();
        x_offset += 10.0;
    }
    if (joy.buttons_label) {
        canvas.PushAndCompose(dvec2(x_offset + 5.0, 30));
        canvas << *joy.buttons_label;
        canvas.PopGeometry();
        for (size_t i = 0u; i < joy.buttons.size(); i++) {
            const auto& button = joy.buttons[i];
            double width = button.bg.size.x;
            double small_width = button.label->width();
            Color color;
            if (joy.joystick->IsDown(i))
                color = Color(0.0, 1.0, 0.0);
            else 
                color = Color(0.5, 0.5, 0.5);
            canvas.PushAndCompose(dvec2(x_offset, 90.0));
            canvas.PushAndCompose(color);
            canvas << button.bg;
            canvas.PopVisualEffect();
            canvas.PushAndCompose(dvec2(width / 2.0 - small_width / 2.0, -30.0));
            canvas << *button.label;
            canvas.PopGeometry();
            canvas.PopGeometry();
            x_offset += width + 10;
        }
        x_offset += 10.0;
    }
    if (joy.hats_label) {
        canvas.PushAndCompose(dvec2(x_offset + 5.0, 30.0));
        canvas << *joy.hats_label;
        canvas.PopGeometry();
        for (size_t i = 0u; i < joy.hats.size(); i++) {
            const auto& hat = joy.hats[i];
            auto status = joy.joystick->GetHatStatus(i);
            double side = hat.bg.size.x;
            double small_side = hat.slider.size.x;
            dvec2 offset;
            if (status.HasLeft()) offset.x -= side / 3;
            if (status.HasRight()) offset.x += side / 3;
            if (status.HasUp()) offset.y -= side / 3;
            if (status.HasDown()) offset.y += side / 3;
            canvas.PushAndCompose(dvec2(x_offset, 90.0));
            canvas << hat.bg;
            canvas.PushAndCompose(dvec2(1.0, 1.0)*(side - small_side) * 0.5 + offset);
            canvas << hat.slider;
            canvas.PopGeometry();
            canvas.PushAndCompose(dvec2(side/2.0 - hat.label->width() * 0.5, -30.0));
            canvas << *hat.label;
            canvas.PopGeometry();
            canvas.PopGeometry();
            x_offset += side + 20.0;
            
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
    config.windows_list.front().canvas_size = dvec2(1280, 720);
    config.windows_list.front().size        = math::Integer2D(1280, 720);
    system::Initialize(config);

    // Load font
    text::manager().AddFont("default", "DejaVuSansMono.ttf", 18);

    // Mesh data
    mesh = graphic::Mesh2D(graphic::DrawMode::TRIANGLE_STRIP(), graphic::manager().white_texture());
    mesh.Fill({
        {.0f, .0f, .0f, .0f},
        {.0f, 1.f, .0f, 1.f},
        {.0f, .0f, 1.f, 0.f},
        {.0f, 1.f, 1.f, 1.f}
    });

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

    scene->set_render_function(0u,
        [](graphic::Canvas& canvas) {        
            using namespace graphic;
            const dvec2 base(10.0, 10.0);

            canvas.Clear(ugdk::structure::Color(0.2, 0.2, 0.2, 1));
            canvas.ChangeShaderProgram(graphic::manager().shaders().current_shader());

            for (size_t i = 0; i < displays.size(); i++) {
                const JoystickDisplay &display = displays[i];
                canvas.PushAndCompose(base + dvec2(0.0, display.bg.size.y + 15.0) * static_cast<F64>(i));
                canvas << display;
                canvas.PopGeometry();
            }
        }
    );
    
    system::PushScene(std::move(scene));

    system::Run();
    system::Release();
    return 0;
}
