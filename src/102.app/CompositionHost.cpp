#include "CompositionHost.h"

#include <d2d1_1.h>
#include <windowsx.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/base.h>

#include "CheckFailure.h"

using namespace winrt;
using namespace winrt::Windows::System;
using namespace winrt::Windows::UI;
using namespace winrt::Windows::UI::Composition;
using namespace winrt::Windows::UI::Composition::Desktop;
using namespace winrt::Windows::Foundation::Numerics;

// offset value for each visual, make sure to update this when modifying number of visuals to
// be created
static const std::vector<float2> kOffSetValues{{0, 0}, {1.25f, 0}, {0.5f, 0.5f}, {0, 1.25f}, {1.25f, 1.25f}};
static const size_t kNumberOfVisuals = kOffSetValues.size();

CompositionHost::~CompositionHost() {
  if (webview_composition_controller_) {
    webview_composition_controller_->put_RootVisualTarget(nullptr);
    DestroyWinCompVisualTree();
  }
}

void CompositionHost::Initialize(AppWindow* window) {
  assert(window);
  m_appWindow = window;
  webview_controller_ = m_appWindow->GetWebViewController();
  webview_composition_controller_ = m_appWindow->GetWebViewCompositionController();
  GetClientRect(m_appWindow->GetMainWindow(), &m_appBounds);
  EnsureDispatcherQueue();
  if (m_dispatcherQueueController) {
    m_compositor = Compositor();
  }

  if (m_compositor && webview_composition_controller_) {
    CreateDesktopWindowTarget(m_appWindow->GetMainWindow());
    CreateCompositionRoot();
    CreateVisuals();
    ResizeAllVisuals();
  }
}

void CompositionHost::EnsureDispatcherQueue() {
  namespace abi = ABI::Windows::System;

  if (m_dispatcherQueueController == nullptr) {
    DispatcherQueueOptions options{
        sizeof(DispatcherQueueOptions), /* dwSize */
        DQTYPE_THREAD_CURRENT,          /* threadType */
        DQTAT_COM_ASTA                  /* apartmentType */
    };

    winrt::Windows::System::DispatcherQueueController controller{nullptr};
    check_hresult(CreateDispatcherQueueController(
        options, reinterpret_cast<abi::IDispatcherQueueController**>(put_abi(controller))));
    m_dispatcherQueueController = controller;
  }
}

void CompositionHost::CreateDesktopWindowTarget(HWND window) {
  namespace abi = ABI::Windows::UI::Composition::Desktop;

  auto interop = m_compositor.as<abi::ICompositorDesktopInterop>();
  check_hresult(interop->CreateDesktopWindowTarget(window, false,
                                                   reinterpret_cast<abi::IDesktopWindowTarget**>(put_abi(m_target))));
}

void CompositionHost::CreateCompositionRoot() {
  m_rootVisual = m_compositor.CreateContainerVisual();
  m_rootVisual.RelativeSizeAdjustment({1.0f, 1.0f});
  m_rootVisual.Offset({0, 0, 0});
  m_target.Root(m_rootVisual);
}

void CompositionHost::OnMouseMessage(UINT message, WPARAM wParam, LPARAM lParam) {
  if (m_rootVisual && webview_composition_controller_) {
    POINT point;
    point.x = GET_X_LPARAM(lParam);
    point.y = GET_Y_LPARAM(lParam);
    UpdateVisual(point, message, wParam);
  }
}

void CompositionHost::DestroyWinCompVisualTree() {
  if (m_webViewVisual) {
    m_webViewVisual = nullptr;

    m_rootVisual.Children().RemoveAll();
    m_rootVisual = nullptr;

    m_target.Root(nullptr);
    m_target = nullptr;
  }
}

void CompositionHost::CreateWebViewVisual() {
  assert(webview_controller_ && m_rootVisual);
  m_webViewVisual = m_compositor.CreateContainerVisual();
  m_rootVisual.Children().InsertAtTop(m_webViewVisual);
  CHECK_FAILURE(webview_composition_controller_->put_RootVisualTarget(m_webViewVisual.as<IUnknown>().get()));
}

void CompositionHost::AddElement() {
  assert(m_rootVisual);
  auto element = m_compositor.CreateSpriteVisual();
  element.Brush(m_compositor.CreateColorBrush(RandomBlue()));
  m_rootVisual.Children().InsertAtTop(element);
}

void CompositionHost::UpdateVisual(POINT point, UINT message, WPARAM wParam) {
  ContainerVisual selectedVisual = FindVisual(point);
  if (selectedVisual == nullptr) return;

  if (selectedVisual != m_webViewVisual && message == WM_LBUTTONDOWN) {
    selectedVisual.as<SpriteVisual>().Brush(m_compositor.CreateColorBrush(RandomBlue()));
  } else if (selectedVisual == m_webViewVisual) {
    auto offset = selectedVisual.Offset();
    point.x -= offset.x;
    point.y -= offset.y;
    CHECK_FAILURE(webview_composition_controller_->SendMouseInput(
        static_cast<COREWEBVIEW2_MOUSE_EVENT_KIND>(message),
        static_cast<COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS>(GET_KEYSTATE_WPARAM(wParam)), 0, point));
  }
}

ContainerVisual CompositionHost::FindVisual(POINT point) {
  ContainerVisual visual{nullptr};
  auto visuals = m_rootVisual.Children();
  for (auto v : visuals) {
    auto offset = v.Offset();
    auto size = v.Size();
    if ((point.x >= offset.x) && (point.x < offset.x + size.x) && (point.y >= offset.y) &&
        (point.y < offset.y + size.y)) {
      visual = v.as<ContainerVisual>();
    }
  }
  return visual;
}

void CompositionHost::CreateVisuals() {
  for (auto i = 0; i < kNumberOfVisuals; ++i) {
    // create webview visual in the middle with multiple overlays to better demonstrate
    // interaction with these visual objects
    if (i == kNumberOfVisuals / 2) {
      CreateWebViewVisual();
    } else {
      AddElement();
    }
  }
}

Color CompositionHost::RandomBlue() {
  uint8_t random = (double)(double)(rand() % 255);
  return ColorHelper::FromArgb(255, 50, 120, random);
}

void CompositionHost::SetBounds(RECT bounds) {
  m_appBounds = bounds;
  if (m_webViewVisual && m_rootVisual) {
    ResizeAllVisuals();
  }
}

void CompositionHost::SetWebViewVisualBounds() {
  RECT desiredBounds = m_appBounds;
  desiredBounds.top += LONG(m_webViewVisual.Offset().y);
  desiredBounds.left += LONG(m_webViewVisual.Offset().x);
  desiredBounds.bottom = LONG(m_webViewVisual.Size().y + desiredBounds.top);
  desiredBounds.right = LONG(m_webViewVisual.Size().x + desiredBounds.left);
  webview_controller_->put_Bounds(m_appBounds);
}

void CompositionHost::ResizeAllVisuals() {
  const float2 webViewSize = {((m_appBounds.right - m_appBounds.left) / 2.0f),
                              ((m_appBounds.bottom - m_appBounds.top) / 2.0f)};
  const float2 otherVisualSize = {webViewSize.x / 1.25f, webViewSize.y / 1.25f};

  auto visuals = m_rootVisual.Children();
  size_t index = 0;
  for (auto v : visuals) {
    const float2 updatedSize = v == m_webViewVisual ? webViewSize : otherVisualSize;
    // make sure index won't go out of kOffSetValues' bound
    const float2 offset = kOffSetValues[index % kNumberOfVisuals];
    v.Size(updatedSize);
    v.Offset({webViewSize.x * offset.x, webViewSize.y * offset.y, 0.0f});
    ++index;
  }
  SetWebViewVisualBounds();
}