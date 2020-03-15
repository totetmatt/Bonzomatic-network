namespace Network
{
  void LoadSettings(jsonxx::Object & o);
  void CommandLine(int argc, const char *argv[]);
  void OpenConnection();
  void BroadcastMessage(const char* msg);
  void SendShader(const char* msg);
  bool IsConnected();
  bool IsGrabber();
  bool HasNewShader();
  std::string GetNewShader();
  void Tick();
  void Release();
}