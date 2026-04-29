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
    wprintf(L"Usage:\n");
    wprintf(L"  power-plan-revealer-cli.exe list\n");
    wprintf(L"      列出所有电源计划项目\n\n");

    wprintf(L"  power-plan-revealer-cli.exe hidden\n");
    wprintf(L"      列出所有隐藏的电源计划项目\n\n");

    wprintf(L"  power-plan-revealer-cli.exe unhide <subgroup-guid> <setting-guid>\n");
    wprintf(L"      取消隐藏单个项目\n\n");

    wprintf(L"  power-plan-revealer-cli.exe hide <subgroup-guid> <setting-guid>\n");
    wprintf(L"      隐藏单个项目\n\n");

    wprintf(L"      注意，所有 guid 可以带 {}，也可以不带 {}\n");
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
        fwprintf(stderr, L"Failed, error: %lu\n", rc);
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

int wmain(int argc, wchar_t **argv) {
    setup_output_encoding();

    if (argc < 2) {
        print_usage();
        return 0;
    }

    if (wcscmp(argv[1], L"list") == 0) {
        return command_list(0);
    }

    if (wcscmp(argv[1], L"hidden") == 0) {
        return command_list(1);
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
