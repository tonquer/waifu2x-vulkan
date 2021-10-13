$LIB_NAME='waifu2x-vulkan'
$TAG_NAME=(git describe --abbrev=0 --tags)
$HEAD_SHA_SHORT=(git rev-parse --short HEAD)
$PACKAGE_PREFIX=($LIB_NAME + '-' + $TAG_NAME + '_' + $HEAD_SHA_SHORT)
$PACKAGENAME=($PACKAGE_PREFIX + '-windows')

$oldPath=$pwd
# Vulkan SDK
$TRUE_FALSE=(Test-Path ".\VulkanSDK")

if(! $TRUE_FALSE)
{
  Invoke-WebRequest -Uri `
    https://sdk.lunarg.com/sdk/download/1.2.162.0/windows/VulkanSDK-1.2.162.0-Installer.exe?Human=true `
    -OutFile VulkanSDK-1.2.162.0-Installer.exe
  try
  {
    7z x -aoa .\VulkanSDK-1.2.162.0-Installer.exe -oVulkanSDK
  }
  Catch
  {
    &"C:\Program Files\7-Zip\7z.exe" x -aoa .\VulkanSDK-1.2.162.0-Installer.exe -oVulkanSDK
  }
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
  $Env:PYTHON_BIN=(cmd /c where python|select -first 1)
}
$PYTHON_DIR=$Env:PYTHON_BIN.Replace("python.exe", "")
$V=(&$Env:PYTHON_BIN -V 2>&1).Replace("Python ", "").Substring(0, 3).Replace(".", "")

$PYTHON_LIBRARIES="$($PYTHON_DIR + '\libs\python3.lib;' + $PYTHON_DIR + "\libs\python" + $V + ".lib")"
$PYTHON_INCLUDE_DIRS="$($PYTHON_DIR + '\include')"

echo $PYTHON_DIR
echo $Env:PYTHON_BIN
echo $V
echo $oldPath
mkdir -Force build; Set-Location .\build\
cmake -A x64 `
      -DNCNN_VULKAN=ON `
      -DNCNN_BUILD_TOOLS=OFF `
      -DNCNN_BUILD_EXAMPLES=OFF `
      -DPYTHON_LIBRARIES="$PYTHON_LIBRARIES" `
      -DPYTHON_INCLUDE_DIRS="$PYTHON_INCLUDE_DIRS" `
      ..\src
cmake --build . --config Release -j 2
Set-Location .\Release\
Copy-Item -Force waifu2x_vulkan.dll waifu2x_vulkan.pyd

# Package
Set-Location $oldPath
mkdir -Force "$($PACKAGENAME)"
Copy-Item -Force -Verbose -Path "README.md" -Destination "$($PACKAGENAME)"
Copy-Item -Force -Verbose -Path "LICENSE" -Destination "$($PACKAGENAME)"
Copy-Item -Force -Verbose -Path "build\Release\waifu2x_vulkan.pyd" -Destination "$($PACKAGENAME)"
Copy-Item -Force -Verbose -Recurse -Path "models" -Destination "$($PACKAGENAME)"
Copy-Item -Force -Verbose -Recurse -Path "test" -Destination "$($PACKAGENAME)"
