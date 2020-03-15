namespace Network
{
  void LoadSettings(jsonxx::Object & o);
  void CommandLine(int argc, const char *argv[]);
  void OpenConnection();
  void BroadcastMessage(const char* msg);
  void SendShader(const char* msg);
  bool HasRecievedShader();
  std::string GetLastShader();
  void Tick();
  void Release();
}