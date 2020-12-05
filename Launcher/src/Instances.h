#pragma once

#include <windows.h>
#include <vector>
#include <string>
#include "jsonxx.h"

class Instance {

public:
  HWND hwnd = NULL;
  STARTUPINFOA siStartupInfo;
  PROCESS_INFORMATION piProcessInfo;

  int Index = 0;
  std::string CoderName;
  bool IsFullScreen = false;
  bool IsHidden = false;

  Instance();
  bool Init(int InstanceIndex, std::string CoderName);
  void Release();
  void Restart();
};

bool LaunchInstances(jsonxx::Object options);
void ReleaseInstances();
void FindDesktopResolution();

std::vector<class Instance*>& GetInstances();

enum class DisplayAction {
  FirstDisplay,
  ShowMosaic,
  SetFullscreen
};
void ChangeDisplay(DisplayAction Action, Instance* Target=nullptr);
void ToggleHidden(Instance* Target);

void FullscreenPrev();
void FullscreenNext();
void ToggleFullscreen(int index);
void ToggleFullscreen(Instance* Cur);
void ToggleTextEditor();

void StartDiaporama();
void StopDiaporama();
void UpdateDiaporama(float ElapsedTime);
bool IsDiapoLaunched();
int GetCurrentDiapo();

