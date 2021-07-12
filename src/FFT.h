#define FFT_SIZE 1024

namespace FFT
{
  extern float fAmplification;
  extern bool bPeakNormalization;
  extern float fPeakMinValue;
  extern float fPeakSmoothing;

  bool Open(bool CapturePlaybackDevices, const char* CaptureDeviceSearchString);
  bool GetFFT( float * _samples );
  void Close();
}