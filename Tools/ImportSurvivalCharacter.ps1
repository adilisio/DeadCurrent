# One-time migration of the Survival_Character pack from the asset library, with textures cut to 1K.
# The pack's textures are 4K and 8K (about 770 MB). They must not enter the repo at that size, so this:
#   1. copies only what SK_Survival_Character needs (not the Demo folder) into Content/Survival_Character,
#   2. exports every texture to PNG (export_survival_textures.py),
#   3. resizes the PNGs to at most 1024 px,
#   4. reimports them over the originals at the same asset paths, keeping each texture's compression and sRGB
#      settings, so the migrated material instances resolve unchanged (reimport_survival_textures.py).
# Run with the editor closed. Re-runnable. The result is committed; RebuildContent.bat does not run this.
#   powershell -File Tools\ImportSurvivalCharacter.ps1
$ErrorActionPreference = "Stop"
$repo = Split-Path -Parent $PSScriptRoot
$src = "C:\FO5_AssetLibrary\FO5_AssetLibrary\Content"
$dst = Join-Path $repo "Content"
$work = Join-Path $repo "Saved\SurvivalExport"
$ue = if ($env:UE_ROOT) { $env:UE_ROOT } else { "C:\Program Files\Epic Games\UE_5.8" }
$cmd = Join-Path $ue "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
$project = Join-Path $repo "DeadCurrent.uproject"

# 1. Dependency closure inside the pack, from the package name tables. The Demo folder is the pack's own mannequin
#    showcase and is not needed.
$enc = [Text.Encoding]::GetEncoding(28591)
$seen = @{}
$queue = New-Object System.Collections.Queue
foreach ($s in "SK_Survival_Character", "SKEL_Survival_Character", "PHYS_Survival_Character") {
    $queue.Enqueue("/Game/Survival_Character/Meshes/$s")
}
while ($queue.Count) {
    $p = $queue.Dequeue()
    if ($seen.ContainsKey($p) -or $p -like "*/Demo/*" -or $p -notlike "/Game/Survival_Character/*") { continue }
    $seen[$p] = 1
    $file = Join-Path $src ($p.Substring(6).Replace("/", "\") + ".uasset")
    if (-not (Test-Path $file)) { continue }
    $text = $enc.GetString([IO.File]::ReadAllBytes($file))
    foreach ($m in [regex]::Matches($text, "/Game/[A-Za-z0-9_/\-]+")) {
        if (-not $seen.ContainsKey($m.Value)) { $queue.Enqueue($m.Value) }
    }
}
Write-Host "Pack closure: $($seen.Count) packages"
foreach ($p in $seen.Keys) {
    $rel = $p.Substring(6).Replace("/", "\")
    foreach ($ext in ".uasset", ".uexp") {
        $from = Join-Path $src ($rel + $ext)
        if (Test-Path $from) {
            $to = Join-Path $dst ($rel + $ext)
            New-Item -ItemType Directory -Force (Split-Path $to) | Out-Null
            Copy-Item $from $to -Force
        }
    }
}

# 2. Export.
if (Test-Path $work) { Remove-Item $work -Recurse -Force }
New-Item -ItemType Directory -Force "$work\full", "$work\1k" | Out-Null
& $cmd $project -run=pythonscript -script="$PSScriptRoot\EditorScripts\export_survival_textures.py" -unattended -nullrhi -nosplash -log -abslog="$repo\Saved\Logs\SurvivalExport.log" | Out-Null

# 3. Resize (bicubic, alpha kept).
Add-Type -AssemblyName System.Drawing
foreach ($f in Get-ChildItem "$work\full" -Recurse -File -Include *.png, *.tga, *.bmp) {
    $img = [System.Drawing.Image]::FromFile($f.FullName)
    $scale = [Math]::Min(1.0, 1024.0 / [Math]::Max($img.Width, $img.Height))
    $w = [Math]::Max(1, [int]($img.Width * $scale))
    $h = [Math]::Max(1, [int]($img.Height * $scale))
    $bmp = New-Object System.Drawing.Bitmap $w, $h, ([System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
    $g.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::HighQuality
    $g.DrawImage($img, 0, 0, $w, $h)
    $out = Join-Path "$work\1k" ($f.BaseName + ".png")
    $bmp.Save($out, [System.Drawing.Imaging.ImageFormat]::Png)
    $g.Dispose(); $bmp.Dispose(); $img.Dispose()
}

# 4. Reimport.
& $cmd $project -run=pythonscript -script="$PSScriptRoot\EditorScripts\reimport_survival_textures.py" -unattended -nullrhi -nosplash -log -abslog="$repo\Saved\Logs\SurvivalReimport.log" | Out-Null
Write-Host "Done. Check Saved\Logs\SurvivalReimport.log for [DCSURV] lines."
