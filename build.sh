cd src/ncnn && mkdir build
cd build 
cmake -DCMAKE_BUILD_TYPE=Release -DNCNN_VULKAN=ON -DNCNN_SYSTEM_GLSLANG=ON -DNCNN_BUILD_EXAMPLES=ON ..
make -j$(nproc)

cd ../../libwebp && mkdir build
cd build
cmake ..
make -j$(nproc)

