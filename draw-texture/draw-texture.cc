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
    
    // Create scene
    auto &graphicman = graphic::manager();
    auto ourscene    = std::make_unique<action::Scene>();

    graphic::Mesh2D box(graphic::DrawMode::TRIANGLE_STRIP(), archimedes);
    
    box.Fill({
        {0, 0, 0, 0},
        {0, 100, 0, 1},
        {100, 0, 1, 0},
        {100, 100, 1, 1}});

    // And we access the RenderTarget's Renderer and add a step, which is a lambda
    graphicman.default_target().lock()->MyRenderer()->AddStep(
        [&box](graphic::Canvas& canvas) {
            canvas.Clear(ugdk::structure::Color(0.2, 0.2, 0.2, 1));
            canvas.ChangeShaderProgram(graphic::manager().shaders().current_shader());
            canvas << box;
    });

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