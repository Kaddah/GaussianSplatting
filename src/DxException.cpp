#include "DxException.h"
#include <iostream>

std::wstring AnsiToWString(const std::string& str)
{
  if (str.empty())
    return std::wstring();
  int          size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
  std::wstring wstrTo(size_needed, 0);
  MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
  return wstrTo;
}

DxException::DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber)
    : ErrorCode(hr)
    , FunctionName(functionName)
    , Filename(filename)
    , LineNumber(lineNumber)
{
  // Create the error message for console output
  std::string errorMsg = "Error in: " + std::string(FunctionName.begin(), FunctionName.end()) + "\n" + 
                         "in file: " + std::string(Filename.begin(), Filename.end()) + "\n" +
                         "at line: " + std::to_string(LineNumber) + "\n" + 
                         "with error code: " + std::to_string(hr);

  // Print the error message to the console
  std::cerr << errorMsg << std::endl;
}

const char* DxException::what() const noexcept
{
  whatBuffer = "Error in line: " + std::string(FunctionName.begin(), FunctionName.end()) + "\n" + 
               "in file: " + std::string(Filename.begin(), Filename.end()) + "\n" + 
               "at line: " + std::to_string(LineNumber) + "\n" +
               "with error code : " + std::to_string(ErrorCode);
  return whatBuffer.c_str();
}
