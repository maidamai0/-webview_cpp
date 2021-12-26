#include "CheckFailure.h"

#include <iomanip>
#include <sstream>

// Notify the user of a failure with a message box.
void ShowFailure(HRESULT hr, const std::wstring& message) {
  std::wstringstream formated_message;
  formated_message << message << ": 0x" << std::hex << std::setw(8) << hr;
  MessageBox(nullptr, formated_message.str().c_str(), nullptr, MB_OK);
}

// If something failed, show the error code and fail fast.
void CheckFailure(HRESULT hr, const std::wstring& message) {
  if (FAILED(hr)) {
    ShowFailure(hr, message);
    FAIL_FAST();
  }
}
