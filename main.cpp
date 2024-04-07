#include <iostream>
#include <chrono>
#include <thread>
#include <fmt/format.h>
#include <d3d12.h> // Für Direct3D12-Funktionalität
#include <dxgi1_4.h> // Für DXGI-Funktionalität (optional, aber oft verwendet zusammen mit Direct3D12)
#include <Windows.h> // Für Windows-spezifische Funktionalität (z. B. Fenstererstellung)

int main()
{
    int countdown = 9;


    fmt::print("Hello World!\n");
    for (int i = countdown; i > 0; --i) {
        std::cout << "\rClose Window in: " << i << "s" << std::flush;
        std::this_thread::sleep_for(std::chrono::seconds(1)); // Pause für 1 Sekunde
    }
    return 0;
}