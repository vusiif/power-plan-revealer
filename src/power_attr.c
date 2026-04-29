#include "power_attr.h"
#include <powrprof.h>

#ifndef POWER_ATTRIBUTE_HIDE
#define POWER_ATTRIBUTE_HIDE 0x00000001
#endif

int is_setting_hidden(const GUID *subgroup, const GUID *setting) {
    DWORD attr = PowerReadSettingAttributes(subgroup, setting);
    return (attr & POWER_ATTRIBUTE_HIDE) != 0;
}

DWORD unhide_setting(const GUID *subgroup, const GUID *setting) {
    /*
       0 = clear POWER_ATTRIBUTE_HIDE
       也就是显示这个电源设置。
    */
    return PowerWriteSettingAttributes(subgroup, setting, 0);
}

DWORD hide_setting(const GUID *subgroup, const GUID *setting) {
    return PowerWriteSettingAttributes(subgroup, setting, POWER_ATTRIBUTE_HIDE);
}
