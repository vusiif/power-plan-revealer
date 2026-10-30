# PowerPlanRevealer

<div align="center">

![PowerPlanRevealer Banner](resources/app.png)

**安全、现代、纯粹的 Windows 电源计划隐藏高级选项管理器**

[![License: LGPL-2.1](https://img.shields.io/badge/License-LGPL--2.1-blue.svg)](LICENCE)
[![Platform: Windows 10 / 11](https://img.shields.io/badge/Platform-Windows%2010%20%7C%2011%20(x64)-0078D6.svg?logo=windows)](https://github.com/vusiif/power-plan-revealer)
[![Qt: 6.x](https://img.shields.io/badge/Qt-6.x-41CD52.svg?logo=qt)](https://www.qt.io/)
[![Release: v0.3.0](https://img.shields.io/badge/Release-v0.3.0-brightgreen.svg)](https://github.com/vusiif/power-plan-revealer/releases)

[下载最新版本](https://github.com/vusiif/power-plan-revealer/releases) • [主要功能](#-主要特性) • [场景预设](#-一键场景预设包) • [CLI用法](#️-命令行工具-cli-用法) • [构建指南](#-构建与编译指南)

</div>

---

## 💡 为什么需要 PowerPlanRevealer？

Windows 电源管理体系极其精密，包含数百个关于 CPU 调度策略、核心停车 (Core Parking)、C-States 深度休眠、PCIe 链路电源管理 (ASPM)、异构架构性能调度等关键选项。

然而，微软在控制面板中**隐藏了 90% 以上的高级设置**。很多极客与性能调优玩家被迫在网上到处寻找并运行来源不明的 `.reg` 注册表脚本，存在误改其他注册表项的风险；而在 Windows 11 中，传统控制面板被进一步收紧，寻找入口繁琐。

**PowerPlanRevealer 坚守“只解隐，不改值”的底线**：
1. **纯粹安全**：严格只修改配置项的 `Attributes` 显隐属性，**绝不私自篡改您的任何电压、频率或调度参数数值**。
2. **操作闭环**：一键解锁隐藏项，并提供“直达高级电源设置”按钮唤出原生控制面板供用户微调。
3. **退路无忧**：支持一键导出原生 `.reg` 备份，并内置了“恢复系统默认显隐”硬编码逻辑，随时一键复原。

---

## ✨ 主要特性

- **双版本支持**：
  - **现代化图形界面 (GUI)**：基于 Qt 6 Widgets 构建，支持 Windows 11 原生深色/浅色自适应、沉浸式卡片布局与平滑交互。
  - **命令行工具 (CLI)**：轻量无外部依赖，输出对齐美观，适合运维自动化与批处理脚本。
- **🎮 场景化一键预设包 (Profiles & Presets)**：
  - **电竞低延迟包**：一键解锁 CPU 核心停车 (Core Parking)、处理器性能提升激进模式、PCIe 节能优化等关键调优选项。
  - **笔电极限续航包**：解锁动态刷新率、最大睿频封顶、WiFi 节能模式、AHCI 节能等。
  - **音频/DAW 生产力包**：针对专业音频宿主与防 DPC 延迟突发解锁调度选项。
  - **恢复出厂显隐状态**：内置微软原厂出厂白名单，一键恢复默认的仅公开少数基础项，不影响数值。
- **🚀 原生工作流闭环**：
  - 界面直通 **“⚙ 打开高级电源设置”**（调用 `control.exe powercfg.cpl,,1`），解锁后立刻弹出原生调参窗口，告别繁琐的层层寻找。
- **深度信息获取与全景数值查看 (Value Inspector)**：
  - 调用底层的 `PowerReadDescription` 读取微软官方详尽中文释义。
  - 实时读取并在悬浮卡片中对比“接通电源 (AC)”与“使用电池 (DC)”下的当前设定值（十六进制与十进制双显）。
- **安全第一的设计保障**：
  - **一键备份注册表**：一键将 `PowerSettings` 注册表分支无损导出为标准 `.reg` 备份文件。
  - **双重校验与即时生效**：修改属性后自动调用 `PowerSetActiveScheme` 刷新当前激活方案，即时生效无需注销或重启。
- **智能搜索与语义别名**：
  - 支持按分类名、设置项名称、GUID、说明关键字即时过滤。
  - 支持常用中文通俗词与状态别名搜索（如输入“防降频”、“调度”、“休眠”、“隐藏”、“可见”）。
- **注册表编辑器一键联动**：
  - 支持右键“在注册表编辑器中定位”，直接打开 `regedit.exe` 并自动展开定位至该项对应的物理注册表路径。
- **多电源计划管理**：
  - 枚举系统中已安装的全部电源计划（卓越性能、高性能、平衡、节能等），并支持在 GUI/CLI 中一键切换当前激活方案。

---

## 📦 下载与安装

进入 [Releases 页面](https://github.com/vusiif/power-plan-revealer/releases) 获取预编译程序：

| 分发方式 | 文件名 | 说明 |
| :--- | :--- | :--- |
| **安装包 (推荐)** | `PowerPlanRevealer-v0.3.0-win64-setup.exe` | 包含完整安装向导、桌面与开始菜单快捷方式、卸载程序 |
| **绿色便携版** | `PowerPlanRevealer-v0.3.0-win64.zip` | 解压即用，无任何注册表残留，包含 CLI 与 GUI 完整运行时 |

> **运行说明**：修改注册表电源选项可见性需要 Windows 管理员权限。程序已内嵌提权清单，启动时会弹出 UAC 提权确认。

---

## 🎮 一键场景预设包

在 GUI 顶部工具栏点击 **“⚡ 场景预设与恢复”** 下拉菜单，即可快速执行常用操作：

1. **电竞低延迟包 (Gaming & Latency)**：
   - 处理器性能核心驻留最小核心数 / 最大核心数 (Core Parking)
   - 处理器性能提升模式 (Aggressive Turbo)
   - 处理器性能降低阈值 / 增加阈值
   - PCI Express 链路状态电源管理 (ASPM)
2. **笔电极限续航包 (Battery Saver)**：
   - 最大处理器状态限制
   - 显卡动态节能控制
   - 无线适配器最高节能模式
   - AHCI Link 节能管理 (HIPM / DIPM)
3. **音频/DAW 生产力防爆音包 (Audio DAW)**：
   - 处理器闲置禁用 (Idle Disable)
   - 允许节流状态 (Allow Throttle States)
   - 异构线程调度策略 (Hetero Scheduling)
4. **一键恢复默认隐藏状态 (Reset Visibility)**：
   - 将除微软默认公开的少量基础项之外的所有项目属性恢复为隐藏（`Attributes = 1`），且完全不改动任何当前参数数值。

---

## 🖥️ 命令行工具 (CLI) 用法

命令行程序为 `power-plan-revealer-cli.exe`，支持以下指令：

```powershell
# 查看帮助说明
power-plan-revealer-cli.exe --help

# 列出所有已安装的电源计划方案（标 * 的为当前激活方案）
power-plan-revealer-cli.exe schemes

# 切换系统的活动电源计划方案
power-plan-revealer-cli.exe set-scheme <scheme-guid>

# 列出所有电源计划项目（含分类、GUID、隐藏状态、当前 AC/DC 数值与详细描述）
power-plan-revealer-cli.exe list

# 仅列出当前处于隐藏状态的项目
power-plan-revealer-cli.exe hidden

# 取消隐藏某个具体设置项 (需要管理员权限)
power-plan-revealer-cli.exe unhide <subgroup-guid> <setting-guid>

# 隐藏某个具体设置项 (需要管理员权限)
power-plan-revealer-cli.exe hide <subgroup-guid> <setting-guid>

# 一键备份电源设置注册表至 .reg 文件 (默认保存为 PowerSettings_backup.reg)
power-plan-revealer-cli.exe backup [output-file.reg]
```

> **提示**：命令行中传入的 GUID 支持带 `{}` 或不带 `{}` 格式，例如：
> `power-plan-revealer-cli.exe unhide 54533251-82be-4824-96c1-47b60b740d00 be337238-0d82-4146-a960-4f3749d470c7`

---

## 🛠️ 构建与编译指南

### 环境要求
- **操作系统**：Windows 10 / 11 (x64)
- **编译器**：MSVC (推荐 Visual Studio 2022 / 2026 Build Tools，C11 & C++17 支持)
- **构建工具**：CMake 3.20+、Ninja
- **图形库**：Qt 6.x (包含 Widgets、Svg 模块)
- **安装包生成 (可选)**：Inno Setup 6.x

### 本地编译步骤

以 MSVC x64 命令行环境为例：

```powershell
# 1. 打开“x64 Native Tools Command Prompt”或运行 vcvars64.bat 初始化环境
& "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

# 2. 生成 CMake 构建工程（指定本地 Qt 6 路径）
cmake -B build -G Ninja -DCMAKE_PREFIX_PATH="C:/Qt/6.12.0/msvc2022_64" -DCMAKE_BUILD_TYPE=Release

# 3. 编译二进制
cmake --build build --config Release
```

编译产物位于 `build/` 目录下：
- `power-plan-revealer.exe`：GUI 主程序（内嵌 UAC 管理员提权与原生暗黑标题栏适配）
- `power-plan-revealer-cli.exe`：CLI 命令行程序

---

## ⚠️ 免责与安全须知

1. **不修改数值原则**：本工具的设计哲学是纯粹的“可见性切换器”。工具仅读取和修改注册表中的 `Attributes` 键值（`1` 为隐藏，`0` 或 `2` 为公开可见），**绝不会主动修改任何选项的 AC/DC 参数数值**。
2. **调参注意事项**：在解锁并在控制面板中微调某些深层硬件调度项（如 CPU 降频/停车激进度、PCIe 供电）前，请确保您了解其实际硬件影响。
3. **安全恢复**：随时可以使用软件中的 **“备份注册表”** 按钮导出备份；亦可使用 **“恢复出厂默认显隐”** 一键重置界面展示状态。

---

## 📄 开源许可证

本项目遵循 [GNU Lesser General Public License v2.1](LICENCE) (LGPL-2.1) 协议开源。
