#include <stdio.h>
#include <string>
#include <assert.h>
#include <vector>

#include "Network.h"
#include "Instances.h"
#include "ControlWindow.h"
#include "jsonxx.h"

// TMP
#include <windows.h>

// TODO:
// bonzomatic: start with text editor hidden
// button to toggle text code ?
// full screen size in config file


LARGE_INTEGER LastPCV = { 0 };
double currentTime = 0.0;
double WhatTime()
{
  LARGE_INTEGER count, freq;
  if (!LastPCV.QuadPart) {
    QueryPerformanceCounter(&LastPCV);
  }
  QueryPerformanceCounter(&count);
  QueryPerformanceFrequency(&freq);

  currentTime += (double)(count.QuadPart - LastPCV.QuadPart) / (double)(freq.QuadPart);

  LastPCV = count;

  return currentTime;
}

int main(int argc, const char *argv[])
{
  printf("[LAUNCHER] Started \n");

  std::string configFile = "launcher.json";
  jsonxx::Object options;
  FILE * fConf = fopen(configFile.c_str(), "rb");
  if (fConf)
  {
    printf("Launcher config file found, parsing...\n");

    char szConfig[65535];
    memset(szConfig, 0, 65535);
    fread(szConfig, 1, 65535, fConf);
    fclose(fConf);

    options.parse(szConfig);
  }

  FindDesktopResolution();

  //Network::PrepareConnection();
  //Network::OpenConnection("ws://127.0.0.1:8000/");

  LaunchInstances(options);

  InitControlWindow(options);

  double Time = WhatTime();
  while (!WantsToQuit()) {
    double NewTime = WhatTime();
    float ElapsedTime = NewTime - Time;
    Time = NewTime;
    UpdateControlWindow(ElapsedTime);
    //Network::Tick();
	  //Sleep(30);
  }
  CloseControlWindow();
  

  ReleaseInstances();

  //Network::Release();


  exit(0);
  return 0;
}