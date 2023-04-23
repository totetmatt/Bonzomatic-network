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

  std::string CoderName;
  bool Launched;
  bool IsFullScreen = false;
  bool IsHidden = false;
  bool FirstDisplay = true;
  float TimeLastLive = 10000.0f;

  Instance();
  bool Init(std::string CoderName);
  bool InitBonzo();
  void Release();
  void Restart();

  bool IsShowMosaic() { return Launched && !IsHidden; }
};

Instance* AddInstance(std::string CoderName, bool FromNetwork);
void RemoveInstance(Instance* instance);
bool LaunchInstances(jsonxx::Object options);
void ReleaseInstances();
void SortInstances();
void FindDesktopResolution();

std::vector<class Instance*>& GetInstances();

void RefreshDisplay();
void ShowMosaic();
void ToggleHidden(Instance* Target);

void FullscreenPrev();
void FullscreenNext();
void ToggleFullscreen(int index);
void ToggleFullscreen(Instance* Cur);
void RandomFullscreen();
void ToggleTextEditor();

void StartDiaporama();
void StopDiaporama();
void TickInstances(float ElapsedTime);
void UpdateDiaporama(float ElapsedTime);
bool IsDiapoLaunched();

void SignalLiveUser(std::string UserName);
void ToggleNetwork();
