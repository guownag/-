# 找到 Visual Studio 的 C++ 命令行环境，并"导入"到当前 PowerShell 会话。
#
# 为什么不直接 cmd /c "vcvars64.bat && cmake ..."？
# 因为你当前的路径里有中文（嵌入式 三个字）。cmd.exe 处理中文路径的编码
# 很容易出问题：要么找不到文件，要么传参乱码。这里改成：只让 cmd 跑一句
# 纯 ASCII 路径的 vcvars64.bat 并把环境变量 dump 出来，剩下的活全部由
# PowerShell（原生 Unicode）来干。
[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'

function Find-VcVars64 {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (Test-Path -LiteralPath $vswhere) {
        $install = & $vswhere -latest -products * `
            -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
            -property installationPath 2>$null
        if ($install) {
            $candidate = Join-Path $install 'VC\Auxiliary\Build\vcvars64.bat'
            if (Test-Path -LiteralPath $candidate) { return $candidate }
        }
    }

    $fallbacks = @(
        'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat'
        'C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat'
        'C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat'
        'C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat'
        'C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat'
    )
    foreach ($f in $fallbacks) {
        if (Test-Path -LiteralPath $f) { return $f }
    }
    return $null
}

$vcvars = Find-VcVars64
if (-not $vcvars) {
    throw "没找到 Visual Studio 的 C++ 工具链 (vcvars64.bat)。请安装 'Visual Studio 生成工具' 并勾选 '使用 C++ 的桌面开发'。"
}

# 只提取我们要的环境变量，避免把整个环境都覆盖掉。
$names = 'PATH', 'INCLUDE', 'LIB', 'LIBPATH', 'VCToolsInstallDir', 'WindowsSdkDir', 'WindowsSdkVersion'
$query = ($names | ForEach-Object { "set $_" }) -join ' && '
$dump  = & cmd.exe /c "`"$vcvars`" >nul 2>&1 && $query"
if ($LASTEXITCODE -ne 0) {
    throw "执行 vcvars64.bat 失败: $vcvars"
}

foreach ($line in $dump) {
    if ($line -match '^([A-Za-z_][A-Za-z0-9_()]*)=(.*)$') {
        Set-Item -Path ('env:' + $matches[1]) -Value $matches[2]
    }
}

$cl = Get-Command cl.exe -ErrorAction SilentlyContinue
if (-not $cl) { throw "vcvars 跑完了，但还是找不到 cl.exe，环境有问题。" }

Write-Host "MSVC 环境就绪: $($cl.Source)" -ForegroundColor DarkGray

