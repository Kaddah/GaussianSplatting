#include "DxException.h"

std::wstring AnsiToWString(const std::string& str) {
    if (str.empty()) return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

DxException::DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber)
    : ErrorCode(hr), FunctionName(functionName), Filename(filename), LineNumber(lineNumber) {}

const char* DxException::what() const noexcept {
    whatBuffer = "Error in " + std::string(FunctionName.begin(), FunctionName.end()) +
        " in file " + std::string(Filename.begin(), Filename.end()) +
        " at line " + std::to_string(LineNumber);
    return whatBuffer.c_str();
}