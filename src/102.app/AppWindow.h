#pragma once

#include <regex>

#define MAX_LOADSTRING 100

class CompositionHost;

class AppWindow {
 public:
  AppWindow();

  ICoreWebView2Controller* GetWebViewController() { return webview_controller_.get(); }

  ICoreWebView2CompositionController* GetWebViewCompositionController() {
    return webview_composition_controller_.get();
  }

  ICoreWebView2* GetWebView() { return webview_.get(); }

  HWND GetMainWindow() { return window_; }

 private:
  static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
  static INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
  static PCWSTR GetWindowClass();

  bool HandleWindowMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
  void InitializeWebView();
  HRESULT on_create_webview_controller_completed(HRESULT result,
                                                 ICoreWebView2CompositionController* compositionController);
  void close_window();

 private:
  HWND window_ = nullptr;
  std::unique_ptr<CompositionHost> composition_host_;

  wil::com_ptr<ICoreWebView2Environment> webview_environment_;
  wil::com_ptr<ICoreWebView2CompositionController> webview_composition_controller_;
  wil::com_ptr<ICoreWebView2Controller> webview_controller_;
  wil::com_ptr<ICoreWebView2> webview_;
};
