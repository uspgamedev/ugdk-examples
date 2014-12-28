
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

#include <memory>
#include <functional>
#include <random>

using namespace ugdk;

class MovableRect;

namespace {
    const math::Vector2D box_size(50.0, 50.0);
    const math::Vector2D canvas_size(800.0, 600.0);

    std::random_device rd;
    std::default_random_engine e1(rd());
    std::uniform_real_distribution<double> width_dist( canvas_size.x * 0.2, canvas_size.x * 0.8);
    std::uniform_real_distribution<double> height_dist(canvas_size.y * 0.2, canvas_size.y * 0.8);

    std::list<std::shared_ptr < MovableRect >> active_joystick_listeners;
}

class MovableRect :
    public system::Listener<input::JoystickAxisEvent>,
    public system::Listener<input::JoystickDisconnectedEvent>,
    public std::enable_shared_from_this<MovableRect>
{
public:
    MovableRect(std::shared_ptr<ui::Node> n, math::Vector2D origin)
        : node_(n)
        , origin_(origin)
    {}
    ~MovableRect() {
        if (node_->parent())
            node_->parent()->RemoveChild(node_.get());
    }

    void Register(std::shared_ptr<input::Joystick> joystick) {
        joystick->event_handler().AddObjectListener(this);
        connected_joystick_ = joystick;
    }

    void Deregister() {
        connected_joystick_.lock()->event_handler().RemoveObjectListener(this);
        connected_joystick_.reset();
    }

    void Handle(const input::JoystickAxisEvent& ev) override {
        math::Vector2D current_offset = node_->geometry().offset();
        if (ev.axis_id == 0)
            current_offset.x = origin_.x + 100 * ev.axis_status.Percentage();
        else if (ev.axis_id == 1)
            current_offset.y = origin_.y + 100 * ev.axis_status.Percentage();
        node_->geometry().set_offset(current_offset);
    }

    void Handle(const input::JoystickDisconnectedEvent& ev) override {
        // We don't have to worry about removing ourselves from the joystick event handler
        // because the joystick is going to be destroyed after a disconnection event.

        // Removing the object from the listeners list clears the only shared_ptr to
        // this, so the object is destroyed.
        active_joystick_listeners.remove(shared_from_this());
    }

private:
    std::shared_ptr<ui::Node> node_;
    math::Vector2D origin_;
    std::weak_ptr<input::Joystick> connected_joystick_;
};

namespace {
    void ClearJoystickListeners(const action::SceneFinishedEvent&) {
        for (const auto& listener : active_joystick_listeners)
            listener->Deregister();

        // This list contains the only copies of the shared_ptr, so the objects are also
        // destroyed at this point.
        active_joystick_listeners.clear();
    }
}

int main(int argc, char *argv[]) {
    system::Configuration config;
    config.canvas_size = canvas_size;
    system::Initialize(config);

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

            // Sets the point the rect will rotate around.
            math::Vector2D origin(width_dist(e1), height_dist(e1));

            // Create the visual.
            auto joystick_node = std::make_shared<ui::Node>(MakeUnique<ui::TexturedRectangle>(graphic::manager()->white_texture(), box_size));
            joystick_node->drawable()->set_hotspot(ui::HookPoint::CENTER);
            root_weak.lock()->AddChild(joystick_node);
            joystick_node->geometry().set_offset(origin);

            // Create the logic object that will listen to joystick events.
            auto rect = std::make_shared<MovableRect>(joystick_node, origin);
            rect->Register(ev.joystick.lock());
            active_joystick_listeners.push_back(rect);
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