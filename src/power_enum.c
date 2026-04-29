#include "power_enum.h"
#include "power_attr.h"

#include <windows.h>
#include <powrprof.h>
#include <stdio.h>

static void guid_to_string(const GUID *g, wchar_t *out, size_t cch) {
    swprintf(
        out,
        cch,
        L"%08lX-%04hX-%04hX-%02X%02X-%02X%02X%02X%02X%02X%02X",
        g->Data1,
        g->Data2,
        g->Data3,
        g->Data4[0],
        g->Data4[1],
        g->Data4[2],
        g->Data4[3],
        g->Data4[4],
        g->Data4[5],
        g->Data4[6],
        g->Data4[7]
    );
}

static void read_name(
    const GUID *scheme,
    const GUID *subgroup,
    const GUID *setting,
    wchar_t *out,
    DWORD out_cch
) {
    DWORD bytes = out_cch * sizeof(wchar_t);
    out[0] = L'\0';

    DWORD rc = PowerReadFriendlyName(
        NULL,
        scheme,
        subgroup,
        setting,
        (PUCHAR)out,
        &bytes
    );

    if (rc != ERROR_SUCCESS || out[0] == L'\0') {
        wcscpy_s(out, out_cch, L"(no name)");
    }
}

int list_power_settings(int hidden_only) {
    GUID *scheme = NULL;
    DWORD rc = PowerGetActiveScheme(NULL, &scheme);

    if (rc != ERROR_SUCCESS) {
        fwprintf(stderr, L"PowerGetActiveScheme failed: %lu\n", rc);
        return 1;
    }

    wchar_t scheme_name[256];
    read_name(scheme, NULL, NULL, scheme_name, 256);
    wprintf(L"Active scheme: %s\n\n", scheme_name);

    for (DWORD subgroup_index = 0; ; subgroup_index++) {
        GUID subgroup;
        DWORD size = sizeof(subgroup);

        rc = PowerEnumerate(
            NULL,
            scheme,
            NULL,
            ACCESS_SUBGROUP,
            subgroup_index,
            (PUCHAR)&subgroup,
            &size
        );

        if (rc == ERROR_NO_MORE_ITEMS) {
            break;
        }

        if (rc != ERROR_SUCCESS) {
            continue;
        }

        wchar_t subgroup_name[256];
        wchar_t subgroup_guid[64];

        read_name(scheme, &subgroup, NULL, subgroup_name, 256);
        guid_to_string(&subgroup, subgroup_guid, 64);

        int printed_group = 0;

        for (DWORD setting_index = 0; ; setting_index++) {
            GUID setting;
            size = sizeof(setting);

            rc = PowerEnumerate(
                NULL,
                scheme,
                &subgroup,
                ACCESS_INDIVIDUAL_SETTING,
                setting_index,
                (PUCHAR)&setting,
                &size
            );

            if (rc == ERROR_NO_MORE_ITEMS) {
                break;
            }

            if (rc != ERROR_SUCCESS) {
                continue;
            }

            int hidden = is_setting_hidden(&subgroup, &setting);

            if (hidden_only && !hidden) {
                continue;
            }

            if (!printed_group) {
                wprintf(L"[%s]\n", subgroup_name);
                wprintf(L"  subgroup guid: %s\n", subgroup_guid);
                printed_group = 1;
            }

            wchar_t setting_name[256];
            wchar_t setting_guid[64];

            read_name(scheme, &subgroup, &setting, setting_name, 256);
            guid_to_string(&setting, setting_guid, 64);

            wprintf(L"  - %s\n", setting_name);
            wprintf(L"    setting guid: %s\n", setting_guid);
            wprintf(L"    hidden      : %s\n", hidden ? L"yes" : L"no");
            wprintf(L"\n");
        }

        if (printed_group) {
            wprintf(L"\n");
        }
    }
    LocalFree(scheme);
    return 0;
}
