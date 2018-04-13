
#include <ugdk/system/engine.h>
#include <ugdk/action/scene.h>
#include <ugdk/input/events.h>
#include <ugdk/system/compatibility.h>
#include <ugdk/graphic/canvas.h>
#include <ugdk/graphic/module.h>
#include <ugdk/graphic/opengl.h>
#include <ugdk/desktop/module.h>
#include <ugdk/desktop/window.h>
#include <ugdk/math/integer2D.h>
#include <ugdk/audio/module.h>
#include <ugdk/audio/sampleframe.h>
#include <ugdk/audio/manager.h>
#include <ugdk/audio/source.h>
#include <ugdk/audio/sampler.h>
#include <ugdk/structure/types.h>

#include <vector>
#include <iostream>
#include <memory>
#include <cmath>

using namespace ugdk;
using namespace audio;

/**
  * + [ ] Multichannel SampleFrame
  * + [ ] Multichannel SampleData
  * + [ ] SpatialTrack
  * + [ ] Sampler<params...>(in_tracks, out_tracks, processor)
  * + [ ] StereoTrack
  * + [ ] "OGG" Sampler
  * + [ ] "WAV" Sampler
  */

int main(int argc, char* argv[]) {
    system::Configuration config;
    config.windows_list.front().canvas_size = math::Vector2D(1280, 720);
    config.windows_list.front().size        = math::Integer2D(1280, 720);
    system::Initialize(config);

    system::FunctionListener<input::KeyPressedEvent> listener(
        [] (const ugdk::input::KeyPressedEvent& ev) {
            if (ev.scancode == ugdk::input::Scancode::ESCAPE)
                ugdk::system::CurrentScene().Finish();
        }
    );

    auto ourscene = std::make_unique<action::Scene>();

    ourscene->event_handler().AddListener(listener);
    ourscene->set_render_function(0u,
        [](graphic::Canvas& canvas)
            {canvas.Clear(ugdk::structure::Color(0.2, 0.2, 0.2, 1));}
    );

    auto sine_func = [](audio::SampleFrame frame) -> F64 {
        F64 t = frame.time();
        F64 d = 3.14159 * 2;
        return sin(0.5 * t * d) * sin(440.0 * t * d);
    };

    std::shared_ptr<Source>  source  = audio::manager().LoadSource("source1");
    std::shared_ptr<Sampler> sampler = audio::manager().LoadSampler("sine", 800000,
                                                                    true, true,
                                                                    44100, sine_func);
    source->set_volume(0.2);
    source->set_sampler(sampler);
    source->Play();

    system::PushScene(std::move(ourscene));

    system::Run();
    system::Release();
    return 0;
}
