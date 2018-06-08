#include <ugdk/system/engine.h>
#include <ugdk/action/scene.h>
#include <ugdk/input/events.h>
#include <ugdk/desktop/window.h>
#include <ugdk/desktop/module.h>
#include <ugdk/graphic/canvas.h>
#include <ugdk/graphic/drawmode.h>
#include <ugdk/graphic/module.h>
#include <ugdk/graphic/mesh.h>
#include <ugdk/graphic/rendertarget.h>
#include <ugdk/graphic/rendertexture.h>
#include <ugdk/structure/color.h>
#include <ugdk/structure/types.h>
#include <ugdk/system/compatibility.h>
#include <ugdk/filesystem/module.h>
#include <ugdk/filesystem/file.h>
#include <ugdk/resource/manager.h>
#include <ugdk/resource/module.h>

#include <glm/vec2.hpp>

#include <memory>
#include <vector>
#include <iostream>
#include <tuple>

using namespace ugdk;
using namespace glm;

int main(int argc, char*argv[]) {
    // UGDK initialization
    system::Configuration config;

    // EXAMPLE_LOCATION is defined by CMake to be the full path to the directory
    // that contains the source code for this example
    config.base_path = EXAMPLE_LOCATION "/content/";
    config.windows_list.front().canvas_size = dvec2(1280, 720);
    config.windows_list.front().size        = math::Integer2D(1280, 720);
    system::Initialize(config);
    
    auto &desktopman  = desktop::manager();
    auto &resourceman = resource::manager();
    auto archimedes   = resourceman.GetContainer<graphic::GLTexture>()->Load("arquimedes.png", "img logo");
    auto &graphicman = graphic::manager();
    auto ourscene    = std::make_unique<action::Scene>();

    auto screen_texture = graphicman.RegisterTexture(math::Integer2D(1280, 720)).lock();

    constexpr float len = 100;

    graphic::Mesh2D box(graphic::DrawMode::TRIANGLE_STRIP(), archimedes);    
    dvec2 box_position;
    box.Fill({
        {0  , 0  , 0, 0},
        {0  , len, 0, 1},
        {len, 0  , 1, 0},
        {len, len, 1, 1}});

    screen_texture->MyRenderer()->AddStep(
        [&box, &box_position](graphic::Canvas& canvas) {
            canvas.ChangeShaderProgram(graphic::manager().shaders().current_shader());

            //canvas.Clear(ugdk::structure::Color(0 , 0, 1, 1));
            ///1280*2 -1
            ///720 *2 -1 -dvec2(300/2,300/2)
            canvas.PushAndCompose(math::Geometry(math::Vector2D(-1,-1), math::Vector2D(1.0/1280*2, 1.0/720*2)));            
            canvas.PushAndCompose(math::Geometry(box_position - dvec2(len/2,len/2), math::Vector2D(1, 1),0));
                canvas << box;
            canvas.PopGeometry();
            canvas.PopGeometry();
    });

    graphicman.default_target().lock()->MyRenderer()->AddStep(
        [screen_texture](graphic::Canvas& canvas) {
            canvas.ChangeShaderProgram(graphic::manager().shaders().current_shader());
            
            canvas.Clear(ugdk::structure::Color(0.2, 0.2, 0.2, 1));

            graphic::Mesh2D screen_box(graphic::DrawMode::TRIANGLE_STRIP(), screen_texture->texture());
            screen_box.Fill({
                {0   , 0  , 0, 0},
                {0   , 720, 0, 1},
                {1280, 0  , 1, 0},
                {1280, 720, 1, 1}});
                canvas << screen_box;
    });

    // Box drag event
    system::FunctionListener<input::MouseMotionEvent> box_listener(
        [&box_position](const input::MouseMotionEvent& ev) {
            box_position.x = static_cast<double>(ev.position.x);
            box_position.y = static_cast<double>(ev.position.y);
        }
    );
    ourscene->event_handler().AddListener<input::MouseMotionEvent>(box_listener);

    // And we access the RenderTarget's Renderer and add a step, which is a lambda
    

    // This is a listener for the ESC key press event, to quit out.
    system::FunctionListener<input::KeyPressedEvent> esc_listener(
        [] (const ugdk::input::KeyPressedEvent& ev) {
            if (ev.scancode == ugdk::input::Scancode::ESCAPE)
                ugdk::system::CurrentScene().Finish();
        }
    );
    ourscene->event_handler().AddListener(esc_listener); 

    system::PushScene(std::move(ourscene));

    system::Run();
    system::Release();
    return 0;
}