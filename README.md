# ToDo
```diff
- Bitte Repo Clonen und dieser README folgen.
```
1. CMake installieren
2. vcpkg installieren

# CMake
## CMake installieren
1. Öffne cmd (als admin)
```
pip install cmake
```
2. Prüfe ob alles funktioniert
```
cmake --version
```
Nun sollte auf der Konsole die aktuelle CMake version (3.29.0) angezeigt wird.

In der Datei "CMakeList.txt" ist hinterlegt das ältere CMake Versionen als v3.29.0 nicht Aktzepiert werden. 


## Visual Studio Projekt erstellen

1. Öffne cmd
2. Navigiere zum Projekt (zum Beispiel: D:\MyWorspace\GaussianSplatting)
```
cd D:\MyWorspace\GaussianSplatting
```
3. Erstelle einen neuen Ordner in welchen das Projekt Erstellt wird (zb. build)
```
mkdir build
```
5. Navigiere in den neuen Ordner
```
cd build
```
6. Erstelle hier das Projekt auf der Basis der Dateien im Überordner (im überordner muss CMakeLists.txt und main.cpp liegen)
```
cmake ..
```


## Projekt Compilieren

1. Navigiere zum Projekt
```
cd D:\MyWorspace\GaussianSplatting
```
2. Erstelle eine ausführbare datei
```
cmake --build build
```		
Die .exe Datei ist nun in GaussianSplatting/build/Debug/Gaussian-Splatting.exe zu finden

# vcpkg
## vcpkg installieren
1. Clone das Repo von Github (zum Beispiel neben das Repo "GaussianSplatting)
```
git clone https://github.com/microsoft/vcpkg.git
```
2. Navigiere in das Repo unf führe das bootstrap-vcpkg.bat file aus
```
bootstrap-vcpkg.bat
```
3. Teste ob vcpkg funktioniert
```
./vcpkg version
```

## vcpkg verwenden
im Repo liegt eine `vcpkg.json` Datei hier können die nötigen Dependencies in die liste mit aufgenommen werden. Zusätzlich müssen diese wie auf der Webseite beschrieben in das vcpkg Repo installiert werden.

Dependencies können hier gesucht werden: https://vcpkg.io/en/packages
