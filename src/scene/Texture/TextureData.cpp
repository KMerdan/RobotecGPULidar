

#include "TextureData.hpp"
#include "macros/cuda.hpp"
#include <cuda_runtime.h>

TextureData::TextureData(void *data, rgl_texture_format format, int resolution)
{
	cudaResourceDesc res_desc = {};
	cudaChannelFormatDesc channel_desc;

	int32_t width = resolution;
	int32_t height = resolution;
	int32_t numComponents = 1;
	int32_t pitch = width * numComponents * getFormatSize(format);

	channel_desc = cudaCreateChannelDesc<uchar1>();

	// TODO prybicki
	// Should we leave it like this, or add new copiers in DeivceBuffer.hpp?
	// Current copyFromHost and ensureDeviceCanFit are not working with cudaArray_t
	CHECK_CUDA(cudaMallocArray(&dPixelArray, &channel_desc, width, height));


	CHECK_CUDA(cudaMemcpy2DToArray(
			dPixelArray,
			0, 0,
			data,
			pitch, pitch, height,
			cudaMemcpyHostToDevice));

	res_desc.resType = cudaResourceTypeArray;
	res_desc.res.array.array = dPixelArray;

	cudaTextureDesc tex_desc = {};

	tex_desc.addressMode[0] = cudaAddressModeWrap;
	tex_desc.addressMode[1] = cudaAddressModeWrap;
	tex_desc.filterMode = cudaFilterModeLinear;
	tex_desc.readMode = cudaReadModeNormalizedFloat;
	tex_desc.normalizedCoords = 1;
	tex_desc.maxAnisotropy = 1;
	tex_desc.maxMipmapLevelClamp = 99;
	tex_desc.minMipmapLevelClamp = 0;
	tex_desc.mipmapFilterMode = cudaFilterModePoint;
	tex_desc.borderColor[0] = 1.0f;
	// TO check tex_desc.sRGB = 0;

	CHECK_CUDA(cudaCreateTextureObject(&textureObject, &res_desc, &tex_desc, nullptr));

}

TextureData::~TextureData()
{
	CHECK_CUDA(cudaDestroyTextureObject(textureObject));
	CHECK_CUDA(cudaFreeArray(dPixelArray));
}
