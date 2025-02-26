# Bombratter

Bombratter is a small game that was made with Salmon-Engine.
It is shelved for the time being. (It's gonna take too long to make)

# Getting started

To compile the project:

```bash
git clone -b bombratter https://github.com/TheSlugInTub/Salmon-Engine.git 
cd Salmon-Engine
mkdir build
cmake -S . -B build
cmake --build build --config Release 
```

Copy the 'res', 'shaders' folders and assimp DLL if you're on windows to the executable's directory.
Replace the 'mkdir' command with 'md' if you're on windows.
