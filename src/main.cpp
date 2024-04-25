#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <Windows.h>
#include <initguid.h>
#include <imgui.h>
#include <wrl/client.h>
#include <iostream>

#include "vector.h"
#include "matrix.h"
#include "DxException.h"
#include "Window.h"


void attachConsole() {
    AllocConsole();

    FILE* fpStdin;
    freopen_s(&fpStdin, "CONIN$", "r", stdin);
    std::cin.clear();

    FILE* fpStdout;
    freopen_s(&fpStdout, "CONOUT$", "w", stdout);
    std::cout.clear();

    FILE* fpStderr;
    freopen_s(&fpStderr, "CONOUT$", "w", stderr);
    std::cerr.clear();

    std::wcin.clear();
    std::wcout.clear();
    std::wcerr.clear();
    std::wclog.clear();

    std::ios::sync_with_stdio(true);

    SetConsoleTitle(L"Dreieck Console");
    std::cout << "Hello World" << std::endl;
}
//


// Simulierte Funktion, die HRESULT zurückgibt
HRESULT SimulateDirectXFunction() {
    // Hier simulieren wir einen Fehler
    return E_FAIL;  // Simuliere einen Fehlschlag
}

// entry point
//main methode
int WINAPI WinMain(HINSTANCE hInstance, // Main windows function
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nShowCmd)

{
    attachConsole();
    //TESTING EXCEPTION WORKING - MH
     try {
        // Testen der DirectX-Funktion mit dem ThrowIfFailed Makro
    ThrowIfFailed(SimulateDirectXFunction());
        }
        catch (const DxException& e) {
    // Fehlermeldung in einer MessageBox anzeigen
    MessageBoxA(NULL, e.what(), "Exception Caught", MB_ICONERROR);
    }


       // Window window(L"Triangle",800, 600, false );
        //catch MessageBox(0, L"Window Initialization - Failed",
       // L"Error", MB_OK);
       
    
    
    // start the main loop
    mainloop();
    // wait for gpu to finish executing the command list before we start releasing everything
    WaitForPreviousFrame();
    // close the fence event
    CloseHandle(fenceEvent);
    // clean up everything

    return 0;
}
// create and show the window
//hier war mal initializeWindow

void mainloop()
{
    MSG msg;
    ZeroMemory(&msg, sizeof(MSG));

    while (_running)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                break;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            // run game code
            Update(); // update the game logic
            Render(); // execute the command queue (rendering the scene is the result of the gpu executing the command lists)
        }
    }
}



