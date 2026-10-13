#include "power_core.h"
#include "power_attr.h"

#include <windows.h>
#include <powrprof.h>
#include <objbase.h>
#include <shellapi.h>

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

static void read_description(
    const GUID *scheme,
    const GUID *subgroup,
    const GUID *setting,
    wchar_t *out,
    DWORD out_cch
) {
    DWORD bytes = out_cch * sizeof(wchar_t);
    out[0] = L'\0';

    DWORD rc = PowerReadDescription(
        NULL,
        scheme,
        subgroup,
        setting,
        (PUCHAR)out,
        &bytes
    );

    if (rc != ERROR_SUCCESS) {
        out[0] = L'\0';
    }
}

static void read_setting_values(
    const GUID *scheme,
    const GUID *subgroup,
    const GUID *setting,
    int *has_ac,
    DWORD *ac_val,
    int *has_dc,
    DWORD *dc_val
) {
    *has_ac = 0;
    *ac_val = 0;
    *has_dc = 0;
    *dc_val = 0;

    DWORD val = 0;
    DWORD rc = PowerReadACValueIndex(NULL, scheme, subgroup, setting, &val);
    if (rc == ERROR_SUCCESS) {
        *has_ac = 1;
        *ac_val = val;
    }

    val = 0;
    rc = PowerReadDCValueIndex(NULL, scheme, subgroup, setting, &val);
    if (rc == ERROR_SUCCESS) {
        *has_dc = 1;
        *dc_val = val;
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

            read_description(
                scheme,
                &subgroup,
                &setting,
                item.description,
                PPR_DESC_CCH
            );

            read_setting_values(
                scheme,
                &subgroup,
                &setting,
                &item.has_ac_value,
                &item.ac_value,
                &item.has_dc_value,
                &item.dc_value
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

int enumerate_power_schemes(
    PowerSchemeCallback callback,
    void *user_data
) {
    if (callback == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    GUID *active_scheme = NULL;
    DWORD active_rc = PowerGetActiveScheme(NULL, &active_scheme);

    for (DWORD index = 0; ; index++) {
        GUID scheme;
        DWORD size = sizeof(scheme);

        DWORD rc = PowerEnumerate(
            NULL,
            NULL,
            NULL,
            ACCESS_SCHEME,
            index,
            (PUCHAR)&scheme,
            &size
        );

        if (rc == ERROR_NO_MORE_ITEMS) {
            break;
        }

        if (rc != ERROR_SUCCESS) {
            continue;
        }

        PowerSchemeItem item;
        ZeroMemory(&item, sizeof(item));
        item.scheme_guid = scheme;

        guid_to_string(&scheme, item.scheme_guid_text, PPR_GUID_CCH);
        read_name(&scheme, NULL, NULL, item.scheme_name, PPR_NAME_CCH);

        if (active_rc == ERROR_SUCCESS && active_scheme != NULL &&
            IsEqualGUID(&scheme, active_scheme)) {
            item.is_active = 1;
        } else {
            item.is_active = 0;
        }

        if (callback(&item, user_data) != 0) {
            if (active_scheme != NULL) {
                LocalFree(active_scheme);
            }
            return ERROR_SUCCESS;
        }
    }

    if (active_scheme != NULL) {
        LocalFree(active_scheme);
    }
    return ERROR_SUCCESS;
}

DWORD set_active_scheme_by_guid(const GUID *scheme_guid) {
    if (scheme_guid == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    return PowerSetActiveScheme(NULL, scheme_guid);
}

int open_in_regedit(
    const wchar_t *subgroup_guid_text,
    const wchar_t *setting_guid_text
) {
    wchar_t reg_path[1024];
    if (setting_guid_text != NULL && setting_guid_text[0] != L'\0') {
        swprintf_s(
            reg_path,
            1024,
            L"Computer\\HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Power\\PowerSettings\\%s\\%s",
            subgroup_guid_text,
            setting_guid_text
        );
    } else {
        swprintf_s(
            reg_path,
            1024,
            L"Computer\\HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Power\\PowerSettings\\%s",
            subgroup_guid_text
        );
    }

    HKEY hKey = NULL;
    LSTATUS status = RegOpenKeyExW(
        HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Applets\\Regedit",
        0,
        KEY_SET_VALUE,
        &hKey
    );

    if (status == ERROR_SUCCESS) {
        RegSetValueExW(
            hKey,
            L"LastKey",
            0,
            REG_SZ,
            (const BYTE *)reg_path,
            (DWORD)((wcslen(reg_path) + 1) * sizeof(wchar_t))
        );
        RegCloseKey(hKey);
    }

    HINSTANCE hInst = ShellExecuteW(
        NULL,
        L"open",
        L"regedit.exe",
        NULL,
        NULL,
        SW_SHOWNORMAL
    );

    return ((INT_PTR)hInst > 32) ? 0 : (int)GetLastError();
}

int backup_power_settings_registry(const wchar_t *export_file_path) {
    if (export_file_path == NULL || export_file_path[0] == L'\0') {
        return ERROR_INVALID_PARAMETER;
    }

    wchar_t cmdline[2048];
    swprintf_s(
        cmdline,
        2048,
        L"reg.exe export \"HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Power\\PowerSettings\" \"%s\" /y",
        export_file_path
    );

    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcessW(
        NULL,
        cmdline,
        NULL,
        NULL,
        FALSE,
        CREATE_NO_WINDOW,
        NULL,
        NULL,
        &si,
        &pi
    )) {
        return (int)GetLastError();
    }

    WaitForSingleObject(pi.hProcess, 10000);

    DWORD exit_code = 0;
    GetExitCodeProcess(pi.hProcess, &exit_code);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return (int)exit_code;
}
