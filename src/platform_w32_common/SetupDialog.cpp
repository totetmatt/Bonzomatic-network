#include <windows.h>
#ifdef __MINGW32__
#include <stdio.h>
#endif
#include <tchar.h>
#include "../Renderer.h"
#include "resource.h"

#include <vector>
#include <algorithm>

class CSetupDialog;

CSetupDialog * pGlobal = NULL;

INT_PTR CALLBACK DlgFunc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

class CSetupDialog
{
public:
  typedef struct {
    int nWidth;
    int nHeight;
  } RESOLUTION;

  std::vector<RESOLUTION> gaResolutions;
  HWND hWndSetupDialog;

  CSetupDialog(void)
  {
    hWndSetupDialog = NULL;
  }

  ~CSetupDialog(void)
  {
  }

  RENDERER_SETTINGS * setup;
  NETWORK_SETTINGS * network;

  int __cdecl ResolutionSort(const void * a, const void * b)
  {
    RESOLUTION * aa = (RESOLUTION *)a;
    RESOLUTION * bb = (RESOLUTION *)b;
    if (aa->nWidth < bb->nWidth) return -1;
    if (aa->nWidth > bb->nWidth) return 1;
    if (aa->nHeight < bb->nHeight) return -1;
    if (aa->nHeight > bb->nHeight) return 1;
    return 0;
  }
  bool DialogProcedure( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) 
  {
    switch (uMsg) {
    case WM_INITDIALOG:
      {        
        int i = 0;
        while(1) {
          DEVMODE d;
          BOOL h = EnumDisplaySettings(NULL,i++,&d);
          if (!h) break;

          //if ((d.dmPelsWidth * 9) / 16 != d.dmPelsHeight) continue;
          if (d.dmBitsPerPel != 32) continue;
          if (d.dmDisplayOrientation != DMDO_DEFAULT) continue;

          if (!gaResolutions.size() 
            || gaResolutions[ gaResolutions.size() - 1 ].nWidth != d.dmPelsWidth 
            || gaResolutions[ gaResolutions.size() - 1 ].nHeight != d.dmPelsHeight) 
          {
            RESOLUTION res;
            res.nWidth  = d.dmPelsWidth;
            res.nHeight = d.dmPelsHeight;
            gaResolutions.push_back(res);

          }
        }
        //std::sort(gaResolutions.begin(),gaResolutions.end(),&CSetupDialog::ResolutionSort);

        bool bFoundBest = false;
        for (i=0; i<gaResolutions.size() ; i++)
        {
          TCHAR s[50];
          _sntprintf(s,50,_T("%d * %d"),gaResolutions[i].nWidth,gaResolutions[i].nHeight);
          SendDlgItemMessage(hWnd, IDC_RESOLUTION, CB_ADDSTRING, 0, (LPARAM)s);

          if (gaResolutions[i].nWidth == setup->nWidth && gaResolutions[i].nHeight == setup->nHeight)
          {
            SendDlgItemMessage(hWnd, IDC_RESOLUTION, CB_SETCURSEL, i, 0);
            bFoundBest = true;
          }
          if (!bFoundBest && gaResolutions[i].nWidth == GetSystemMetrics(SM_CXSCREEN) && gaResolutions[i].nHeight == GetSystemMetrics(SM_CYSCREEN))
          {
            SendDlgItemMessage(hWnd, IDC_RESOLUTION, CB_SETCURSEL, i, 0);
          }
        }

        if (setup->windowMode == RENDERER_WINDOWMODE_FULLSCREEN) {
          SendDlgItemMessage(hWnd, IDC_FULL, BM_SETCHECK, 1, 1);
        } else if(setup->windowMode == RENDERER_WINDOWMODE_BORDERLESS) {
          SendDlgItemMessage(hWnd, IDC_BORDERLESS, BM_SETCHECK, 1, 1);
        } else {
          SendDlgItemMessage(hWnd, IDC_WINDOWED, BM_SETCHECK, 1, 1);
        }
        if (setup->bVsync) {
          SendDlgItemMessage(hWnd, IDC_VSYNC, BM_SETCHECK, 1, 1);
        }

        if (!network->EnableNetwork) {
          SendDlgItemMessage(hWnd, IDC_OFFLINE, BM_SETCHECK, 1, 1);
        } else if(network->NetworkModeString == "sender") {
          SendDlgItemMessage(hWnd, IDC_SENDER, BM_SETCHECK, 1, 1);
        } else {
          SendDlgItemMessage(hWnd, IDC_GRABBER, BM_SETCHECK, 1, 1);
        }
        
        std::string ServerName;
        std::string RoomName;
        std::string NickName;
        Network_Break_URL(network->ServerURL, ServerName, RoomName, NickName);
        
        SetDlgItemText(hWnd, IDC_SERVER, ServerName.c_str());
        SetDlgItemText(hWnd, IDC_ROOM, RoomName.c_str());
        SetDlgItemText(hWnd, IDC_NICK, NickName.c_str());

        return true;
      } break;

    case WM_COMMAND:
      {
        switch( LOWORD(wParam) ) 
        {
        case IDOK: 
          {
            setup->nWidth  = gaResolutions[ SendDlgItemMessage(hWnd, IDC_RESOLUTION, CB_GETCURSEL, 0, 0) ].nWidth;
            setup->nHeight = gaResolutions[ SendDlgItemMessage(hWnd, IDC_RESOLUTION, CB_GETCURSEL, 0, 0) ].nHeight;
            setup->windowMode = RENDERER_WINDOWMODE_WINDOWED;
            if (SendDlgItemMessage(hWnd, IDC_FULL, BM_GETCHECK, 0, 0)) {
              setup->windowMode = RENDERER_WINDOWMODE_FULLSCREEN;
            }
            if (SendDlgItemMessage(hWnd, IDC_BORDERLESS, BM_GETCHECK, 0, 0)) {
              setup->windowMode = RENDERER_WINDOWMODE_BORDERLESS;
            }

            setup->bVsync = SendDlgItemMessage(hWnd, IDC_VSYNC, BM_GETCHECK , 0, 0) > 0;

            // ACCEPT
            if (SendDlgItemMessage(hWnd, IDC_OFFLINE, BM_GETCHECK, 0, 0)) {
              network->EnableNetwork = false;
            }
            network->NetworkModeString = "grabber";
            if (SendDlgItemMessage(hWnd, IDC_SENDER, BM_GETCHECK, 0, 0)) {
              network->NetworkModeString = "sender";
            }

            int ServerLen = SendDlgItemMessage(hWnd, IDC_SERVER, WM_GETTEXTLENGTH, 0, 0);
            char ServerName[512];
            GetDlgItemText(hWnd, IDC_SERVER, ServerName, min(ServerLen+1,511));

            int RoomLen = SendDlgItemMessage(hWnd, IDC_ROOM, WM_GETTEXTLENGTH, 0, 0);
            char RoomName[512];
            GetDlgItemText(hWnd, IDC_ROOM, RoomName, min(RoomLen+1,511));

            int NickLen = SendDlgItemMessage(hWnd, IDC_NICK, WM_GETTEXTLENGTH, 0, 0);
            char NickName[512];
            GetDlgItemText(hWnd, IDC_NICK, NickName, min(NickLen+1,511));

            network->ServerURL = std::string(ServerName) + "/" + RoomName + "/" + NickName;

            EndDialog (hWnd, TRUE);
          } break;
        case IDCANCEL: 
          {
            EndDialog (hWnd, FALSE);
          } break;
        } 
      } break;
    }
    return false;
  }

  bool Open(HINSTANCE hInstance, HWND hWnd) {
    return DialogBoxParam(hInstance,MAKEINTRESOURCE(IDD_SETUP),hWnd,DlgFunc,(LPARAM)this) != NULL;
  }
};

INT_PTR CALLBACK DlgFunc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if (uMsg == WM_INITDIALOG) {
    pGlobal = (CSetupDialog *)lParam; // todo: split to multiple hWnd-s! (if needed)
  }
  return pGlobal->DialogProcedure(hWnd,uMsg,wParam,lParam);
}

bool Renderer::OpenSetupDialog( RENDERER_SETTINGS * settings, NETWORK_SETTINGS* netSettings )
{
  CSetupDialog dlg;
  dlg.setup = settings;
  dlg.network = netSettings;
  return dlg.Open( GetModuleHandle(NULL), NULL );
}
