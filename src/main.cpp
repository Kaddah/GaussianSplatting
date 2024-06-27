#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3dcompiler.lib")


#include <iostream>
#include "DxException.h"
#include "GaussianRenderer.h"
#include <PlyReader.h>
#include "Vertex.h"

std::vector<Vertex> vertices;
std::vector<Vertex> quads;
//GaussianRenderer window;

void attachConsole()
{
	AllocConsole();

	FILE *fpStdin;
	freopen_s(&fpStdin, "CONIN$", "r", stdin);
	std::cin.clear();

	FILE *fpStdout;
	freopen_s(&fpStdout, "CONOUT$", "w", stdout);
	std::cout.clear();

	FILE *fpStderr;
	freopen_s(&fpStderr, "CONOUT$", "w", stderr);
	std::cerr.clear();

	std::wcin.clear();
	std::wcout.clear();
	std::wcerr.clear();
	std::wclog.clear();

	std::ios::sync_with_stdio(true);

	SetConsoleTitle(L"Dreieck Console");
	std::cout << "Program start" << std::endl;
}

// Simulierte Funktion, die HRESULT zur�ckgibt
HRESULT SimulateDirectXFunction()
{
	// Hier simulieren wir einen Fehler
	return E_FAIL; // Simuliere einen Fehlschlag
}

// main methode
int WINAPI WinMain(HINSTANCE hInstance,
				   HINSTANCE hPrevInstance,
				   LPSTR lpCmdLine,
				   int nShowCmd)

{
	attachConsole();
	// TESTING EXCEPTION WORKING - MH
	try
	{
		// Testen der DirectX-Funktion mit dem ThrowIfFailed Makro
		// ThrowIfFailed(SimulateDirectXFunction());
	}
	catch (const DxException &e)
	{
		// Fehlermeldung in einer MessageBox anzeigen
		MessageBoxA(NULL, e.what(), "Exception Caught", MB_ICONERROR);
	}


	// #10 start to import PLY file - MH
	//std::string plyFilename = "../triangle-data-test.ply";
	//std::string plyFilename = "../bycicle-test.ply";
	//std::string plyFilename = "../assets/file.ply";
	
	//default if exe get no argument
	std::string plyFilename = "../assets/file.ply";
	if (strlen(lpCmdLine) > 0)
	{
		// set path to given argument (if run in Visual Studio the argument is set by CMake File to "../assets/file.ply")
		plyFilename = lpCmdLine;
	}

	vertices = PlyReader::readPlyFile(plyFilename);

	try
	{
		GaussianRenderer window (L"Triangle", 800, 600, false, hInstance, nShowCmd, vertices, quads);
		// start the main loop
		window.mainloop();
		
	}
	catch (std::exception)
	{
		MessageBox(0, L"Window Initialization - Failed",
				   L"Error", MB_OK);
	}

	return 0;
}
