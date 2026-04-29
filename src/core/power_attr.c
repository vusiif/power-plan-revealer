#include "power_attr.h"

#include <powrprof.h>

#ifndef POWER_ATTRIBUTE_HIDE
#define POWER_ATTRIBUTE_HIDE 0x00000001
#endif

DWORD get_setting_attributes(const GUID *subgroup, const GUID *setting) {
    return PowerReadSettingAttributes(subgroup, setting);
}

int is_setting_hidden(const GUID *subgroup, const GUID *setting) {
    DWORD attr = get_setting_attributes(subgroup, setting);
    return (attr & POWER_ATTRIBUTE_HIDE) != 0;
}

DWORD unhide_setting(const GUID *subgroup, const GUID *setting) {
    DWORD attr = get_setting_attributes(subgroup, setting);
    attr &= ~POWER_ATTRIBUTE_HIDE;
    return PowerWriteSettingAttributes(subgroup, setting, attr);
}

DWORD hide_setting(const GUID *subgroup, const GUID *setting) {
    DWORD attr = get_setting_attributes(subgroup, setting);
    attr |= POWER_ATTRIBUTE_HIDE;
    return PowerWriteSettingAttributes(subgroup, setting, attr);
}
