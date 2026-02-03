#include "PGR/Application/Application.h"

#include <Windows.h>
#include <string>
#include <iostream>

int main(int argc, char** argv) {

    system("CHCP 65001");

    char workDir[MAX_PATH];
    if (!_getcwd(workDir, MAX_PATH))
        Exit("Failed to get working directory", 1);

    char exeFullPath[MAX_PATH] = { 0 };
    DWORD pathLen = GetModuleFileNameA(NULL, exeFullPath, MAX_PATH);

    if (pathLen == 0 || pathLen >= MAX_PATH)
        return false;

    if (!PathRemoveFileSpecA(exeFullPath))
        return false;

    if (!SetCurrentDirectoryA(exeFullPath))
        return false;

    while (true) {
        if (!_chdir(".\\resources"))
            break;
        if (_chdir(".."))
            exit(1);
    }

    char ResDir[MAX_PATH];
    if (!_getcwd(ResDir, MAX_PATH))
        Exit("Failed to get resources directory", 1);

    PGR::Application App(argc, argv, workDir, ResDir);

    App.Run();

    return 0;
}
