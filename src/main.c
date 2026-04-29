#include "power_enum.h"

#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include <fcntl.h>
#include <io.h>
#include <locale.h>
#include "power_attr.h"
#include <objbase.h>


static void print_usage(void) {
    wprintf(L"Usage:\n");
    wprintf(L"  power_settings_tool.exe list\n");
    wprintf(L"  power_settings_tool.exe hidden\n");
    wprintf(L"  power_settings_tool.exe unhide-all\n");
}

static int parse_guid_arg(const wchar_t *text, GUID *out) {
    HRESULT hr = CLSIDFromString(text, out);

    if (FAILED(hr)) {
        fwprintf(stderr, L"Invalid GUID: %s\n", text);
        return 0;
    }

    return 1;
}

static void refresh_active_scheme(void) {
    GUID *scheme = NULL;

    DWORD rc = PowerGetActiveScheme(NULL, &scheme);
    if (rc == ERROR_SUCCESS && scheme != NULL) {
        PowerSetActiveScheme(NULL, scheme);
        LocalFree(scheme);
    }
}

static int command_unhide_or_hide(int hide, const wchar_t *subgroup_text, const wchar_t *setting_text) {
    GUID subgroup;
    GUID setting;

    if (!parse_guid_arg(subgroup_text, &subgroup)) {
        return 1;
    }

    if (!parse_guid_arg(setting_text, &setting)) {
        return 1;
    }

    DWORD before = get_setting_attributes(&subgroup, &setting);

    DWORD rc = hide
        ? hide_setting(&subgroup, &setting)
        : unhide_setting(&subgroup, &setting);

    DWORD after = get_setting_attributes(&subgroup, &setting);

    if (rc != ERROR_SUCCESS) {
        fwprintf(stderr, L"Failed, error: %lu\n", rc);
        return 1;
    }

    refresh_active_scheme();

    wprintf(L"OK\n");
    wprintf(L"Before attributes: 0x%08lX\n", before);
    wprintf(L"After  attributes: 0x%08lX\n", after);
    wprintf(L"Hidden now       : %s\n", is_setting_hidden(&subgroup, &setting) ? L"yes" : L"no");

    return 0;
}


int wmain(int argc, wchar_t **argv) {
    // SetConsoleOutputCP(CP_UTF8);
    setlocale(LC_ALL, "");

    _setmode(_fileno(stdout), _O_U16TEXT);
    _setmode(_fileno(stderr), _O_U16TEXT);

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

    if (wcscmp(argv[1], L"unhide") == 0) {
        if (argc != 4) {
            wprintf(L"Usage:\n");
            wprintf(L"  power_settings_tool.exe unhide <subgroup-guid> <setting-guid>\n");
            return 1;
        }

        return command_unhide_or_hide(0, argv[2], argv[3]);
    }

    if (wcscmp(argv[1], L"hide") == 0) {
        if (argc != 4) {
            wprintf(L"Usage:\n");
            wprintf(L"  power_settings_tool.exe hide <subgroup-guid> <setting-guid>\n");
            return 1;
        }

        return command_unhide_or_hide(1, argv[2], argv[3]);
    }

    print_usage();
    return 0;
}
