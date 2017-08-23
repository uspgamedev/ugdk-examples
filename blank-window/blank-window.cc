
#include <ugdk/system/engine.h>
#include <ugdk/action/scene.h>
#include <ugdk/input/events.h>
#include <ugdk/system/compatibility.h>

#include <memory>

void QuitOnEscape(const ugdk::input::KeyPressedEvent& ev) {
    if (ev.scancode == ugdk::input::Scancode::ESCAPE)
        ugdk::system::CurrentScene().Finish();
}

int main(int argc, char* argv[]) {
    using namespace ugdk;
    system::Initialize();
    system::FunctionListener<input::KeyPressedEvent> listener(QuitOnEscape);

    auto ourscene = std::make_unique<action::Scene>();

    ourscene->event_handler().AddListener(listener);

    system::PushScene(std::move(ourscene));

    system::Run();
    system::Release();
    return 0;
}
