$LIB_NAME='waifu2x-vulkan'
$TAG_NAME=(git describe --abbrev=0 --tags)
$HEAD_SHA_SHORT=(git rev-parse --short HEAD)
$PACKAGE_PREFIX=($LIB_NAME + '-' + $TAG_NAME + '_' + $HEAD_SHA_SHORT)
$PACKAGENAME1=($PACKAGE_PREFIX + '-windows')

$oldPath=$pwd
# Vulkan SDK
$TRUE_FALSE=(Test-Path ".\VulkanSDK")

if(! $TRUE_FALSE)
{
  Invoke-WebRequest -Uri `
    https://sdk.lunarg.com/sdk/download/1.2.162.0/windows/VulkanSDK-1.2.162.0-Installer.exe?Human=true `
    -OutFile VulkanSDK-1.2.162.0-Installer.exe
  7z x -aoa .\VulkanSDK-1.2.162.0-Installer.exe -oVulkanSDK
  Remove-Item .\VulkanSDK\Demos, `
              .\VulkanSDK\Samples, `
              .\VulkanSDK\Third-Party, `
              .\VulkanSDK\Tools, `
              .\VulkanSDK\Tools32, `
              .\VulkanSDK\Bin32, `
              .\VulkanSDK\Lib32 `
              -Recurse
}

# Python (x86_64)
# $Env:pythonLocation='C:\Python37'
$Env:VULKAN_SDK=((Get-Location).Path + '\VulkanSDK')
if(! $Env:PYTHON_BIN)
{
  $Env:PYTHON_BIN="$($Env:pythonLocation + '\python.exe')"
}

if(! $Env:BUILD_PATH)
{
  $Env:BUILD_PATH="build"
}

$V=&$Env:PYTHON_BIN -V 2>&1
$V=$V.Replace("Python ","")
echo $Env:PYTHON_BIN
echo $V
echo $oldPath
mkdir -p $Env:BUILD_PATH; Set-Location .\$Env:BUILD_PATH\
cmake -A x64 `
      -DNCNN_VULKAN=ON `
      -DNCNN_BUILD_TOOLS=OFF `
      -DNCNN_BUILD_EXAMPLES=OFF `
      -DPYTHON_EXECUTABLE="$Env:PYTHON_BIN" `
      -DPYBIND11_FINDPYTHON=OFF `
      ..\src
cmake --build . --config Release -j 2
Set-Location .\Release\
Copy-Item waifu2x_vulkan.dll waifu2x_vulkan.pyd

# Package
Set-Location $oldPath
$PACKAGENAME=($PACKAGENAME1 + '-py' + $V)
mkdir "$($PACKAGENAME)"
Copy-Item -Verbose -Path "README.md" -Destination "$($PACKAGENAME)"
Copy-Item -Verbose -Path "LICENSE" -Destination "$($PACKAGENAME)"
Copy-Item -Verbose -Path "$Env:BUILD_PATH\Release\waifu2x_vulkan.pyd" -Destination "$($PACKAGENAME)"
Copy-Item -Verbose -Recurse -Path "models" -Destination "$($PACKAGENAME)"
Copy-Item -Verbose -Recurse -Path "test" -Destination "$($PACKAGENAME)"
