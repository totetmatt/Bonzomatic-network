#pragma once

#include <string>

namespace Network
{
  void PrepareConnection();
  void OpenConnection(std::string ServerURL);
  void Tick(float ElapsedTime);
  void Release();
  bool IsLaunched();
  bool IsActive();
}