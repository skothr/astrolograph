#include "cutools.hpp"
#include "cudaField.hpp"
#include <iostream>

#define STB_IMAGE_WRITE_STATIC
// #define STBIW_ZLIB_COMPRESS
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// #define BLOCKDIM_X 16
// #define BLOCKDIM_Y 16

bool gCudaInitialized = false;
int  gDevId = -1;

bool initDevice(int devId)
{
  if(!gCudaInitialized || (devId > 0 && devId != gDevId))
    {
      int devCount; checkCudaErrors(cudaGetDeviceCount(&devCount));
  
      std::cout << "\n";
      std::cout << "==== Number of GPUs with CUDA capability detected: " << devCount << "\n";
      if(devCount == 0)
        { std::cout << "====> WARNING(cuda): No devices found that support CUDA.\n\n"; return false; }

      devId = std::max(devId, 0);
      if(devId > devCount-1)
        { std::cout << "====> ERROR(cuda): Device " << devId << " is not a valid CUDA device.\n\n"; return false; }

      int computeMode = -1; int major = 0; int minor = 0;
      checkCudaErrors(cudaDeviceGetAttribute(&computeMode, cudaDevAttrComputeMode,      devId));
      checkCudaErrors(cudaDeviceGetAttribute(&major, cudaDevAttrComputeCapabilityMajor, devId));
      checkCudaErrors(cudaDeviceGetAttribute(&minor, cudaDevAttrComputeCapabilityMinor, devId));
      if(computeMode == cudaComputeModeProhibited)
        { std::cout << "====> ERROR(cuda): Compute Mode is prohibited on this device(" << devId << "). No threads can use cudaSetDevice().\n"; return false; }
      else if(major < 1)
        { std::cout << "====> ERROR(cuda): GPU does not support CUDA.\n"; return false; }

      std::cout << "==== Using CUDA device " << devId << " (" << _ConvertSMVer2ArchName(major, minor) << ")\n";
      checkCudaErrors(cudaSetDevice(devId));

      // get number of SMs on this GPU
      cudaDeviceProp deviceProps; checkCudaErrors(cudaGetDeviceProperties(&deviceProps, devId));
      printf("CUDA device [%s] has %d Multi-Processors\n\n", deviceProps.name, deviceProps.multiProcessorCount);
      
      gDevId = devId;
      gCudaInitialized = true;
    }
  return true;
}

bool writeTexture(const std::string &path, CudaFieldTex *tex)
{
  stbi_write_png_compression_level = 6;
  
  if(tex->size.x > 0 && tex->size.y > 0)
    {
      if(path.find(".png") != std::string::npos && !stbi_write_png(path.c_str(), tex->size.x, tex->size.y, 4, (const float*)tex->hData, 4*tex->size.x))
        { std::cout << "====> ERROR: Could not write CudaFieldTex data to PNG file '" << path << "'!\n";  return false; }
      else if(path.find(".bmp") != std::string::npos && !stbi_write_bmp(path.c_str(), tex->size.x, tex->size.y, 4, (const float*)tex->hData))
        { std::cout << "====> ERROR: Could not write CudaFieldTex data to BMP file '" << path << "'!\n";  return false; }
      else if(path.find(".hdr") != std::string::npos && !stbi_write_hdr(path.c_str(), tex->size.x, tex->size.y, 4, (const float*)tex->hData))
        { std::cout << "====> ERROR: Could not write CudaFieldTex data to HDR file '" << path << "'!\n";  return false; }
      else { return true; }
    }
  else { return false; }
}

