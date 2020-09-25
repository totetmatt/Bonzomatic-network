
#include "Instances.h"

#include <stdio.h>
#include <string>
#include <assert.h>


int ScreenWidth = 0;
int ScreenHeight = 0;

float ScreenOffsetX = 0.1f;
float ScreenOffsetY = 0.1f;

float ScreenPercentageX = 0.8f;
float ScreenPercentageY = 0.8f;

int ScreenBlankSpaceX = 30;
int ScreenBlankSpaceY = 30;

float FullScreenOffsetX = 0.1f;
float FullScreenOffsetY = 0.1f;

float FullScreenPercentageX = 0.8f;
float FullScreenPercentageY = 0.8f;


int DelayInitWindows = 2000;
std::string ConfigCommandLine = "skipdialog networkMode=grabber";

std::string ExecutableName = "Bonzomatic.exe";
std::string ServerURL = "ws://127.0.0.1:8000/roomy/";

std::vector<class Instance*> Instances;


float DiapoDelay = 50.0f;
int DiapoLoops = 1;
int DiapoCurrentIndex = 0;
float DiapoCurrentTime = 0;
int DiapoCurrentLoop = 0;
bool DiapoLaunched = false;

bool IsDiapoLaunched() { return DiapoLaunched; }
int GetCurrentDiapo() { return DiapoCurrentIndex; }

void FullscreenIndex(int index) {
  if (index >= Instances.size()) return;
  ChangeDisplay(DisplayAction::SetFullscreen, Instances[index]);
}

void StartDiaporama() {
  DiapoLaunched = true;
  DiapoCurrentIndex = -1;
  DiapoCurrentTime = 0;
  DiapoCurrentLoop = 0;
}

void StopDiaporama() {
  DiapoLaunched = false;
  ChangeDisplay(DisplayAction::ShowMosaic);
}

void UpdateDiaporama(float ElapsedTime) {
  if (DiapoLaunched) {
    DiapoCurrentTime += ElapsedTime;
    if (DiapoCurrentTime >= DiapoDelay || DiapoCurrentIndex < 0) {
      DiapoCurrentTime = 0;
      // Go to next diapo
      
      bool CurDiapoHidden = false;
      do {
        DiapoCurrentIndex += 1;
        if (DiapoCurrentIndex >= Instances.size()) {
          DiapoCurrentIndex = 0;
          DiapoCurrentLoop += 1;
          if (DiapoCurrentLoop >= DiapoLoops) {
            // end of diaporama
            DiapoLaunched = false;
            // staap
          }
        }
        // skip hidden diapo
        CurDiapoHidden = DiapoCurrentIndex < Instances.size() ? Instances[DiapoCurrentIndex]->IsHidden : true;
      } while (CurDiapoHidden && DiapoLaunched);

      if (DiapoLaunched) {
        FullscreenIndex(DiapoCurrentIndex);
      }
      else {
        ChangeDisplay(DisplayAction::ShowMosaic);
      }
    }
  }
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);

Instance::Instance()
{
}

bool Instance::Init(int InstanceIndex, std::string InCoderName)
{
  Index = InstanceIndex;
  CoderName = InCoderName;

  memset(&siStartupInfo, 0, sizeof(siStartupInfo));
  memset(&piProcessInfo, 0, sizeof(piProcessInfo));
  siStartupInfo.cb = sizeof(siStartupInfo);

  //std::string ShaderName = " shader=netshad_" + std::to_string(InstanceIndex) + ".glsl";
  std::string ServerCommand = " serverURL=" + ServerURL + CoderName;

  std::string CommandLine = ServerCommand + std::string(" ") + ConfigCommandLine;
  char* CommandString = new char[CommandLine.size() + 1];
  strncpy_s(CommandString, (CommandLine.size() + 1), CommandLine.c_str(), (CommandLine.size() + 1));

  DWORD dwExitCode = 0;
  if (CreateProcessA(ExecutableName.c_str(),
    CommandString, 0, 0, false,
    CREATE_DEFAULT_ERROR_MODE, 0, 0,
    &siStartupInfo, &piProcessInfo) != false)
  {
    //printf("[LAUNCHER] Created succesfull.\n");

    /* Watch the process. */
    dwExitCode = WaitForSingleObject(piProcessInfo.hProcess, DelayInitWindows);

    //printf("[LAUNCHER] Process initialized.\n");

    EnumWindows(EnumWindowsProc, (LPARAM)(this));
  }
  else
  {
    /* CreateProcess failed */
    size_t iReturnVal = GetLastError();
    printf("[LAUNCHER] Error with creating process: %d\n", iReturnVal);
    return false;
  }

  return true;
}

void Instance::Release()
{
  SendMessage(hwnd, WM_CLOSE, NULL, NULL);
  /* Release handles */
  CloseHandle(piProcessInfo.hProcess);
  CloseHandle(piProcessInfo.hThread);
}


BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
  DWORD dwPID;

  GetWindowThreadProcessId(hwnd, &dwPID);

  Instance* CurrentInstance = (Instance*)(lParam);
  if (dwPID == CurrentInstance->piProcessInfo.dwProcessId) {
    //printf("[LAUNCHER] PID found %d \n", dwPID);
    CurrentInstance->hwnd = hwnd;
    return FALSE;
  }

  return TRUE;
}

void FindDesktopResolution() {
  RECT desktop;
  const HWND hDesktop = GetDesktopWindow();
  GetWindowRect(hDesktop, &desktop);
  ScreenWidth = desktop.right - desktop.left;
  ScreenHeight = desktop.bottom - desktop.top;
}

void ToggleHidden(Instance* Target) {
  Target->IsHidden = !Target->IsHidden;
  //ShowWindow(Target->hwnd, Target->IsHidden ? SW_HIDE : SW_SHOW);
  ChangeDisplay(DisplayAction::ShowMosaic);
}

bool GridForceRatio = false;
float GridWantedRatio = 1.7777f;
bool FullForceRatio = false;
float FullWantedRatio = 1.7777f;

void SetInstancePositionRatio(Instance* Cur, int PosX, int PosY, int Width, int Height, bool UseRatio, float WantedRatio) {

  if (UseRatio && Height>0) {
    float Ratio = float(Width) / Height;
    if (Ratio > WantedRatio) {
      int NewWidth = Height * WantedRatio;
      PosX += (Width - NewWidth) / 2;
      Width = NewWidth;
    } else {
      int NewHeight = Width / WantedRatio;
      PosY += (Height - NewHeight) / 2;
      Height = NewHeight;
    }
  }

  SetWindowPos(Cur->hwnd, HWND_TOP, PosX, PosY, Width, Height, SWP_ASYNCWINDOWPOS);
  ShowWindow(Cur->hwnd, SW_SHOW);
}

void SetMinimalPosition(Instance* Cur) {
  SetWindowPos(Cur->hwnd, HWND_TOP, 0, 0, 1, 1, SWP_ASYNCWINDOWPOS);
  ShowWindow(Cur->hwnd, SW_HIDE);
}

void ChangeDisplay(DisplayAction Action, Instance* Target) {

  int TableWidth = ScreenWidth * ScreenPercentageX;
  int TableHeight = ScreenHeight * ScreenPercentageY;
  int PixelScreenOffsetX = ScreenWidth * ScreenOffsetX;
  int PixelScreenOffsetY = ScreenHeight * ScreenOffsetY;

  bool ShowOnlyHidden = true;
  int NumberOfInstances = 0;
  for (int i = 0; i < Instances.size(); ++i) {
    if (!ShowOnlyHidden || !Instances[i]->IsHidden) {
      ++NumberOfInstances;
    }
  }
  //int NumberOfInstances = Instances.size();
  int NumColumn = NumberOfInstances<1 ? 1 : ceil(sqrt(NumberOfInstances));
  int NumRow = NumberOfInstances < 1 ? 1 : ceil(float(NumberOfInstances) / NumColumn);
  int ColumnSize = TableWidth / NumColumn;
  int RowSize = TableHeight / NumRow;
  
  int CurIndex = 0;
  for (auto const& Cur : Instances) {

    int PosX = (CurIndex % NumColumn) * ColumnSize + PixelScreenOffsetX;
    int PosY = floor(CurIndex / NumColumn) * RowSize + PixelScreenOffsetY;

    if (Action == DisplayAction::FirstDisplay || Action == DisplayAction::ShowMosaic) {  
      if (Cur->IsHidden) {
        SetMinimalPosition(Cur);
      } else {
        SetInstancePositionRatio(Cur, PosX, PosY, ColumnSize - ScreenBlankSpaceX, RowSize - ScreenBlankSpaceY, GridForceRatio, GridWantedRatio);
      }
      Cur->IsFullScreen = false;
    }

    if (Action == DisplayAction::SetFullscreen) {
      if (Cur == Target) {
        int FullWidth = ScreenWidth * FullScreenPercentageX;
        int FullHeight = ScreenHeight * FullScreenPercentageY;
        int PixelFullScreenOffsetX = ScreenWidth * FullScreenOffsetX;
        int PixelFullScreenOffsetY = ScreenHeight * FullScreenOffsetY;

        SetInstancePositionRatio(Cur, PixelFullScreenOffsetX, PixelFullScreenOffsetY, FullWidth, FullHeight, FullForceRatio, FullWantedRatio);
        Cur->IsFullScreen = true;
      } else {
        Cur->IsFullScreen = false;
        SetMinimalPosition(Cur);
      }
    }
    
    if (Action == DisplayAction::FirstDisplay) {
      LONG WindowStyle = GetWindowLong(Cur->hwnd, GWL_STYLE);
      WindowStyle &= ~WS_CAPTION;
      WindowStyle &= ~WS_MAXIMIZEBOX;
      WindowStyle &= ~WS_THICKFRAME;
      SetWindowLong(Cur->hwnd, GWL_STYLE, WindowStyle);
    }

    if (!ShowOnlyHidden || !Cur->IsHidden) {
      ++CurIndex;
    }
  }
}

Instance* AddInstance(std::string CoderName) {
  Instance* NewInstance = new Instance();
  int NewIndex = Instances.size();
  if (NewInstance->Init(NewIndex, CoderName)) {
    Instances.push_back(NewInstance);
  }
  else {
    delete NewInstance;
  }
  return NewInstance;
}

bool LaunchInstances(jsonxx::Object options)
{
  if (options.has<jsonxx::Object>("mosaic"))
  {
    jsonxx::Object winjson = options.get<jsonxx::Object>("mosaic");
    if (winjson.has<jsonxx::Number>("startpercent_x")) ScreenOffsetX = winjson.get<jsonxx::Number>("startpercent_x");
    if (winjson.has<jsonxx::Number>("startpercent_y")) ScreenOffsetY = winjson.get<jsonxx::Number>("startpercent_y");
    if (winjson.has<jsonxx::Number>("sizepercent_x")) ScreenPercentageX = winjson.get<jsonxx::Number>("sizepercent_x");
    if (winjson.has<jsonxx::Number>("sizepercent_y")) ScreenPercentageY = winjson.get<jsonxx::Number>("sizepercent_y");
    if (winjson.has<jsonxx::Number>("border_x")) ScreenBlankSpaceX = winjson.get<jsonxx::Number>("border_x");
    if (winjson.has<jsonxx::Number>("border_y")) ScreenBlankSpaceY = winjson.get<jsonxx::Number>("border_y");
    if (winjson.has<jsonxx::Boolean>("forceratio")) GridForceRatio = winjson.get<jsonxx::Boolean>("forceratio");
    if (winjson.has<jsonxx::Number>("wantedratio")) GridWantedRatio = winjson.get<jsonxx::Number>("wantedratio");
  }

  if (options.has<jsonxx::Object>("fullscreen"))
  {
    jsonxx::Object winjson = options.get<jsonxx::Object>("fullscreen");
    if (winjson.has<jsonxx::Number>("startpercent_x")) FullScreenOffsetX = winjson.get<jsonxx::Number>("startpercent_x");
    if (winjson.has<jsonxx::Number>("startpercent_y")) FullScreenOffsetY = winjson.get<jsonxx::Number>("startpercent_y");
    if (winjson.has<jsonxx::Number>("sizepercent_x")) FullScreenPercentageX = winjson.get<jsonxx::Number>("sizepercent_x");
    if (winjson.has<jsonxx::Number>("sizepercent_y")) FullScreenPercentageY = winjson.get<jsonxx::Number>("sizepercent_y");
    if (winjson.has<jsonxx::Boolean>("forceratio")) FullForceRatio = winjson.get<jsonxx::Boolean>("forceratio");
    if (winjson.has<jsonxx::Number>("wantedratio")) FullWantedRatio = winjson.get<jsonxx::Number>("wantedratio");
  }

  if (options.has<jsonxx::Object>("bonzo"))
  {
    jsonxx::Object bonzojson = options.get<jsonxx::Object>("bonzo");
    if (bonzojson.has<jsonxx::Number>("delay_between_spawn")) DelayInitWindows = bonzojson.get<jsonxx::Number>("delay_between_spawn");
    if (bonzojson.has<jsonxx::String>("commandline")) ConfigCommandLine = bonzojson.get<jsonxx::String>("commandline");
    if (bonzojson.has<jsonxx::String>("exename")) ExecutableName = bonzojson.get<jsonxx::String>("exename");
  }

  if (options.has<jsonxx::Object>("diaporama"))
  {
    jsonxx::Object diapojson = options.get<jsonxx::Object>("diaporama");
    if (diapojson.has<jsonxx::Number>("delay")) DiapoDelay = diapojson.get<jsonxx::Number>("delay");
    if (diapojson.has<jsonxx::Number>("loops")) DiapoLoops = diapojson.get<jsonxx::Number>("loops");
  }

  bool UseNetwork = false;
  if (options.has<jsonxx::Object>("network"))
  {
    jsonxx::Object netjson = options.get<jsonxx::Object>("network");
    if (netjson.has<jsonxx::Boolean>("receiveuserlist")) UseNetwork = netjson.get<jsonxx::Boolean>("receiveuserlist");
    if (netjson.has<jsonxx::String>("serverURL")) ServerURL = netjson.get<jsonxx::String>("serverURL");
  }

  if (UseNetwork) {
    // TODO
  } else {
    if (options.has<jsonxx::Array>("coders")) {
      jsonxx::Array coderjson = options.get<jsonxx::Array>("coders");
      for (int i = 0; i < coderjson.size(); ++i) {
        std::string codername = coderjson.get<jsonxx::String>(i);
        printf("[LAUNCHER] Add coder %s \n", codername);
        AddInstance(codername);
      }
    } else {
      printf("[LAUNCHER] Missing coders section in config file \n");
    }
  }

  ChangeDisplay(DisplayAction::FirstDisplay);

  return true;
}

void ReleaseInstances() {
  for (auto const& Cur : Instances) {
    Cur->Release();
    delete Cur;
  }
  Instances.clear();
}

std::vector<class Instance*>& GetInstances()
{
  return Instances;
}