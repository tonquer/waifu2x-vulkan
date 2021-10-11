$LIB_NAME='waifu2x-vulkan'
$TAG_NAME=(git describe --abbrev=0 --tags)
$HEAD_SHA_SHORT=(git rev-parse --short HEAD)
$PACKAGE_PREFIX=($LIB_NAME + '-' + $TAG_NAME + '_' + $HEAD_SHA_SHORT)
$PACKAGENAME1=($PACKAGE_PREFIX + '-windows')

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
$Env:VULKAN_SDK=((Get-Location).Path + '\VulkanSDK')
$V=python -V 2>&1
$V=$V.Replace("Python ","")
$PythonEx=cmd /c where python|select -first 1
mkdir -p build; Set-Location .\build\
cmake -A x64 `
      -DNCNN_VULKAN=ON `
      -DNCNN_BUILD_TOOLS=OFF `
      -DNCNN_BUILD_EXAMPLES=OFF `
      -DPYTHON_EXECUTABLE=$PythonEx `
      -DPYBIND11_FINDPYTHON=OFF `
      ..\src
cmake --build . --config Release -j 2
Set-Location .\Release\
Move-Item waifu2x_vulkan.dll waifu2x_vulkan.pyd

# Package
Set-Location .\..\..\
$PACKAGENAME=($PACKAGENAME1 + '-py' + $V)
mkdir "$($PACKAGENAME)"
Copy-Item -Verbose -Path "README.md" -Destination "$($PACKAGENAME)"
Copy-Item -Verbose -Path "LICENSE" -Destination "$($PACKAGENAME)"
Copy-Item -Verbose -Path "build\Release\waifu2x_vulkan.pyd" -Destination "$($PACKAGENAME)"
Copy-Item -Verbose -Recurse -Path "models" -Destination "$($PACKAGENAME)"
Copy-Item -Verbose -Recurse -Path "test" -Destination "$($PACKAGENAME)"
