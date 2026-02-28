#include "PGR/Application/Application.h"

PGR::Application App;

void CtrlC() {
    std::cout << "\n\n\r";
    LogNotice("Ctrl-C", 0);
}

BOOL WINAPI ConsoleHandler(DWORD dwCtrlType) {
    switch (dwCtrlType) {
    case CTRL_C_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_BREAK_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
        Exit("Console closed", 0);
        break;
    default:
        return FALSE;
    }
    return TRUE;
}

int main(int argc, char** argv) {
    system("CHCP 65001 > nul 2>&1");

    if (!SetConsoleCtrlHandler(ConsoleHandler, TRUE)) Exit("Failed to set console control handler", 1);

    char workDir[MAX_PATH];
    if (!_getcwd(workDir, MAX_PATH))
        Exit("Failed to get working directory", 1);

    char exeFullPath[MAX_PATH] = { 0 };
    DWORD pathLen = GetModuleFileNameA(NULL, exeFullPath, MAX_PATH);

    if (pathLen == 0 || pathLen >= MAX_PATH)
        Exit("Failed to get executable path", 1);

    if (!PathRemoveFileSpecA(exeFullPath))
        Exit("Failed to remove file spec from executable path", 1);

    if (!SetCurrentDirectoryA(exeFullPath))
        Exit("Failed to set current directory to executable path", 1);

    while (true) {
        if (!_chdir(".\\resources"))
            break;
        if (_chdir(".."))
            Exit("Failed to set current directory to resources", 1);
    }

    char ResDir[MAX_PATH];
    if (!_getcwd(ResDir, MAX_PATH))
        Exit("Failed to get resources directory", 1);

    App = PGR::Application(argc, argv, workDir, ResDir);
    App.Run();

    LogNotice("Exiting application");
    return 0;
}
