# 一键：配置 + 编译 + 跑测试。
#
#   .\tools\build.ps1                  # Debug 构建并跑测试
#   .\tools\build.ps1 -Config Release
#   .\tools\build.ps1 -Lesson 01       # 只构建/测试第 01 课
#   .\tools\build.ps1 -Clean
[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release')][string]$Config = 'Debug',
    [string]$Lesson = '',
    [switch]$Clean,
    [switch]$NoTests
)

$ErrorActionPreference = 'Stop'

$root     = Split-Path -Parent $PSScriptRoot
$buildDir = Join-Path $root ('build\' + $Config.ToLower())

if ($Clean -and (Test-Path -LiteralPath $buildDir)) {
    Write-Host "清理 $buildDir" -ForegroundColor DarkGray
    Remove-Item -LiteralPath $buildDir -Recurse -Force
}

. (Join-Path $PSScriptRoot 'vsenv.ps1') | Out-Null

$cmakeArgs = @(
    '-S', $root
    '-B', $buildDir
    '-G', 'Ninja'
    "-DCMAKE_BUILD_TYPE=$Config"
    '-DCMAKE_EXPORT_COMPILE_COMMANDS=ON'
)

Write-Host "`n[1/3] 配置 CMake" -ForegroundColor Cyan
& cmake @cmakeArgs
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "`n[2/3] 编译" -ForegroundColor Cyan
# 全量编译。工程很小，全量编译也就几秒，
# 换来的是不用维护"课号 -> 目标名"的映射，省心。
& cmake --build $buildDir
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

if ($NoTests) { return }

Write-Host "`n[3/3] 跑测试" -ForegroundColor Cyan
Push-Location $buildDir
try {
    if ($Lesson) {
        & ctest --output-on-failure -R "^l${Lesson}"
    } else {
        & ctest --output-on-failure
    }
    $code = $LASTEXITCODE
} finally {
    Pop-Location
}

if ($code -ne 0) {
    Write-Host "`n有测试没过。这不是坏事 —— 它就是你的待办清单。" -ForegroundColor Yellow
}
exit $code
