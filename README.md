# PowerPlanRevealer

PowerPlanRevealer 是一个专为 Windows 系统设计的电源计划高级设置隐藏项管理工具，提供 **现代化图形界面 (GUI)** 与 **命令行工具 (CLI)** 两个版本。

通过调用 Windows 原生 `PowrProf` API，本项目可以深入读取电源计划中的所有高级配置项目及其官方描述文本，并允许用户安全地查看、搜索、隐藏或取消隐藏特定高级设置项（例如：处理器性能提升模式、AHCI 链接电源管理、异构线程调度策略等）。

---

## ✨ 主要特性

- **双版本支持**：
  - **GUI 图形界面**：基于 Qt 6 Widgets 构建，提供树形分类展示、实时搜索过滤、当前电源计划显示与现代感 UI。
  - **CLI 命令行工具**：轻量无依赖，适合系统运维、批处理脚本与高级用户快速查询。
- **深度信息获取**：调用 Windows 底层 `PowerReadDescription` API，不仅显示设置项名称与 GUID，还可查看系统官方详细功能说明。
- **安全第一的设计**：
  - **一键备份注册表**：GUI 与 CLI 均支持一键将 `PowerSettings` 注册表项完整导出为 `.reg` 备份文件。
  - **修改二次确认**：显示当前与目标隐藏状态，并在操作完成后回显修改前后的十六进制属性对比（Attributes: 0x00000001 vs 0x00000000）。
  - **即时刷新生效**：修改属性后自动调用 `PowerSetActiveScheme` 刷新当前激活方案，即时生效无需重启。
- **多电源计划管理**：支持枚举系统中安装的所有电源计划（如“平衡”、“高性能”、“节能”、“卓越性能”），并在 GUI 与 CLI 中一键切换系统的当前活动方案。
- **配置参数全景查看 (Value Inspector)**：调用 `PowerReadACValueIndex` 与 `PowerReadDCValueIndex`，实时查看设置项在“接通电源 (AC)”与“使用电池 (DC)”模式下的当前数值（十六进制与十进制对照）。
- **注册表编辑器联动**：支持在右键菜单中一键打开 `regedit.exe` 并直接定位至所选设置项或分类的注册表路径。
- **现代感 UI & 系统深色模式自适应**：
  - 自动检测并实时监听 Windows 10/11 系统的主题切换（浅色/深色），动态无缝切换浅色卡片与深色极客质感 QSS 样式表。
- **高效交互体验**：
  - 支持右键快捷菜单：一键复制名称、复制 GUID、复制注册表路径、在注册表编辑器中定位、快速切换状态。
  - 支持双击切换隐藏状态。
  - 智能搜索框：支持根据分类名、设置项名称、GUID 以及说明关键字即时检索。
  - 支持“仅显示隐藏项”快速过滤。

---

## 🖥️ 命令行工具 (CLI) 用法

命令行程序为 `power-plan-revealer-cli.exe`，支持以下指令：

```powershell
# 查看帮助说明
power-plan-revealer-cli.exe --help

# 列出所有已安装的电源计划方案（标 * 的为当前活动方案）
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

# 一键备份电源设置注册表至 .reg 文件 (可选输出路径，默认保存为 PowerSettings_backup.reg)
power-plan-revealer-cli.exe backup [output-file.reg]
```

> **提示**：命令行中传入的 GUID 支持带 `{}` 或不带 `{}` 格式，例如：
> `power-plan-revealer-cli.exe unhide 54533251-82be-4824-96c1-47b60b740d00 be337238-0d82-4146-a960-4f3749d470c7`

---

## 🛠️ 构建与编译指南

### 环境要求
- **操作系统**：Windows 10 / 11 (x64)
- **编译器**：MSVC (推荐 Visual Studio 2022 / 2026 Build Tools，C11 & C++17 支持)
- **构建工具**：CMake 3.20+、Ninja 或 MSBuild
- **第三方库**：Qt 6.x (Widgets 模块)

### 使用 CMake 命令行编译

以 MSVC x64 环境为例：

```powershell
# 1. 打开“x64 Native Tools Command Prompt”或调用 vcvars64.bat 初始化环境
& "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

# 2. 生成构建文件（指定本地 Qt 6 路径）
cmake -B build -G Ninja -DCMAKE_PREFIX_PATH="C:/Qt/6.11.1/msvc2022_64" -DCMAKE_BUILD_TYPE=Release

# 3. 编译
cmake --build build --config Release
```

编译完成后产物位于构建目录下：
- `power-plan-revealer.exe`：GUI 主程序（已内嵌管理员权限提权清单）
- `power-plan-revealer-cli.exe`：CLI 命令行程序

---

## ⚠️ 安全与恢复须知

1. 本工具仅读写 Windows 电源选项的显示/隐藏属性（`POWER_ATTRIBUTE_HIDE`），**不会**擅自修改您当前的实际电源参数数值。
2. Windows 高级电源设置直接作用于系统硬件与驱动调度，在调整可见性并前往控制面板修改其具体数值前，请确保您了解该配置项的作用。
3. 若需备份或还原注册表，可点击 GUI 工具栏中的 **“备份注册表”** 按钮导出 `.reg` 文件。如需恢复，双击导出的 `.reg` 文件导入即可。

---

## 📄 开源许可证

本项目基于 [GNU Lesser General Public License v2.1](LICENCE) (LGPL-2.1) 开源。
