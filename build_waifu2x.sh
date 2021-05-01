PythonDir="/home/tonquer/Downloads/Python-3.7.9"
PythonLib="/home/tonquer/Downloads/Python-3.7.9/libpython3.7m.a"

cd src
g++  -fopenmp -I$PythonDir -I$PythonDir/Include -Incnn/build/src -Incnn/src/ -Ilibwebp/src waifu2x_py.cpp waifu2x_main.cpp waifu2x.cpp ncnn/build/src/libncnn.a libwebp/build/libwebp.a ncnn/build/glslang/glslang/libMachineIndependent.a ncnn/build/glslang/OGLCompilersDLL/libOGLCompiler.a ncnn/build/glslang/glslang/OSDependent/Unix/libOSDependent.a ncnn/build/glslang/glslang/libGenericCodeGen.a ncnn/build/glslang/SPIRV/libSPIRV.a ncnn/build//glslang/glslang/libglslang.a $PythonLib -lvulkan -pthread  -shared -fPIC -o ../waifu2x.so
cd ..
