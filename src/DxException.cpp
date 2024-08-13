#include "DxException.h"
#include <comdef.h>
#include <iomanip>
#include <iostream>
#include <sstream>

std::wstring AnsiToWString(const std::string& str)
{
  if (str.empty())
    return std::wstring();
  int          size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
  std::wstring wstrTo(size_needed, 0);
  MultiByteToWideChar(CP_UTF8, 0, &str[0], size_needed, &wstrTo[0], size_needed);
  return wstrTo;
}

std::string WStringToAnsi(const std::wstring& wstr)
{
  if (wstr.empty())
    return std::string();
  int         size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
  std::string strTo(size_needed, 0);
  WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
  return strTo;
}

std::string GetErrorMessage(HRESULT hr)
{
  char* msgBuffer = nullptr;

  FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, hr,
                 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&msgBuffer, 0, NULL);

  std::string message = msgBuffer ? msgBuffer : "Unknown error code";
  LocalFree(msgBuffer);

  return message;
}

std::string GetErrorCodeName(HRESULT hr)
{
  _com_error err(hr);
  return WStringToAnsi(err.ErrorMessage());
}

DxException::DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber)
    : ErrorCode(hr)
    , FunctionName(functionName)
    , Filename(filename)
    , LineNumber(lineNumber)
{
  std::string errorMessage  = GetErrorMessage(hr);
  std::string errorCodeName = GetErrorCodeName(hr);

  std::wstringstream ws;
  ws << L"Error in: " << FunctionName << L"\n"
     << L"in file: " << Filename << L" (" << AnsiToWString(WStringToAnsi(Filename)) << L")\n"
     << L"at line: " << lineNumber << L"\n"
     << L"error code: " << hr << L" (0x" << std::hex << std::setw(8) << std::setfill(L'0') << hr << L")\n"
     << L"error message: " << AnsiToWString(errorMessage);

  std::wstring errorMsg = ws.str();
  std::wcerr << errorMsg << std::endl;
}

const char* DxException::what() const noexcept
{
  std::string errorMessage  = GetErrorMessage(ErrorCode);
  std::string errorCodeName = GetErrorCodeName(ErrorCode);

  std::wstringstream ws;
  ws << L"Error in: " << FunctionName << L"\n"
     << L"in file: " << Filename << L" (" << AnsiToWString(WStringToAnsi(Filename)) << L")\n"
     << L"at line: " << LineNumber << L"\n"
     << L"error code: " << ErrorCode << L" (0x" << std::hex << std::setw(8) << std::setfill(L'0') << ErrorCode << L")\n"
     << L"error message: " << AnsiToWString(errorMessage);

  std::wstring wwhatBuffer = ws.str();
  whatBuffer               = WStringToAnsi(wwhatBuffer);
  return whatBuffer.c_str();
}
