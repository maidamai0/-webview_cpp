#include "AppWindow.h"

#include <WinUser.h>

#include <filesystem>

#include "App.h"
#include "CheckFailure.h"
#include "CompositionHost.h"
#include "common/common.hpp"

namespace wrl = Microsoft::WRL;

AppWindow::AppWindow() : composition_host_(std::make_unique<CompositionHost>()) {
  window_ = CreateWindowW(GetWindowClass(), TEXT("Application"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT,
                          0, nullptr, nullptr, hInst, nullptr);

  SetWindowLongPtr(window_, GWLP_USERDATA, (LONG_PTR)this);
  ShowWindow(window_, nCmdShow);
  UpdateWindow(window_);
  InitializeWebView();
}

// Register the Win32 window class for the app window.
PCWSTR AppWindow::GetWindowClass() {
  // Only do this once
  static PCWSTR windowClass = [] {
    static WCHAR windowClass[MAX_LOADSTRING];
    LoadStringW(hInst, IDC_WEBVIEW2SAMPLEWINCOMP, windowClass, MAX_LOADSTRING);

    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInst;
    wcex.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_WEBVIEW2SAMPLEWINCOMP));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_WEBVIEW2SAMPLEWINCOMP);
    wcex.lpszClassName = windowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    RegisterClassExW(&wcex);
    return windowClass;
  }();
  return windowClass;
}

LRESULT CALLBACK AppWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
  if (auto app = (AppWindow*)GetWindowLongPtr(hWnd, GWLP_USERDATA)) {
    if (app->HandleWindowMessage(hWnd, message, wParam, lParam)) {
      return 0;
    }
  }
  return DefWindowProc(hWnd, message, wParam, lParam);
}

bool AppWindow::HandleWindowMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
  if (message >= WM_MOUSEFIRST && message <= WM_MOUSELAST) {
    composition_host_->OnMouseMessage(message, wParam, lParam);
    return true;
  }

  switch (message) {
    case WM_MOVE: {
      if (webview_controller_) {
        webview_controller_->NotifyParentWindowPositionChanged();
      }
      return true;
    }
    case WM_SIZE: {
      RECT availableBounds = {0};
      GetClientRect(window_, &availableBounds);
      composition_host_->SetBounds(availableBounds);
      return true;
    }
    case WM_COMMAND: {
      int wmId = LOWORD(wParam);
      // Parse the menu selections:
      switch (wmId) {
        case IDM_ABOUT:
          DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
          return true;
        case IDM_EXIT:
          close_window();
          return true;
      }
      break;
    }
    case WM_PAINT: {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hWnd, &ps);
      EndPaint(hWnd, &ps);
      return true;
    }
    case WM_DESTROY:
      PostQuitMessage(0);
      return true;
  }
  return false;
}

void AppWindow::InitializeWebView() {
  auto options = wrl::Make<CoreWebView2EnvironmentOptions>();
  HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(
      nullptr, nullptr, options.Get(),
      wrl::Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
          [this](HRESULT result, ICoreWebView2Environment* environment) -> HRESULT {
            webview_environment_ = environment;
            wil::com_ptr<ICoreWebView2Environment3> webViewEnvironment3 =
                webview_environment_.try_query<ICoreWebView2Environment3>();

            if (webViewEnvironment3) {
              CHECK_FAILURE(webViewEnvironment3->CreateCoreWebView2CompositionController(
                  window_, wrl::Callback<ICoreWebView2CreateCoreWebView2CompositionControllerCompletedHandler>(
                               this, &AppWindow::on_create_webview_controller_completed)
                               .Get()));
            }
            return S_OK;
          })
          .Get());
  assert(SUCCEEDED(hr));
}

HRESULT AppWindow::on_create_webview_controller_completed(HRESULT result,
                                                          ICoreWebView2CompositionController* compositionController) {
  if (result == S_OK) {
    webview_composition_controller_ = compositionController;
    CHECK_FAILURE(webview_composition_controller_->QueryInterface(IID_PPV_ARGS(&webview_controller_)));
    CHECK_FAILURE(webview_controller_->get_CoreWebView2(&webview_));
    webview_controller_->put_IsVisible(true);
    webview_->Navigate(common::resoruce_path("WebView2SamplePage.html").native().c_str());
  } else {
    ShowFailure(result, TEXT("Failed to create webview"));
  }
  composition_host_->Initialize(this);
  return S_OK;
}

INT_PTR CALLBACK AppWindow::About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
  UNREFERENCED_PARAMETER(lParam);
  switch (message) {
    case WM_INITDIALOG:
      return (INT_PTR)TRUE;
    case WM_COMMAND:
      if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
        EndDialog(hDlg, LOWORD(wParam));
        return (INT_PTR)TRUE;
      }
      break;
  }
  return (INT_PTR)FALSE;
}

void AppWindow::close_window() {
  if (webview_controller_) {
    webview_controller_->Close();
    webview_controller_ = nullptr;
    webview_ = nullptr;
  }
  webview_environment_ = nullptr;

  DestroyWindow(window_);
}