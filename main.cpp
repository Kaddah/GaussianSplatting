#include <iostream>
#include <chrono>
#include <thread>
#include <fmt/format.h>
#include <d3d12.h> // F�r Direct3D12-Funktionalit�t
#include <dxgi1_4.h> // F�r DXGI-Funktionalit�t (optional, aber oft verwendet zusammen mit Direct3D12)
#include <Windows.h> // F�r Windows-spezifische Funktionalit�t (z. B. Fenstererstellung)

int main()
{
    int countdown = 9;


    fmt::print("Hello World!\n");
    for (int i = countdown; i > 0; --i) {
        std::cout << "\rClose Window in: " << i << "s" << std::flush;
        std::this_thread::sleep_for(std::chrono::seconds(1)); // Pause f�r 1 Sekunde
    }
    return 0;
}