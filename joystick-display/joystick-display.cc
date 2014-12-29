
// Engine initialization
#include <ugdk/system/engine.h>
#include <ugdk/system/configuration.h>
#include <ugdk/system/compatibility.h>
#include <ugdk/action/scene.h>

#include <ugdk/input/events.h>
#include <ugdk/input/joystick.h>
#include <ugdk/action/events.h>

// Graphic
#include <ugdk/graphic/module.h>
#include <ugdk/ui/drawable/texturedrectangle.h>
#include <ugdk/ui/node.h>
#include <ugdk/text/module.h>
#include <ugdk/text/label.h>

#include <memory>
#include <functional>
#include <random>
#include <cstdio>

using namespace ugdk;

class JoystickDisplay;

namespace {
    const math::Vector2D box_size(50.0, 50.0);
    const math::Vector2D canvas_size(1280.0, 720.0);
    const double display_spacing = 5.0;
    text::Font* default_font = nullptr;

#if defined _MSC_VER && _MSC_VER <= 1800
    auto snprintf = sprintf_s;
#endif

    std::list<std::shared_ptr<JoystickDisplay>> active_joystick_listeners;

    std::shared_ptr<ui::Node> CreateLabelNode(const std::string& str, const math::Vector2D& position, ui::HookPoint hook = ui::HookPoint::TOP_LEFT) {
        auto node = std::make_shared<ui::Node>(MakeUnique<text::Label>(str, default_font));
        node->drawable()->set_hotspot(hook);
        node->geometry().set_offset(position);
        return node;
    }
}

class AxisSlider {
public:
    AxisSlider()
        : node_(new ui::Node)
    {
        auto background = std::make_shared<ui::Node>(MakeUnique<ui::TexturedRectangle>(graphic::manager()->white_texture(), math::Vector2D(width(), 10.0)));
        background->drawable()->set_hotspot(ui::HookPoint::CENTER);
        background->effect().set_color(Color(0.5, 0.5, 0.5));
        node_->AddChild(background);

        slider_ = std::make_shared<ui::Node>(MakeUnique<ui::TexturedRectangle>(graphic::manager()->white_texture(), math::Vector2D(5.0, 10.0)));
        slider_->drawable()->set_hotspot(ui::HookPoint::CENTER);
        slider_->effect().set_color(Color(0.0, 1.0, 0.0));
        node_->AddChild(slider_);
    }

    void SetPercentage(double percentage) {
        slider_->geometry().set_offset(math::Vector2D(percentage * width() / 2, 0.0));
    }

    double width() const { return 50.0; }

    std::shared_ptr<ui::Node> node() { return node_; }
private:
    std::shared_ptr<ui::Node> node_, slider_;
};

class ButtonDisplay {
public:
    ButtonDisplay()
        : node_(new ui::Node) {

        display_ = std::make_shared<ui::Node>(MakeUnique<ui::TexturedRectangle>(graphic::manager()->white_texture(), math::Vector2D(width(), height())));
        display_->drawable()->set_hotspot(ui::HookPoint::CENTER);
        display_->effect().set_color(Color(0.5, 0.5, 0.5));
        node_->AddChild(display_);
    }

    void set_active(bool active) {
        display_->effect().set_color(active ? Color(0.0, 0.5, 0.0)
                                            : Color(0.5, 0.5, 0.5));
    }

    double width() const { return 20.0; }
    double height() const { return 20.0; }

    std::shared_ptr<ui::Node> node() { return node_; }
private:
    std::shared_ptr<ui::Node> node_, display_;
};

class HatDisplay {
public:
    HatDisplay()
        : node_(new ui::Node) {
        auto background = std::make_shared<ui::Node>(MakeUnique<ui::TexturedRectangle>(graphic::manager()->white_texture(), math::Vector2D(width(), height())));
        background->drawable()->set_hotspot(ui::HookPoint::CENTER);
        background->effect().set_color(Color(0.5, 0.5, 0.5));
        node_->AddChild(background);

        slider_ = std::make_shared<ui::Node>(MakeUnique<ui::TexturedRectangle>(graphic::manager()->white_texture(), math::Vector2D(5.0, 5.0)));
        slider_->drawable()->set_hotspot(ui::HookPoint::CENTER);
        slider_->effect().set_color(Color(0.0, 1.0, 0.0));
        node_->AddChild(slider_);
    }

    void set_status(input::HatStatus status) {
        math::Vector2D offset;
        if (status.HasLeft()) offset.x -= width() / 2;
        if (status.HasRight()) offset.x += width() / 2;
        if (status.HasUp()) offset.y -= height() / 2;
        if (status.HasDown()) offset.y += height() / 2;
        slider_->geometry().set_offset(offset);
    }

    double width() const { return 20.0; }
    double height() const { return 20.0; }

    std::shared_ptr<ui::Node> node() { return node_; }
private:
    std::shared_ptr<ui::Node> node_, slider_;
};


class JoystickDisplay :
    public system::Listener<input::JoystickAxisEvent>,
    public system::Listener<input::JoystickButtonPressedEvent>,
    public system::Listener<input::JoystickButtonReleasedEvent>,
    public system::Listener<input::JoystickHatEvent>,
    public system::Listener<input::JoystickDisconnectedEvent>,
    public std::enable_shared_from_this<JoystickDisplay>
{
public:
    JoystickDisplay(std::shared_ptr<input::Joystick> joystick)
        : node_(new ui::Node)
        , joystick_(joystick)
    {
        joystick_->event_handler().AddObjectListener(this);

        auto background = std::make_shared<ui::Node>(MakeUnique<ui::TexturedRectangle>(graphic::manager()->white_texture(), math::Vector2D(width(), height())));
        background->effect().set_color(Color(0.1, 0.1, 0.1));
        node_->AddChild(background);

        char description[250];
        snprintf(description, 250, "Joystick [%p] -- %d Axis, %d Hat, %d Balls, %d Buttons",
                 joystick.get(),
                 joystick->NumAxes(), joystick->NumHats(), joystick->NumTrackballs(), joystick->NumButtons());
        node_->AddChild(CreateLabelNode(description, math::Vector2D(0.0, 0.0)));

        double xoffset = 0.0;

        if (joystick->NumAxes() > 0) {
            node_->AddChild(CreateLabelNode("Axis", math::Vector2D(xoffset + 5.0, 30.0)));
            axis_sliders_.resize(joystick->NumAxes());
            for (size_t i = 0; i < axis_sliders_.size(); ++i) {
                double width = axis_sliders_[i].width();

                axis_sliders_[i].node()->AddChild(CreateLabelNode(std::to_string(i), math::Vector2D(0.0, -10.0), ui::HookPoint::BOTTOM));
                axis_sliders_[i].node()->geometry().set_offset(math::Vector2D(xoffset + width / 2, 80));
                node_->AddChild(axis_sliders_[i].node());

                xoffset += width + 5.0;
            }
            xoffset += 10.0;
        }

        if (joystick->NumButtons() > 0) {
            node_->AddChild(CreateLabelNode("Buttons", math::Vector2D(xoffset + 5.0, 30.0)));
            button_displays_.resize(joystick->NumButtons());
            for (size_t i = 0; i < button_displays_.size(); ++i) {
                double width = button_displays_[i].width();

                button_displays_[i].node()->AddChild(CreateLabelNode(std::to_string(i), math::Vector2D(0.0, 0.0), ui::HookPoint::CENTER));
                button_displays_[i].node()->geometry().set_offset(math::Vector2D(xoffset + width / 2, 75));
                node_->AddChild(button_displays_[i].node());

                xoffset += width + 5.0;
            }
            xoffset += 10.0;
        }

        if (joystick->NumHats() > 0) {
            node_->AddChild(CreateLabelNode("Hats", math::Vector2D(xoffset + 5.0, 30.0)));
            hat_displays_.resize(joystick->NumHats());
            for (size_t i = 0; i < hat_displays_.size(); ++i) {
                double width = hat_displays_[i].width();

                hat_displays_[i].node()->AddChild(CreateLabelNode(std::to_string(i), math::Vector2D(0.0, -15.0), ui::HookPoint::BOTTOM));
                hat_displays_[i].node()->geometry().set_offset(math::Vector2D(xoffset + width / 2, 85));
                node_->AddChild(hat_displays_[i].node());

                xoffset += width + 5.0;
            }
            xoffset += 10.0;
        }
    }

    ~JoystickDisplay() {
        if (joystick_)
            joystick_->event_handler().RemoveObjectListener(this);
        if (node_->parent())
            node_->parent()->RemoveChild(node_.get());
    }

    void Handle(const input::JoystickAxisEvent& ev) override {
        axis_sliders_[ev.axis_id].SetPercentage(ev.axis_status.Percentage());
    }

    void Handle(const input::JoystickButtonPressedEvent& ev) override {
        button_displays_[ev.button].set_active(true);
    }

    void Handle(const input::JoystickButtonReleasedEvent& ev) override {
        button_displays_[ev.button].set_active(false);
    }

    void Handle(const input::JoystickHatEvent& ev) override {
        hat_displays_[ev.hat_id].set_status(ev.hat_status);
    }    

    void Handle(const input::JoystickDisconnectedEvent& ev) override {
        joystick_.reset();
        // Removing the object from the listeners list clears the only shared_ptr to
        // this, so the object is destroyed.
        active_joystick_listeners.remove(shared_from_this());
    }

    std::shared_ptr<ui::Node> node() { return node_; }
    double width() const { return 1000.0; }
    double height() const { return 100.0; }

private:
    std::shared_ptr<ui::Node> node_;
    math::Vector2D origin_;
    std::shared_ptr<input::Joystick> joystick_;
    std::vector<AxisSlider> axis_sliders_;
    std::vector<ButtonDisplay> button_displays_;
    std::vector<HatDisplay> hat_displays_;
};

namespace {
    void ClearJoystickListeners(const action::SceneFinishedEvent&) {
        // This list contains the only copies of the shared_ptr, so the objects are also
        // destroyed at this point.
        active_joystick_listeners.clear();
    }

    void RepositionDisplays() {
        double yoffset = 10.0;
        for (const auto& display : active_joystick_listeners) {
            display->node()->geometry().set_offset(math::Vector2D(10.0, yoffset));
            yoffset += display->height() + display_spacing;
        }
    }
}

int main(int argc, char *argv[]) {
    system::Configuration config;
    config.canvas_size = canvas_size;
    config.windows_list[0].size = canvas_size;
    // EXAMPLE_LOCATION is defined by CMake to be the full path to the directory
    // that contains the source code for this example
    config.base_path = EXAMPLE_LOCATION "/content/";
    system::Initialize(config);

    default_font = text::manager()->AddFont("default", "DejaVuSansMono.ttf", 16);

    auto scene = ugdk::MakeUnique<ugdk::action::Scene>();
    {
        // Create a node and use it as the render function of the scene.
        // Note that we purposedly bind the shared_ptr to the render function, so it's deleted along the scene.
        auto root_node = std::make_shared<ui::Node>();        
        scene->set_render_function(std::bind(&ui::Node::Render, root_node, std::placeholders::_1));

        // Create a weak reference to the root node so we don't delete it at the wrong time.
        std::weak_ptr<ui::Node> root_weak = root_node;

        // We don't check for the already connected joysticks at the moment because there's none:
        // Even joysticks "already connected" when our application starts goes through the joystick connection logic.
        scene->event_handler().AddListener<ugdk::input::JoystickConnectedEvent>([root_weak](const ugdk::input::JoystickConnectedEvent& ev) {
            // Create the logic object that will listen to joystick events.
            auto rect = std::make_shared<JoystickDisplay>(ev.joystick.lock());
            root_weak.lock()->AddChild(rect->node());
            active_joystick_listeners.push_back(rect);
            RepositionDisplays();
        });

        // Clean yourself:
        // Remove the objects listeners when the scene finishes.
        scene->event_handler().AddListener(ClearJoystickListeners);
    }
    system::PushScene(std::move(scene));

    system::Run();    
    system::Release();
    return 0;
}