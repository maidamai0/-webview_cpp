
#include "App.h"

#include "AppWindow.h"
#include "CompositionHost.h"

HINSTANCE hInst;
int nCmdShow;

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int m_nCmdShow) {
  hInst = hInstance;
  nCmdShow = m_nCmdShow;

  AppWindow window;

  const auto accelerators = LoadAccelerators(hInst, MAKEINTRESOURCE(IDC_WEBVIEW2SAMPLEWINCOMP));
  MSG msg;
  while (GetMessage(&msg, nullptr, 0, 0)) {
    if (!TranslateAccelerator(msg.hwnd, accelerators, &msg)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }

  return static_cast<int>(msg.wParam);
}
