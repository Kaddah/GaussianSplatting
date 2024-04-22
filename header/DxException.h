#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <stdexcept>
#include <exception>
#include <string>
#include <d3d12.h>
#include "d3dx12.h"
#include <windows.h>

// Function ANSI-String to std::wstring #MH
std::wstring AnsiToWString(const std::string& str);


// Specific DirectX Exception Class #MH
class DxException : public std::exception
{
public:
    HRESULT ErrorCode;
    std::wstring FunctionName;
    std::wstring Filename;
    int LineNumber;
    mutable std::string whatBuffer;

    DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber);

    virtual const char* what() const noexcept override;
};

// Macro to check HRESULT Values and throw exception #MH
#ifndef ThrowIfFailed
#define ThrowIfFailed(x) \
{ \
    HRESULT hr__ = (x); \
    std::wstring wfn = AnsiToWString(__FILE__); \
    if (FAILED(hr__)) { throw DxException(hr__, L#x, wfn, __LINE__); } \
}
#endif

#endif // EXCEPTION_H
