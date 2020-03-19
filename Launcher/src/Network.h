#pragma once

#include <string>

namespace Network
{
void PrepareConnection();
void OpenConnection(std::string ServerURL);
void Tick();
void Release();
}