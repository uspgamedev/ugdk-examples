#include <ugdk/system/engine.h>
#include <ugdk/action/scene.h>
#include <ugdk/input/events.h>
#include <ugdk/system/compatibility.h>

void QuitOnEscape(const ugdk::input::KeyPressedEvent& ev) {
    if (ev.scancode == ugdk::input::Scancode::ESCAPE)
        ugdk::system::CurrentScene().Finish();
}

int main(int argc, char* argv[]) {
    ugdk::system::Initialize();

    auto ourscene = ugdk::MakeUnique<ugdk::action::Scene>();
    ourscene->event_handler().AddListener(QuitOnEscape);
    ugdk::system::PushScene(std::move(ourscene));

    ugdk::system::Run();
    ugdk::system::Release();
    return 0;
}
