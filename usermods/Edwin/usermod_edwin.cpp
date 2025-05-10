#include "usermod_edwin.h"

#include "img_BIOS_1.inc"
#include "img_BIOS_2.inc"
#include "img_BIOS_3.inc"
#include "img_OSBoot.inc"
#include "img_Taskbar.inc"
#include "img_GreenHill.inc"
#include "img_Clouds.inc"
#include "img_BBCNews.inc"
#include "img_BSOD.inc"

#include "StateMachine.h"
#include "SimpleImageState.h"
#include "BootStates.h"
#include "BSODState.h"
#include "DesktopState.h"
#include "MatrixScreensaverState.h"
#include "PipesScreensaverState.h"
#include "StarfieldScreensaverState.h"
#include "SolitareState.h"

const Image* img_Wallpaper = &img_GreenHill;

EdwinUsermod* EdwinUsermod::_instance = nullptr;

void EdwinUsermod::setup() {
    _instance = this;

    WS2812FX* fx = WS2812FX::getInstance();
    fx->addEffect(255, &EdwinUsermod::staticEffect, "Edwin");

    _sm = new StateMachine();

    const int MaxActiveMillis = 5000;

    auto& bios = _sm->AddState<BIOSState>();
    auto& osBoot = _sm->AddState<OSBootState>(img_OSBoot, MaxActiveMillis);
    auto& desktop = _sm->AddState<DesktopState>();
    auto& browser = _sm->AddState<SimpleImageState>(img_BBCNews, MaxActiveMillis);
    auto& bsod = _sm->AddState<BSODState>(img_BSOD, MaxActiveMillis);
    auto& matrix = _sm->AddState<MatrixScreensaverState>();
    auto& pipes = _sm->AddState<PipesScreensaverState>();
    auto& starfield = _sm->AddState<StarfieldScreensaverState>();
    auto& solitare = _sm->AddState<SolitareState>();

    bios.AddTransition(osBoot);
    osBoot.AddTransition(desktop);
    desktop.AddTransitions(/*browser,*/ matrix, pipes, starfield, solitare, bsod);
    //browser.AddTransitions(desktop);
    bsod.AddTransition(bios);
    matrix.AddTransition(desktop);
    pipes.AddTransition(desktop);
    starfield.AddTransition(desktop);
    solitare.AddTransition(desktop);
    
    _sm->SetNextState(bios);
    //_sm->SetNextState(matrix);
}
    
uint16_t EdwinUsermod::effect() {
    _sm->OnEffect();
    return FRAMETIME;
}

uint16_t EdwinUsermod::staticEffect() {
    return _instance->effect();
}

