#pragma once

#include "jsonxx.h"

bool InitControlWindow(jsonxx::Object options);
void UpdateControlWindow(float ElapsedTime);
bool WantsToQuit();
void CloseControlWindow();