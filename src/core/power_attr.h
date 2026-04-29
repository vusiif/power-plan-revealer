#ifndef POWER_ATTR_H
#define POWER_ATTR_H

#include <windows.h>

DWORD get_setting_attributes(const GUID *subgroup, const GUID *setting);
int is_setting_hidden(const GUID *subgroup, const GUID *setting);
DWORD unhide_setting(const GUID *subgroup, const GUID *setting);
DWORD hide_setting(const GUID *subgroup, const GUID *setting);

#endif
