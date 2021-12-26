#pragma once

#include <Windows.h>

#include <filesystem>
#include <string>

namespace common {

inline auto ws2s(const std::wstring& ws) {
  const auto size = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, nullptr, 0, nullptr, nullptr);
  std::string s(size, 0);
  WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, &s[0], size, nullptr, nullptr);
  return s;
}

inline auto s2ws(const std::string& s) {
  const auto size = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
  std::wstring ws(size, 0);
  MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &ws[0], size);
  return ws;
}

inline auto exe_path() {
  TCHAR exe_path[MAX_PATH];
  GetModuleFileName(nullptr, exe_path, MAX_PATH);
  std::filesystem::path path(exe_path);
  return path.parent_path();
}

inline auto resoruce_path(std::string name) {
  return exe_path() / name;
}
}  // namespace common