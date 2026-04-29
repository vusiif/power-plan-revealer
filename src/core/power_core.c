#include "power_core.h"
#include "power_attr.h"

#include <windows.h>
#include <powrprof.h>
#include <objbase.h>

#include <stdio.h>
#include <wchar.h>

void guid_to_string(const GUID *g, wchar_t *out, size_t cch) {
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

int parse_guid_text(const wchar_t *text, GUID *out) {
    if (text == NULL || out == NULL) {
        return 0;
    }

    HRESULT hr = CLSIDFromString(text, out);

    if (SUCCEEDED(hr)) {
        return 1;
    }

    wchar_t wrapped[64];

    if (text[0] != L'{') {
        if (wcslen(text) + 3 > 64) {
            return 0;
        }

        swprintf_s(wrapped, 64, L"{%s}", text);

        hr = CLSIDFromString(wrapped, out);

        if (SUCCEEDED(hr)) {
            return 1;
        }
    }

    return 0;
}

DWORD get_active_scheme_name(
    wchar_t *out,
    DWORD out_cch
) {
    if (out == NULL || out_cch == 0) {
        return ERROR_INVALID_PARAMETER;
    }

    GUID *scheme = NULL;

    DWORD rc = PowerGetActiveScheme(NULL, &scheme);
    if (rc != ERROR_SUCCESS) {
        wcscpy_s(out, out_cch, L"(unknown)");
        return rc;
    }

    read_name(scheme, NULL, NULL, out, out_cch);

    LocalFree(scheme);
    return ERROR_SUCCESS;
}

DWORD refresh_active_scheme(void) {
    GUID *scheme = NULL;

    DWORD rc = PowerGetActiveScheme(NULL, &scheme);
    if (rc == ERROR_SUCCESS && scheme != NULL) {
        rc = PowerSetActiveScheme(NULL, scheme);
        LocalFree(scheme);
    }

    return rc;
}

int enumerate_power_settings(
    int hidden_only,
    PowerSettingCallback callback,
    void *user_data
) {
    if (callback == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    GUID *scheme = NULL;

    DWORD rc = PowerGetActiveScheme(NULL, &scheme);
    if (rc != ERROR_SUCCESS) {
        return (int)rc;
    }

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

        wchar_t subgroup_name[PPR_NAME_CCH];
        wchar_t subgroup_guid_text[PPR_GUID_CCH];

        read_name(
            scheme,
            &subgroup,
            NULL,
            subgroup_name,
            PPR_NAME_CCH
        );

        guid_to_string(
            &subgroup,
            subgroup_guid_text,
            PPR_GUID_CCH
        );

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

            PowerSettingItem item;
            ZeroMemory(&item, sizeof(item));

            item.subgroup_guid = subgroup;
            item.setting_guid = setting;
            item.hidden = hidden;

            wcscpy_s(
                item.subgroup_name,
                PPR_NAME_CCH,
                subgroup_name
            );

            wcscpy_s(
                item.subgroup_guid_text,
                PPR_GUID_CCH,
                subgroup_guid_text
            );

            read_name(
                scheme,
                &subgroup,
                &setting,
                item.setting_name,
                PPR_NAME_CCH
            );

            guid_to_string(
                &setting,
                item.setting_guid_text,
                PPR_GUID_CCH
            );

            if (callback(&item, user_data) != 0) {
                LocalFree(scheme);
                return ERROR_SUCCESS;
            }
        }
    }

    LocalFree(scheme);
    return ERROR_SUCCESS;
}
