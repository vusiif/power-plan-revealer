#ifndef POWER_CORE_H
#define POWER_CORE_H

#include <windows.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PPR_NAME_CCH 256
#define PPR_GUID_CCH 64

typedef struct PowerSettingItem {
    GUID subgroup_guid;
    GUID setting_guid;

    wchar_t subgroup_name[PPR_NAME_CCH];
    wchar_t setting_name[PPR_NAME_CCH];

    wchar_t subgroup_guid_text[PPR_GUID_CCH];
    wchar_t setting_guid_text[PPR_GUID_CCH];

    int hidden;
} PowerSettingItem;

typedef int (*PowerSettingCallback)(
    const PowerSettingItem *item,
    void *user_data
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

#ifdef __cplusplus
}
#endif

#endif
