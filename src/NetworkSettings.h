#pragma once

#include <string>

struct NETWORK_SETTINGS {
  bool EnableNetwork;
  std::string ServerURL;
  std::string NetworkModeString;
};

void Network_Break_URL(std::string ServerURL, std::string& ServerName, std::string& RoomName, std::string& NickName);