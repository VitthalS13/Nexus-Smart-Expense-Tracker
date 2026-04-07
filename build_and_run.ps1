param (
    [switch]$SkipDownload = $false
)

$baseDir = $PSScriptRoot
$cxx = "g++"

# Check if g++ exists
if ((Get-Command "g++" -ErrorAction SilentlyContinue) -eq $null) {
    Write-Host "g++ not found. Downloading portable compiler (w64devkit)..." -ForegroundColor Yellow
    $zipPath = Join-Path $baseDir "w64devkit.zip"
    $devKitDir = Join-Path $baseDir "w64devkit"
    
    if (-not (Test-Path $devKitDir)) {
        if (-not (Test-Path $zipPath)) {
            Write-Host "Downloading w64devkit (this may take a minute)..."
            Invoke-WebRequest -Uri "https://github.com/skeeto/w64devkit/releases/download/v1.23.0/w64devkit-1.23.0.zip" -OutFile $zipPath
        }
        Write-Host "Extracting compiler..."
        Expand-Archive -Path $zipPath -DestinationPath $baseDir -Force
    }
    
    $cxx = Join-Path $devKitDir "bin\g++.exe"
    Write-Host "Using standalone compiler at: $cxx" -ForegroundColor Green
}

Write-Host "================== COMPILING BACKEND ==================" -ForegroundColor Cyan
Set-Location -Path (Join-Path $baseDir "backend")

# Need to add bin folder to PATH so g++ can find as.exe and ld.exe
if (Test-Path $devKitDir) {
    $env:PATH = (Join-Path $devKitDir "bin") + ";" + $env:PATH
}

# Files to compile
$srcFiles = "main.cpp src/models.cpp src/database.cpp src/server_router.cpp"
$outFile = "server.exe"

$compileCmd = "$cxx -std=c++17 -O3 $srcFiles -o $outFile -lws2_32 -lmswsock -ladvapi32"
Invoke-Expression $compileCmd

if ($LASTEXITCODE -eq 0) {
    Write-Host "Compilation successful! Output: $outFile" -ForegroundColor Green
    Write-Host "==================== STARTING SERVER ====================" -ForegroundColor Magenta
    
    # Run the server in a new window or inline
    Start-Process -FilePath ".\$outFile" -NoNewWindow
} else {
    Write-Host "Compilation failed with code $LASTEXITCODE" -ForegroundColor Red
}
