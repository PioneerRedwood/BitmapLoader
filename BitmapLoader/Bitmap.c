#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <Windows.h>
#include "Bitmap.h"

DWORD ReadFileWithOffset(HANDLE hFile, int size, int offset, void* dst)
{
	DWORD readBytes;
	if (!ReadFile(hFile, dst, size, &readBytes, NULL))
	{
		printf("[ERROR] reading file : %lu \n", GetLastError());
		CloseHandle(hFile);
		return 0;
	}

	// Move the offset as (previous offset + read)
	if (SetFilePointer(hFile, offset + readBytes, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
	{
		printf("[ERROR] setting file pointer: %lu \n", GetLastError());
		CloseHandle(hFile);
		return 0;
	}

	return readBytes;
}

int ReadBitmapHeaderInfo(HANDLE hFile, bitmap* bmp, int* offset) {
	bitmap_header_info* h = &(bmp->header_info);
	// header_field (2 bytes)
	DWORD readBytes = ReadFileWithOffset(hFile, sizeof(char) * 2, *offset, &h->header_field);
	if (readBytes == 0) {
		// TODO: return the reason of failure
		return 1;
	}
	*offset += readBytes;

	// size (4 bytes)
	readBytes = ReadFileWithOffset(hFile, sizeof(uint32_t), *offset, &h->size);
	if (readBytes == 0) return 1;
	*offset += readBytes;

	// reserved1 (2 bytes)
	readBytes = ReadFileWithOffset(hFile, sizeof(uint16_t), *offset, &h->reserved1);
	if (readBytes == 0) return 1;
	*offset += readBytes;

	// reserved2 (2 bytes)
	readBytes = ReadFileWithOffset(hFile, sizeof(uint16_t), *offset, &h->reserved2);
	if (readBytes == 0) return 1;
	*offset += readBytes;

	// pixel_start_offset (4 bytes)
	readBytes = ReadFileWithOffset(hFile, sizeof(uint32_t), *offset, &h->pixel_start_offset);
	if (readBytes == 0) return 1;
	*offset += readBytes;

	return 0;
}

/// <summary>
/// Read BITMAP_INFO_HEADER
/// Start offset of file: 14, total expected header size: 40 bytes
/// </summary>
/// <param name="hFile">File handle</param>
/// <param name="bmp">Bitmap pointer</param>
/// <param name="offset">File offset to read</param>
/// <returns>Success 0, failed or error 1</returns>
int ReadBitmapInfoHeader(HANDLE hFile, bitmap* bmp, int* offset) {
	bitmap_v5_info_header* h = &bmp->dib_header;
	h->size = (uint32_t)bmp->header_type;

	int readBytes = 0;

	// width (4)
	readBytes = ReadFileWithOffset(hFile, sizeof(uint32_t), *offset, &h->width);
	if (readBytes == 0) return 1;
	*offset += readBytes;

	// height (4)
	readBytes = ReadFileWithOffset(hFile, sizeof(uint32_t), *offset, &h->height);
	if (readBytes == 0) return 1;
	*offset += readBytes;

	// num_color_planes (2)
	readBytes = ReadFileWithOffset(hFile, sizeof(uint16_t), *offset, &h->num_color_planes);
	if (readBytes == 0) return 1;
	*offset += readBytes;


	// bits_per_pixel (2)
	readBytes = ReadFileWithOffset(hFile, sizeof(uint16_t), *offset, &h->bits_per_pixel);
	if (readBytes == 0) return 1;
	*offset += readBytes;

	// compression_method (4)
	readBytes = ReadFileWithOffset(hFile, sizeof(bitmap_compression_method), *offset, &h->compression_method);
	if (readBytes == 0) return 1;
	*offset += readBytes;

	// image_size (4)
	readBytes = ReadFileWithOffset(hFile, sizeof(uint32_t), *offset, &h->image_size);
	if (readBytes == 0) return 1;
	*offset += readBytes;


	// horizontal_resolution (4)
	readBytes = ReadFileWithOffset(hFile, sizeof(uint32_t), *offset, &h->horizontal_resolution);
	if (readBytes == 0) return 1;
	*offset += readBytes;

	// vertical_resolution (4)
	readBytes = ReadFileWithOffset(hFile, sizeof(uint32_t), *offset, &h->vertical_resolution);
	if (readBytes == 0) return 1;
	*offset += readBytes;

	// color_palette (4)
	readBytes = ReadFileWithOffset(hFile, sizeof(uint32_t), *offset, &h->color_palette);
	if (readBytes == 0) return 1;
	*offset += readBytes;

	if (h->color_palette > 0) {
		// TODO: Prepare to read color table 
		printf("[DEBUG] this bmp has a non-zero color_palette, prepare to read its color table \n");
	}

	// num_important_colors (4)
	readBytes = ReadFileWithOffset(hFile, sizeof(uint32_t), *offset, &h->num_important_colors);
	if (readBytes == 0) return 1;
	*offset += readBytes;

	return 0;
}

int ReadBitmapV2InfoHeader(HANDLE hFile, bitmap* bmp, int* offset) {
	if (ReadBitmapInfoHeader(hFile, bmp, offset) != 0) {
		// TODO: failed to read bitmap v1 info header
		return 1;
	}

	bitmap_v5_info_header* h = &bmp->dib_header;

	int readBytes = 0;

	// red_bit_mask (4)
	readBytes = ReadFileWithOffset(hFile, sizeof(uint32_t), *offset, &h->red_bit_mask);
	if (readBytes == 0) return 1;
	*offset += readBytes;

	// green_bit_mask (4)
	readBytes = ReadFileWithOffset(hFile, sizeof(uint32_t), *offset, &h->green_bit_mask);
	if (readBytes == 0) return 1;
	*offset += readBytes;

	// blue_bit_mask (4)
	readBytes = ReadFileWithOffset(hFile, sizeof(uint32_t), *offset, &h->blue_bit_mask);
	if (readBytes == 0) return 1;
	*offset += readBytes;

	return 0;
}

int ReadBitmapV3InfoHeader(HANDLE hFile, bitmap* bmp, int* offset) {
	if (ReadBitmapV2InfoHeader(hFile, bmp, offset) != 0) {
		// TODO: failed to read bitmap v1 info header
		return 1;
	}

	bitmap_v5_info_header* h = &bmp->dib_header;

	int readBytes = 0;

	// alpha_bit_mask (4)
	readBytes = ReadFileWithOffset(hFile, sizeof(uint32_t), *offset, &h->alpha_bit_mask);
	if (readBytes == 0) return 1;
	*offset += readBytes;

	return 0;
}

int ReadBitmapV4InfoHeader(HANDLE hFile, bitmap* bmp, int* offset) {
	if (ReadBitmapV3InfoHeader(hFile, bmp, offset) != 0) {
		// TODO: failed to read bitmap v1 info header
		return 1;
	}

	bitmap_v5_info_header* h = &bmp->dib_header;

	int readBytes = 0;

	// color_space_type (4)
	readBytes = ReadFileWithOffset(hFile, sizeof(uint32_t), *offset, &h->color_space_type);
	if (readBytes == 0) return 1;
	*offset += readBytes;

	// color_space_endpoints (36)
	readBytes = ReadFileWithOffset(hFile, sizeof(cie_xyz_triple), *offset, &h->color_space_endpoints);
	if (readBytes == 0) return 1;
	*offset += readBytes;

	// gamma_red (4)
	readBytes = ReadFileWithOffset(hFile, sizeof(uint32_t), *offset, &h->gamma_red);
	if (readBytes == 0) return 1;
	*offset += readBytes;

	// gamma_green (4)
	readBytes = ReadFileWithOffset(hFile, sizeof(uint32_t), *offset, &h->gamma_green);
	if (readBytes == 0) return 1;
	*offset += readBytes;

	// gamma_blue (4)
	readBytes = ReadFileWithOffset(hFile, sizeof(uint32_t), *offset, &h->gamma_blue);
	if (readBytes == 0) return 1;
	*offset += readBytes;

	return 0;
}

int ReadBitmapV5InfoHeader(HANDLE hFile, bitmap* bmp, int* offset) {
	if (ReadBitmapV4InfoHeader(hFile, bmp, offset) != 0) {
		// TODO: failed to read bitmap v1 info header
		return 1;
	}

	bitmap_v5_info_header* h = &bmp->dib_header;

	int readBytes = 0;

	// intent (4)
	readBytes = ReadFileWithOffset(hFile, sizeof(uint32_t), *offset, &h->intent);
	if (readBytes == 0) return 1;
	*offset += readBytes;

	// icc_profile_data (4)
	readBytes = ReadFileWithOffset(hFile, sizeof(uint32_t), *offset, &h->icc_profile_data);
	if (readBytes == 0) return 1;
	*offset += readBytes;

	// TODO: If bigger then zero, prepare to read icc_profile_data as icc_profile_size

	// icc_profile_size (4)
	readBytes = ReadFileWithOffset(hFile, sizeof(uint32_t), *offset, &h->icc_profile_size);
	if (readBytes == 0) return 1;
	*offset += readBytes;

	// reserved (4)
	readBytes = ReadFileWithOffset(hFile, sizeof(uint32_t), *offset, &h->reserved);
	if (readBytes == 0) return 1;
	*offset += readBytes;

	// TODO: Before read pixels, know its pixel format. 
	// bits_per_pixel, compression and RGBA masks are the factors which determine the pixel format. 
	printf("[DEBUG] read_bitmap_v5_info_header determine its pixel format [ bits_per_pixel: %u compression: %d, RGBA masks: %u %u %u %u ] \n",
		h->bits_per_pixel, h->compression_method, h->red_bit_mask, h->green_bit_mask, h->blue_bit_mask, h->alpha_bit_mask);

	switch (h->compression_method) {
		// There is no compression
	case BI_RGB: {
		// bits per pixel: 24 = 3 bytes
		bmp->pixel_data = (uint8_t**)malloc(h->height * sizeof(uint8_t*));
		uint8_t rgb_masks[3];
		rgb_masks[0] = (h->red_bit_mask >> 16) & 0xFF;
		rgb_masks[1] = (h->green_bit_mask >> 8) & 0xFF;
		rgb_masks[2] = (h->blue_bit_mask) & 0xFF;

		// int padding = 0;
		// Read 1st row
		for (int i = h->height - 1; i > 0; --i) {
			int row_size = h->width * (h->bits_per_pixel / 8); // bytes
			uint8_t* row = (uint8_t*)malloc(row_size); // 1bytes * row_size
			// |<--------------------width------------------->|-------|
			// |Pixel[0, h-1]|Pixel[1, h-1]...|Pixel[w-1, h-1]|Padding|
			int readBytes = ReadFileWithOffset(hFile, sizeof(uint8_t) * row_size, *offset, row);
			if (readBytes != row_size) {
				printf("[ERROR] read pixel data failed read size = %d offset: %d \n", readBytes, *offset);
				return 1;
			}
			else {
				// padding = (row_size % 4); // think it's not necessary
				*offset += (row_size + sizeof(uint32_t));

				// printf("[DEBUG] before masking row: %02X %02X %02X \n", row[0], row[1], row[2]);
				// 0..255 &mpersand with red_bit_mask
				for (int j = 0; j < row_size; j += 3) {
					row[j + 0] = row[j + 0] & rgb_masks[0]; // R
					row[j + 1] = row[j + 1] & rgb_masks[1]; // G
					row[j + 2] = row[j + 2] & rgb_masks[2]; // B
				}
				// printf("[DEBUG] after masking row: %02X %02X %02X \n", row[0], row[1], row[2]);

				((uint8_t**)bmp->pixel_data)[i] = row;
			}
		}
		break;
	}
	case BI_RLE8: {
		// https://en.wikipedia.org/wiki/Run-length_encoding
		// RLE 8-bit/pixel
		// Not supported
		return 1;
		break;
	}
	case BI_RLE4: {
		// RLE 4-bit/pixel
		// Not supported
		return 1;
		break;
	}
	case BI_BITFIELDS: {
		// BITMAP_V2_INFO_HEADER: RGB bit fields mask
		// BITMAP_V3_INFO_HEADER+: RGBA
		// Not supported
		return 1;
		break;
	}
	case BI_JPEG: {
		// RLE-24 BITMAP_V4_INFO_HEADER+: JPEG image for printing
		// Not supported
		return 1;
		break;
	}
	case BI_PNG: {
		// BITMAP_V4_INFO_HEADER+: PNG image for printing
		// Not supported
		return 1;
		break;
	}
	case BI_ALPHABITFIELDS: {
		// Only Windows CE 5.0 with .NET 4.0 or later
		// RGBA bit field masks
		// Not supported
		return 1;
		break;
	}
	case BI_CMYK: {
		// Only Windos Metafile CMYK
		// None
		// Not supported
		return 1;
		break;
	}
	case BI_CMYKRLE8: {
		// Only Windows Metafile CMYK
		// RLE-8
		// Not supported
		return 1;
		break;
	}
	case BI_CMYKRLE4: {
		// Only Windows Metafile CMYK
		// RLE-4
		// Not supported
		return 1;
		break;
	}
	default: break;
	}
	return 0;
}

/// <summary>
/// Read bitmap Device-Independent Bitmap(DIB) header.
/// Not supported header type: BITMAP_CORE_HEADER, OS22X_BITMAP_HEADER_2, OS22X_BITMAP_HEADER_SMALL return 1
/// Supported header type: BITMAP_INFO_HEADER, V2~V5 INFO HEADER return 0
/// If error occurred, return 1
/// </summary>
/// <param name="hFile">File pointer</param>
/// <param name="bmp">Bitmap pointer</param>
/// <param name="offset">File offset</param>
/// <returns>0 if success, otherwise 1</returns>
int ReadBitmapDIBHeader(HANDLE hFile, bitmap* bmp, int* offset)
{
	int result = 0;
	switch (bmp->header_type) {
	case BITMAP_CORE_HEADER: {
		// Not supported
		result = 1;
		break;
	}
	case OS22X_BITMAP_HEADER_2: {
		// Not supported
		result = 1;
		break;
	}
	case OS22X_BITMAP_HEADER_SMALL: {
		// Not supported
		result = 1;
		break;
	}
	case BITMAP_INFO_HEADER: {
		result = ReadBitmapInfoHeader(hFile, bmp, offset);
		break;
	}
	case BITMAP_V2_INFO_HEADER: {
		result = ReadBitmapV2InfoHeader(hFile, bmp, offset);
		break;
	}
	case BITMAP_V3_INFO_HEADER: {
		result = ReadBitmapV3InfoHeader(hFile, bmp, offset);
		break;
	}
	case BITMAP_V4_INFO_HEADER: {
		result = ReadBitmapV4InfoHeader(hFile, bmp, offset);
		break;
	}
	case BITMAP_V5_INFO_HEADER: {
		result = ReadBitmapV5InfoHeader(hFile, bmp, offset);
		break;
	}
	default: {
		break;
	}
	}
	return result;
}

int ReadBitmap(HANDLE hFile, bitmap* bmp)
{
	int offset = 0;

	if (ReadBitmapHeaderInfo(hFile, bmp, &offset) != 0)
	{
		printf("[ERROR] failed to read bitmap header info \n");
		return 1;
	}

	int readBytes = ReadFileWithOffset(hFile, sizeof(bitmap_header_type), offset, &bmp->header_type);
	if (readBytes == 0) {
		printf("[ERROR] failed to read dib header size \n");
		return 1;
	}
	offset += readBytes;

	readBytes = ReadBitmapDIBHeader(hFile, bmp, &offset);
	if (readBytes != 0) {
		printf("[ERROR] failed tp read dib_header \n");
		return 1;
	}
	offset += readBytes;

	return 0;
}
