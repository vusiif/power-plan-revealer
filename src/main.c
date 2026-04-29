#include "power_enum.h"

#include <windows.h>
#include <stdio.h>
#include <wchar.h>

static void print_usage(void) {
    wprintf(L"Usage:\n");
    wprintf(L"  power_settings_tool.exe list\n");
    wprintf(L"  power_settings_tool.exe hidden\n");
    wprintf(L"  power_settings_tool.exe unhide-all\n");
}

int wmain(int argc, wchar_t **argv) {
    SetConsoleOutputCP(CP_UTF8);

    if (argc < 2) {
        print_usage();
        return 0;
    }

    if (wcscmp(argv[1], L"list") == 0) {
        return list_power_settings(0, 0);
    }

    if (wcscmp(argv[1], L"hidden") == 0) {
        return list_power_settings(1, 0);
    }

    if (wcscmp(argv[1], L"unhide-all") == 0) {
        return list_power_settings(1, 1);
    }

    print_usage();
    return 0;
}
