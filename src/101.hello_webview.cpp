#include <tchar.h>
#include <wil/com.h>
#include <windows.h>
#include <wrl.h>

#include <string>

#include "WebView2.h"

namespace wrl = Microsoft::WRL;

namespace {
wil::com_ptr<ICoreWebView2Controller> webview_controller;
wil::com_ptr<ICoreWebView2> webview;
HINSTANCE hInst;
}  // namespace

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  WNDCLASSEX window_class;

  window_class.cbSize = sizeof(WNDCLASSEX);
  window_class.style = CS_HREDRAW | CS_VREDRAW;
  window_class.lpfnWndProc = WndProc;
  window_class.cbClsExtra = 0;
  window_class.cbWndExtra = 0;
  window_class.hInstance = hInstance;
  window_class.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
  window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
  window_class.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  window_class.lpszMenuName = NULL;
  window_class.lpszClassName = _T("DesktopApp");
  window_class.hIconSm = LoadIcon(window_class.hInstance, IDI_APPLICATION);

  if (!RegisterClassEx(&window_class)) {
    MessageBox(NULL, _T("Call to RegisterClassEx failed!"), _T("Windows Desktop Guided Tour"), NULL);

    return EXIT_FAILURE;
  }

  hInst = hInstance;
  HWND window = CreateWindow(window_class.lpszClassName, _T("Hello WebView"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
                             CW_USEDEFAULT, 1200, 900, NULL, NULL, hInstance, NULL);

  if (!window) {
    MessageBox(NULL, _T("Call to CreateWindow failed!"), _T("Windows Desktop Guided Tour"), NULL);
    return EXIT_FAILURE;
  }

  ShowWindow(window, nCmdShow);
  UpdateWindow(window);

  // <-- WebView2 sample code starts here -->
  // Step 3 - Create a single WebView within the parent window Locate the browser and set up the environment for WebView
  CreateCoreWebView2EnvironmentWithOptions(
      nullptr, nullptr, nullptr,
      wrl::Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>([window](HRESULT result,
                                                                                         ICoreWebView2Environment *env)
                                                                                    -> HRESULT {
        // Create a CoreWebView2Controller and get the associated CoreWebView2 whose parent is the main window hWnd
        env->CreateCoreWebView2Controller(
            window,
            wrl::Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                [window](HRESULT result, ICoreWebView2Controller *controller) -> HRESULT {
                  if (controller != nullptr) {
                    webview_controller = controller;
                    webview_controller->get_CoreWebView2(&webview);
                  }

                  // Add a few settings for the webview
                  // The demo step is redundant since the values are the default settings
                  ICoreWebView2Settings *Settings;
                  webview->get_Settings(&Settings);
                  Settings->put_IsScriptEnabled(TRUE);
                  Settings->put_AreDefaultScriptDialogsEnabled(TRUE);
                  Settings->put_IsWebMessageEnabled(TRUE);

                  // Resize WebView to fit the bounds of the parent window
                  RECT bounds;
                  GetClientRect(window, &bounds);
                  webview_controller->put_Bounds(bounds);

                  // Schedule an async task to Google
                  webview->Navigate(L"https://www.google.com/");

                  // Step 4 - Navigation events
                  // register an ICoreWebView2NavigationStartingEventHandler to cancel any non-https navigation
                  EventRegistrationToken token;
                  webview->add_NavigationStarting(
                      wrl::Callback<ICoreWebView2NavigationStartingEventHandler>(
                          [](ICoreWebView2 *webview, ICoreWebView2NavigationStartingEventArgs *args) -> HRESULT {
                            PWSTR uri;
                            args->get_Uri(&uri);
                            std::wstring source(uri);
                            if (!source.starts_with(L"https://")) {
                              args->put_Cancel(true);
                            }
                            CoTaskMemFree(uri);
                            return S_OK;
                          })
                          .Get(),
                      &token);

                  // Step 5 - Scripting
                  // Schedule an async task to add initialization script that freezes the Object object
                  webview->AddScriptToExecuteOnDocumentCreated(L"Object.freeze(Object);", nullptr);
                  // Schedule an async task to get the document URL
                  webview->ExecuteScript(L"window.document.URL;",
                                         wrl::Callback<ICoreWebView2ExecuteScriptCompletedHandler>(
                                             [](HRESULT errorCode, LPCWSTR resultObjectAsJson) -> HRESULT {
                                               LPCWSTR URL = resultObjectAsJson;
                                               // doSomethingWithURL(URL);
                                               return S_OK;
                                             })
                                             .Get());

                  // Step 6 - Communication between host and web content
                  // Set an event handler for the host to return received message back to the web content
                  webview->add_WebMessageReceived(
                      wrl::Callback<ICoreWebView2WebMessageReceivedEventHandler>(
                          [](ICoreWebView2 *webview, ICoreWebView2WebMessageReceivedEventArgs *args) -> HRESULT {
                            PWSTR message;
                            args->TryGetWebMessageAsString(&message);
                            // processMessage(&message);
                            webview->PostWebMessageAsString(message);
                            CoTaskMemFree(message);
                            return S_OK;
                          })
                          .Get(),
                      &token);

                  // Schedule an async task to add initialization script
                  // that 1) Add an listener to print message from the host
                  // 2) Post document URL to the host
                  webview->AddScriptToExecuteOnDocumentCreated(
                      L"window.chrome.webview.addEventListener(\'message\',"
                      L" event => alert(event.data));"
                      L"window.chrome.webview.postMessage(window.document."
                      L"URL);",
                      nullptr);

                  return S_OK;
                })
                .Get());
        return S_OK;
      }).Get());

  // <-- WebView2 sample code ends here -->

  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return static_cast<int>(msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
  switch (message) {
    case WM_SIZE:
      if (webview_controller != nullptr) {
        RECT bounds;
        GetClientRect(hWnd, &bounds);
        webview_controller->put_Bounds(bounds);
      };
      break;
    case WM_DESTROY:
      PostQuitMessage(0);
      break;
    default:
      return DefWindowProc(hWnd, message, wParam, lParam);
      break;
  }

  return 0;
}
