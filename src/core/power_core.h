#ifndef POWER_CORE_H
#define POWER_CORE_H

#include <windows.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PPR_NAME_CCH 256
#define PPR_GUID_CCH 64
#define PPR_DESC_CCH 1024

typedef struct PowerSettingItem {
    GUID subgroup_guid;
    GUID setting_guid;

    wchar_t subgroup_name[PPR_NAME_CCH];
    wchar_t setting_name[PPR_NAME_CCH];
    wchar_t description[PPR_DESC_CCH];

    wchar_t subgroup_guid_text[PPR_GUID_CCH];
    wchar_t setting_guid_text[PPR_GUID_CCH];

    int hidden;

    int has_ac_value;
    DWORD ac_value;

    int has_dc_value;
    DWORD dc_value;
} PowerSettingItem;

typedef struct PowerSchemeItem {
    GUID scheme_guid;
    wchar_t scheme_name[PPR_NAME_CCH];
    wchar_t scheme_guid_text[PPR_GUID_CCH];
    int is_active;
} PowerSchemeItem;

typedef int (*PowerSettingCallback)(
    const PowerSettingItem *item,
    void *user_data
);

typedef int (*PowerSchemeCallback)(
    const PowerSchemeItem *item,
    void *user_data
);

int enumerate_power_schemes(
    PowerSchemeCallback callback,
    void *user_data
);

DWORD set_active_scheme_by_guid(const GUID *scheme_guid);

int open_in_regedit(
    const wchar_t *subgroup_guid_text,
    const wchar_t *setting_guid_text
);

int enumerate_power_settings(
    int hidden_only,
    PowerSettingCallback callback,
    void *user_data
);

DWORD get_active_scheme_name(
    wchar_t *out,
    DWORD out_cch
);

void guid_to_string(
    const GUID *g,
    wchar_t *out,
    size_t cch
);

int parse_guid_text(
    const wchar_t *text,
    GUID *out
);

DWORD refresh_active_scheme(void);

int backup_power_settings_registry(const wchar_t *export_file_path);

#ifdef __cplusplus
}
#endif

#endif
