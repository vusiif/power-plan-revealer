#include "power_core.h"
#include "power_attr.h"

#include <windows.h>

#include <stdio.h>
#include <wchar.h>
#include <fcntl.h>
#include <io.h>
#include <locale.h>

typedef struct PrintContext {
    int has_last_subgroup;
    wchar_t last_subgroup_guid[PPR_GUID_CCH];
} PrintContext;

static void print_usage(void) {
    wprintf(L"PowerPlanRevealer CLI\n");
    wprintf(L"Usage:\n");
    wprintf(L"  power-plan-revealer-cli.exe list\n");
    wprintf(L"      列出所有电源计划项目\n\n");

    wprintf(L"  power-plan-revealer-cli.exe hidden\n");
    wprintf(L"      列出所有隐藏的电源计划项目\n\n");

    wprintf(L"  power-plan-revealer-cli.exe unhide <subgroup-guid> <setting-guid>\n");
    wprintf(L"      取消隐藏单个项目 (需要管理员权限)\n\n");

    wprintf(L"  power-plan-revealer-cli.exe hide <subgroup-guid> <setting-guid>\n");
    wprintf(L"      隐藏单个项目 (需要管理员权限)\n\n");

    wprintf(L"  power-plan-revealer-cli.exe schemes\n");
    wprintf(L"      列出所有已安装的电源方案 (查看当前活动方案与可用方案)\n\n");

    wprintf(L"  power-plan-revealer-cli.exe set-scheme <scheme-guid>\n");
    wprintf(L"      切换当前活动的电源方案\n\n");

    wprintf(L"  power-plan-revealer-cli.exe backup [output-file.reg]\n");
    wprintf(L"      备份电源设置注册表至 .reg 文件 (默认保存为 PowerSettings_backup.reg)\n\n");

    wprintf(L"      注意：GUID 可带 {} 或不带 {}\n");
}

static int is_console_handle(DWORD std_handle_id) {
    HANDLE h = GetStdHandle(std_handle_id);
    DWORD mode = 0;

    if (h == NULL || h == INVALID_HANDLE_VALUE) {
        return 0;
    }

    return GetConsoleMode(h, &mode) != 0;
}

static void setup_output_encoding(void) {
    setlocale(LC_ALL, "");

    if (is_console_handle(STD_OUTPUT_HANDLE)) {
        _setmode(_fileno(stdout), _O_U16TEXT);
    } else {
        _setmode(_fileno(stdout), _O_U8TEXT);
        fputwc(0xFEFF, stdout);
    }

    if (is_console_handle(STD_ERROR_HANDLE)) {
        _setmode(_fileno(stderr), _O_U16TEXT);
    } else {
        _setmode(_fileno(stderr), _O_U8TEXT);
    }
}

static int print_setting_callback(
    const PowerSettingItem *item,
    void *user_data
) {
    PrintContext *ctx = (PrintContext *)user_data;

    if (
        !ctx->has_last_subgroup ||
        wcscmp(ctx->last_subgroup_guid, item->subgroup_guid_text) != 0
    ) {
        if (ctx->has_last_subgroup) {
            wprintf(L"\n");
        }

        wprintf(L"[%s]\n", item->subgroup_name);
        wprintf(L"  subgroup guid: %s\n", item->subgroup_guid_text);

        wcscpy_s(
            ctx->last_subgroup_guid,
            PPR_GUID_CCH,
            item->subgroup_guid_text
        );

        ctx->has_last_subgroup = 1;
    }

    wprintf(L"  - %s\n", item->setting_name);
    wprintf(L"    setting guid: %s\n", item->setting_guid_text);
    wprintf(L"    hidden      : %s\n", item->hidden ? L"yes" : L"no");
    if (item->has_ac_value) {
        wprintf(L"    AC value    : 0x%08lX (%lu)\n", item->ac_value, item->ac_value);
    }
    if (item->has_dc_value) {
        wprintf(L"    DC value    : 0x%08lX (%lu)\n", item->dc_value, item->dc_value);
    }
    if (item->description[0] != L'\0') {
        wprintf(L"    description : %s\n", item->description);
    }
    wprintf(L"\n");

    return 0;
}

static int command_list(int hidden_only) {
    wchar_t scheme_name[PPR_NAME_CCH];

    DWORD rc = get_active_scheme_name(
        scheme_name,
        PPR_NAME_CCH
    );

    if (rc != ERROR_SUCCESS) {
        fwprintf(stderr, L"PowerGetActiveScheme failed: %lu\n", rc);
        return 1;
    }

    wprintf(L"Active scheme: %s\n\n", scheme_name);

    PrintContext ctx;
    ZeroMemory(&ctx, sizeof(ctx));

    int result = enumerate_power_settings(
        hidden_only,
        print_setting_callback,
        &ctx
    );

    if (result != ERROR_SUCCESS) {
        fwprintf(stderr, L"enumerate_power_settings failed: %d\n", result);
        return 1;
    }

    if (ctx.has_last_subgroup) {
        wprintf(L"\n");
    }

    return 0;
}

static int command_unhide_or_hide(
    int hide,
    const wchar_t *subgroup_text,
    const wchar_t *setting_text
) {
    GUID subgroup;
    GUID setting;

    if (!parse_guid_text(subgroup_text, &subgroup)) {
        fwprintf(stderr, L"Invalid subgroup GUID: %s\n", subgroup_text);
        return 1;
    }

    if (!parse_guid_text(setting_text, &setting)) {
        fwprintf(stderr, L"Invalid setting GUID: %s\n", setting_text);
        return 1;
    }

    DWORD before = get_setting_attributes(&subgroup, &setting);

    DWORD rc = hide
        ? hide_setting(&subgroup, &setting)
        : unhide_setting(&subgroup, &setting);

    DWORD after = get_setting_attributes(&subgroup, &setting);

    if (rc != ERROR_SUCCESS) {
        fwprintf(stderr, L"操作失败，错误代码: %lu\n", rc);
        if (rc == ERROR_ACCESS_DENIED) {
            fwprintf(stderr, L"提示：拒绝访问。请以管理员身份运行此终端！\n");
        }
        return 1;
    }

    refresh_active_scheme();

    wprintf(L"OK\n");
    wprintf(L"Before attributes: 0x%08lX\n", before);
    wprintf(L"After  attributes: 0x%08lX\n", after);
    wprintf(
        L"Hidden now       : %s\n",
        is_setting_hidden(&subgroup, &setting) ? L"yes" : L"no"
    );

    return 0;
}

static int command_backup(const wchar_t *path) {
    wchar_t target[MAX_PATH];
    if (path != NULL && path[0] != L'\0') {
        wcscpy_s(target, MAX_PATH, path);
    } else {
        wcscpy_s(target, MAX_PATH, L"PowerSettings_backup.reg");
    }

    wprintf(L"正在导出注册表备份至: %s ...\n", target);
    int rc = backup_power_settings_registry(target);
    if (rc != 0) {
        fwprintf(stderr, L"备份失败，错误代码: %d\n", rc);
        if (rc == ERROR_ACCESS_DENIED) {
            fwprintf(stderr, L"提示：拒绝访问。请以管理员身份运行此终端！\n");
        }
        return 1;
    }

    wprintf(L"备份成功！已保存至: %s\n", target);
    return 0;
}

static int print_scheme_callback(const PowerSchemeItem *item, void *user_data) {
    (void)user_data;
    wprintf(
        L"%s [%s]\n",
        item->is_active ? L"*" : L" ",
        item->scheme_name
    );
    wprintf(L"    GUID  : %s\n", item->scheme_guid_text);
    wprintf(L"    Active: %s\n\n", item->is_active ? L"yes" : L"no");
    return 0;
}

static int command_schemes(void) {
    wprintf(L"系统已安装的电源计划方案 (* 表示当前活动方案)：\n\n");
    int rc = enumerate_power_schemes(print_scheme_callback, NULL);
    if (rc != ERROR_SUCCESS) {
        fwprintf(stderr, L"枚举电源方案失败: %d\n", rc);
        return 1;
    }
    return 0;
}

static int command_set_scheme(const wchar_t *guid_text) {
    GUID scheme;
    if (!parse_guid_text(guid_text, &scheme)) {
        fwprintf(stderr, L"无效的方案 GUID: %s\n", guid_text);
        return 1;
    }

    DWORD rc = set_active_scheme_by_guid(&scheme);
    if (rc != ERROR_SUCCESS) {
        fwprintf(stderr, L"切换电源方案失败，错误代码: %lu\n", rc);
        if (rc == ERROR_ACCESS_DENIED) {
            fwprintf(stderr, L"提示：拒绝访问。请以管理员身份运行此终端！\n");
        }
        return 1;
    }

    wchar_t scheme_name[PPR_NAME_CCH];
    get_active_scheme_name(scheme_name, PPR_NAME_CCH);
    wprintf(L"已成功激活电源方案: %s\n", scheme_name);
    return 0;
}

int wmain(int argc, wchar_t **argv) {
    setup_output_encoding();

    if (argc < 2) {
        print_usage();
        return 0;
    }

    if (wcscmp(argv[1], L"-h") == 0 ||
        wcscmp(argv[1], L"--help") == 0 ||
        wcscmp(argv[1], L"help") == 0 ||
        wcscmp(argv[1], L"/?") == 0) {
        print_usage();
        return 0;
    }

    if (wcscmp(argv[1], L"list") == 0) {
        return command_list(0);
    }

    if (wcscmp(argv[1], L"hidden") == 0) {
        return command_list(1);
    }

    if (wcscmp(argv[1], L"schemes") == 0 || wcscmp(argv[1], L"plans") == 0) {
        return command_schemes();
    }

    if (wcscmp(argv[1], L"set-scheme") == 0 || wcscmp(argv[1], L"set-plan") == 0) {
        if (argc != 3) {
            wprintf(L"Usage:\n");
            wprintf(L"  power-plan-revealer-cli.exe set-scheme <scheme-guid>\n");
            return 1;
        }

        return command_set_scheme(argv[2]);
    }

    if (wcscmp(argv[1], L"backup") == 0) {
        return command_backup(argc >= 3 ? argv[2] : NULL);
    }

    if (wcscmp(argv[1], L"unhide") == 0) {
        if (argc != 4) {
            wprintf(L"Usage:\n");
            wprintf(L"  power-plan-revealer-cli.exe unhide <subgroup-guid> <setting-guid>\n");
            return 1;
        }

        return command_unhide_or_hide(0, argv[2], argv[3]);
    }

    if (wcscmp(argv[1], L"hide") == 0) {
        if (argc != 4) {
            wprintf(L"Usage:\n");
            wprintf(L"  power-plan-revealer-cli.exe hide <subgroup-guid> <setting-guid>\n");
            return 1;
        }

        return command_unhide_or_hide(1, argv[2], argv[3]);
    }

    print_usage();
    return 0;
}
