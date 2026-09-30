# Migrate named assets from a marketplace pack in the asset library, with their textures cut to a maximum size.
# The generic form of ImportSurvivalCharacter.ps1 (Phase 5, the Landing Stage's timber kit and skiff):
#   1. copies the dependency closure of the seed packages (from the package name tables; Maps and Demo folders
#      skipped) from the library project's Content into ours, at the same /Game paths,
#   2. exports every texture in that closure to PNG (export_pack_textures.py),
#   3. resizes the PNGs to at most -MaxSize px,
#   4. reimports them over the originals at the same asset paths, keeping compression and sRGB, so the migrated
#      materials resolve unchanged (reimport_pack_textures.py).
# Run with the editor closed. Re-runnable. The result is committed; RebuildContent.bat does not run this.
# Refuses a seed whose closure would bring in a single package over -MaxPackageMB (high-poly scans).
#   powershell -File Tools\ImportPackAssets.ps1 -Name landing -Seeds /Game/Smugglers_cove/meshes/ships/SM_boat_dutch_small_01,...
param(
    [Parameter(Mandatory = $true)][string]$Name,
    [Parameter(Mandatory = $true)][string[]]$Seeds,
    [int]$MaxSize = 1024,
    [int]$MaxPackageMB = 12
)
$ErrorActionPreference = "Stop"
# With -File, "a,b,c" arrives as one string.
$Seeds = @($Seeds | ForEach-Object { $_ -split "," } | ForEach-Object { $_.Trim() } | Where-Object { $_ })
$repo = Split-Path -Parent $PSScriptRoot
$src = "C:\FO5_AssetLibrary\FO5_AssetLibrary\Content"
$dst = Join-Path $repo "Content"
$work = Join-Path $repo "Saved\PackExport\$Name"
$ue = if ($env:UE_ROOT) { $env:UE_ROOT } else { "C:\Program Files\Epic Games\UE_5.8" }
$cmd = Join-Path $ue "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
$project = Join-Path $repo "DeadCurrent.uproject"
$extensions = ".uasset", ".uexp", ".ubulk", ".uptnl"

# 1. Closure.
$enc = [Text.Encoding]::GetEncoding(28591)
$seen = [ordered]@{}
$queue = New-Object System.Collections.Queue
foreach ($s in $Seeds) { $queue.Enqueue($s) }
while ($queue.Count) {
    $p = $queue.Dequeue()
    if ($seen.Contains($p) -or $p -notlike "/Game/*" -or $p -like "*/Maps/*" -or $p -like "*/Demo/*") { continue }
    $file = Join-Path $src ($p.Substring(6).Replace("/", "\") + ".uasset")
    if (-not (Test-Path $file)) { continue }
    $seen[$p] = 1
    $text = $enc.GetString([IO.File]::ReadAllBytes($file))
    foreach ($m in [regex]::Matches($text, "/Game/[A-Za-z0-9_/\-]+")) {
        if (-not $seen.Contains($m.Value)) { $queue.Enqueue($m.Value) }
    }
}
foreach ($p in $seen.Keys) {
    $rel = $p.Substring(6).Replace("/", "\")
    $bytes = 0
    foreach ($ext in $extensions) { $f = Join-Path $src ($rel + $ext); if (Test-Path $f) { $bytes += (Get-Item $f).Length } }
    $isTexture = $rel -match "\\T_[^\\]+$"
    if (-not $isTexture -and $bytes -gt $MaxPackageMB * 1MB) {
        throw "$p is $([Math]::Round($bytes / 1MB)) MB (over -MaxPackageMB $MaxPackageMB). Not migrating it."
    }
}
Write-Host "Closure: $($seen.Count) packages"
foreach ($p in $seen.Keys) {
    $rel = $p.Substring(6).Replace("/", "\")
    foreach ($ext in $extensions) {
        $from = Join-Path $src ($rel + $ext)
        if (Test-Path $from) {
            $to = Join-Path $dst ($rel + $ext)
            New-Item -ItemType Directory -Force (Split-Path $to) | Out-Null
            Copy-Item $from $to -Force
        }
    }
}

# 2. Export the closure's textures.
if (Test-Path $work) { Remove-Item $work -Recurse -Force }
New-Item -ItemType Directory -Force "$work\full", "$work\small" | Out-Null
$seen.Keys | Set-Content -Encoding utf8 "$work\packages.txt"
$env:DC_PACK_WORK = $work
& $cmd $project -run=pythonscript -script="$PSScriptRoot\EditorScripts\export_pack_textures.py" -unattended -nullrhi -nosplash -log -abslog="$repo\Saved\Logs\PackExport_$Name.log" | Out-Null

# 3. Resize (bicubic, alpha kept).
Add-Type -AssemblyName System.Drawing
foreach ($f in Get-ChildItem "$work\full" -Recurse -File -Include *.png, *.tga, *.bmp) {
    $img = [System.Drawing.Image]::FromFile($f.FullName)
    $scale = [Math]::Min(1.0, $MaxSize / [Math]::Max($img.Width, $img.Height))
    $w = [Math]::Max(1, [int]($img.Width * $scale))
    $h = [Math]::Max(1, [int]($img.Height * $scale))
    $bmp = New-Object System.Drawing.Bitmap $w, $h, ([System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
    $g.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::HighQuality
    $g.DrawImage($img, 0, 0, $w, $h)
    $bmp.Save((Join-Path "$work\small" ($f.BaseName + ".png")), [System.Drawing.Imaging.ImageFormat]::Png)
    $g.Dispose(); $bmp.Dispose(); $img.Dispose()
}

# 4. Reimport.
& $cmd $project -run=pythonscript -script="$PSScriptRoot\EditorScripts\reimport_pack_textures.py" -unattended -nullrhi -nosplash -log -abslog="$repo\Saved\Logs\PackReimport_$Name.log" | Out-Null
Write-Host "Done. Check Saved\Logs\PackReimport_$Name.log for [DCPACK] lines."
