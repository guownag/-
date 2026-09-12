# 第 00 课专用：不用 CMake，手工走一遍
#   预处理 -> 编译 -> 汇编 -> 链接
# 让你亲眼看见这四步分别产出什么文件。
[CmdletBinding()]
param(
    [switch]$KeepAsm
)

$ErrorActionPreference = 'Stop'

$root    = Split-Path -Parent $PSScriptRoot
$lesson  = Join-Path $root 'lessons\00-toolchain'
$out     = Join-Path $root 'build\raw'

New-Item -ItemType Directory -Force -Path $out | Out-Null

. (Join-Path $PSScriptRoot 'vsenv.ps1') | Out-Null

# /utf-8 是必须的：源文件里的中文注释是 UTF-8，不加这个 MSVC 会按
# 系统代码页(GBK)去读，注释被读坏之后甚至会连累后面的代码语法报错。
# /Zc:__cplusplus 也不能少：MSVC 默认把 __cplusplus 报成 199711L（C++98），
# 除非你显式打开这个开关。很多跨平台库靠这个宏判断标准版本，被它坑过。
$common = @('/nologo', '/std:c++17', '/EHsc', '/W4', '/utf-8', '/Zc:__cplusplus',
            "/I$lesson", "/Fo$out\")

Write-Host "`n=== 步骤 1/4：预处理（展开 #include / 宏） ===" -ForegroundColor Cyan
Push-Location $lesson
try {
    & cl.exe /P @common main.cpp "/Fi$out\main.i"
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    Write-Host "生成: $out\main.i   （行数：$((Get-Content -LiteralPath "$out\main.i").Count)）"

    Write-Host "`n=== 步骤 2/4：编译 + 3/4 汇编（生成目标文件和汇编清单） ===" -ForegroundColor Cyan
    $asm = if ($KeepAsm) { @('/FAs') } else { @('/FAs', "/Fa$out\") }
    & cl.exe /c @common @asm main.cpp counter.cpp math_util.cpp
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
} finally {
    Pop-Location
}

Get-ChildItem -LiteralPath $out -File | Sort-Object Name |
    Select-Object Name, Length | Format-Table -AutoSize

Write-Host "=== 步骤 4/4：链接（把 .obj 拼成 .exe） ===" -ForegroundColor Cyan
& link.exe '/nologo' "/OUT:$out\app.exe" "$out\main.obj" "$out\counter.obj" "$out\math_util.obj"
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "`n运行:" -ForegroundColor Cyan
& "$out\app.exe"

if (-not $KeepAsm) {
    Remove-Item -LiteralPath "$out\main.asm", "$out\counter.asm", "$out\math_util.asm" -ErrorAction SilentlyContinue
    Write-Host "`n(想保留汇编清单就加 -KeepAsm，文件在 $out)" -ForegroundColor DarkGray
}
