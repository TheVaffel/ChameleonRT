# OpenGL_GL_PREFERENCE=GLVND 
cmake -DOpenGL_GL_PREFERENCE=LEGACY -DENABLE_OPTIX=true -DNO_DISPLAY=ON -DCUDA_TOOLKIT_ROOT_DIR=/usr/local/cuda-10.2/ -DOptiX_INCLUDE_DIR=~/SDK/NVIDIA-OptiX-SDK-7.0.0-linux64/include/ -DOptiX_INSTALL_DIR=~/SDK/NVIDIA-OptiX-SDK-7.0.0-linux64/SDK/ ..
