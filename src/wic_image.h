#ifndef WIC_IMAGE_H
#define WIC_IMAGE_H

// image decoder and encoder with WIC
#include <wincodec.h>
#include <string>
#include <iostream>
#include <stdio.h>
#include "crc32.h"

template <typename T>
inline void ReverseBytes(T& val)
{
    const T copyVal = val;

    unsigned char* ptr = reinterpret_cast<unsigned char*>(&val);
    const unsigned char* copyPtr = reinterpret_cast<const unsigned char*>(&copyVal);
    for (size_t i = 0; i < sizeof(T); ++i)
        ptr[i] = copyPtr[sizeof(T) - i - 1];
}

unsigned char* wic_decode_image_by_data(const unsigned char * fileData, unsigned int fileSize, int* w, int* h, int* c)
{
    IWICImagingFactory* factory = 0;
    IWICBitmapDecoder* decoder = 0;
    IWICBitmapFrameDecode* frame = 0;
    WICPixelFormatGUID pixel_format;
    IWICFormatConverter* converter = 0;
    IWICBitmap* bitmap = 0;
    IWICBitmapLock* lock = 0;
    IWICStream* pStream;
    int width = 0;
    int height = 0;
    int channels = 0;
    WICRect rect = { 0, 0, 0, 0 };
    unsigned int datasize = 0;
    unsigned char* data = 0;
    int stride = 0;
    unsigned char* bgrdata = 0;

    if (CoCreateInstance(CLSID_WICImagingFactory1, 0, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory)))
        goto RETURN;

    if (factory->CreateStream(&pStream))
    {
        goto RETURN;
    }
    if (pStream->InitializeFromMemory((BYTE*)fileData, fileSize))
    {
        goto RETURN;
    }

    if (factory->CreateDecoderFromStream(pStream, NULL, WICMETADATACACHEOPTION_FORCE_DWORD, &decoder))
    {
        goto RETURN;
    }

    if (decoder->GetFrame(0, &frame))
        goto RETURN;

    if (factory->CreateFormatConverter(&converter))
        goto RETURN;

    if (frame->GetPixelFormat(&pixel_format))
        goto RETURN;

    if (!IsEqualGUID(pixel_format, GUID_WICPixelFormat32bppBGRA))
        pixel_format = GUID_WICPixelFormat24bppBGR;

    channels = IsEqualGUID(pixel_format, GUID_WICPixelFormat32bppBGRA) ? 4 : 3;

    if (converter->Initialize(frame, pixel_format, WICBitmapDitherTypeNone, 0, 0.0, WICBitmapPaletteTypeCustom))
        goto RETURN;

    if (factory->CreateBitmapFromSource(converter, WICBitmapCacheOnDemand, &bitmap))
        goto RETURN;

    if (bitmap->GetSize((UINT*)&width, (UINT*)&height))
        goto RETURN;

    rect.Width = width;
    rect.Height = height;
    if (bitmap->Lock(&rect, WICBitmapLockRead, &lock))
        goto RETURN;
    
    if (lock->GetDataPointer(&datasize, &data))
        goto RETURN;

    if (lock->GetStride((UINT*)&stride))
        goto RETURN;

    bgrdata = (unsigned char*)malloc(width * height * channels);
    if (!bgrdata)
        goto RETURN;

    for (int y = 0; y < height; y++)
    {
        const unsigned char* ptr = data + y * stride;
        unsigned char* bgrptr = bgrdata + y * width * channels;
        memcpy(bgrptr, ptr, width * channels);
    }

    *w = width;
    *h = height;
    *c = channels;

RETURN:
    if (lock) lock->Release();
    if (bitmap) bitmap->Release();
    if (decoder) decoder->Release();
    if (frame) frame->Release();
    if (converter) converter->Release();
    if (factory) factory->Release();

    return bgrdata;
}

int wic_encode_image_to_data(int w, int h, int c, void* bgrdata, void*& out, unsigned long& outSize, unsigned long modelIndex, unsigned long toW, unsigned long toH, unsigned long oldW, unsigned long oldH)
{
    IWICImagingFactory* factory = 0;
    IWICStream* stream = 0;
    IWICBitmapEncoder* encoder = 0;
    IWICBitmapFrameEncode* frame = 0;
    IWICBitmap *bitmap2 = 0;
    IWICBitmapScaler* scale = 0;
    WICPixelFormatGUID format = c == 4 ? GUID_WICPixelFormat32bppBGRA : GUID_WICPixelFormat24bppBGR;
    int stride = (w * c * 8 + 7) / 8;
    unsigned char* data = 0;
    int ret = 0;
    IStream* streamIn;
    CreateStreamOnHGlobal(NULL, true, &streamIn);

    if (CoCreateInstance(CLSID_WICImagingFactory1, 0, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory)))
        goto RETURN;

    if (factory->CreateStream(&stream))
        goto RETURN;

    if (stream->InitializeFromIStream(streamIn))
        goto RETURN;

    if (factory->CreateEncoder(GUID_ContainerFormatPng, 0, &encoder))
        goto RETURN;

    if (encoder->Initialize(stream, WICBitmapEncoderNoCache))
        goto RETURN;

    if (encoder->CreateNewFrame(&frame, 0))
        goto RETURN;

    if (frame->Initialize(0))
        goto RETURN;

    if (factory->CreateBitmapScaler(&scale))
        goto RETURN;

    if (factory->CreateBitmapFromMemory(w, h, format, stride, stride * h, (BYTE*)bgrdata, &bitmap2))
        goto RETURN;

    if (scale->Initialize(bitmap2, toW, toH, WICBitmapInterpolationModeFant))
        goto RETURN;

    if (frame->SetSize((UINT)toW, (UINT)toH))
        goto RETURN;

    if (frame->SetPixelFormat(&format))
        goto RETURN;

    if (!IsEqualGUID(format, c == 4 ? GUID_WICPixelFormat32bppBGRA : GUID_WICPixelFormat24bppBGR))
        goto RETURN;
    
    //if (frame->WritePixels(h, stride, h * stride, (BYTE *)bgrdata))
        //goto RETURN;
    if (frame->WriteSource(scale, NULL))
        goto RETURN;

    if (frame->Commit())
        goto RETURN;

    if (encoder->Commit())
        goto RETURN;
    
    ret = 1;

RETURN:
    if (ret == 1) {
        STATSTG sts;
        stream->Stat(&sts, STATFLAG_DEFAULT);
        ULARGE_INTEGER uli = sts.cbSize;
        LARGE_INTEGER zero;
        zero.QuadPart = 0;
        unsigned long outSize2 = (ULONG)uli.QuadPart;

        unsigned int extSize = 4 + 4 + 4;

        int extChunkSize = extSize + 12;

        out = malloc(outSize2 + extChunkSize);
        outSize = outSize2 + extChunkSize;

        unsigned long start = 8 + 25;
        unsigned long end = outSize2 - start;

        ULONG written;
        stream->Seek(zero, STREAM_SEEK_SET, NULL);
        stream->Read(out, start, &written);
           
        ReverseBytes(extSize);

        unsigned char insertData[16];
        memcpy(insertData, "tEXt", 4);
        memcpy(insertData + 4, (unsigned char *)&modelIndex, 4);
        memcpy(insertData + 8, (unsigned char *)&oldW, 4);
        memcpy(insertData + 12, (unsigned char *)&oldH, 4);
        
        unsigned int crc = crc32((const unsigned char *)insertData, 16);
        ReverseBytes(crc);

        memcpy(((unsigned char *)out + start), (unsigned char*)&extSize, 4);
        memcpy(((unsigned char*)out + start + 4), insertData, 16);
        memcpy(((unsigned char*)out + start + 4 + 16), (unsigned char *)&crc, 4);
        stream->Read(((unsigned char*)out + start + 8 + 16), end, &written);
    }
    if (scale) scale->Release();
    if (bitmap2) bitmap2->Release();
    if (data) free(data);
    if (encoder) encoder->Release();
    if (frame) frame->Release();
    if (stream) stream->Release();
    if (streamIn) streamIn->Release();
    if (factory) factory->Release();

    return ret;
}
int wic_encode_jpeg_image_to_data(int w, int h, int c, void* bgrdata, void *& out, unsigned long & outSize, unsigned long modelIndex, unsigned long toW, unsigned long toH, unsigned long oldW, unsigned long oldH)
{
    // assert c == 3

    IWICImagingFactory* factory = 0;
    IWICStream* stream = 0;
    IWICBitmapEncoder* encoder = 0;
    IWICBitmapFrameEncode* frame = 0;
    IPropertyBag2* propertybag = 0;
    WICPixelFormatGUID format = GUID_WICPixelFormat24bppBGR;
    IStream *streamData;
    IWICBitmap* bitmap2 = 0;
    IWICBitmapScaler* scale = 0;
    CreateStreamOnHGlobal(NULL, true, &streamData);
    int stride = (w * c * 8 + 7) / 8;
    int ret = 0;

    PROPBAG2 option = { 0 };
    option.pstrName = L"ImageQuality";
    VARIANT varValue;
    VariantInit(&varValue);
    varValue.vt = VT_R4;
    varValue.fltVal = 1.0f;
    unsigned long size = h * stride;
    IStream *streamIn;
    CreateStreamOnHGlobal(NULL, true, &streamIn);

    if (CoCreateInstance(CLSID_WICImagingFactory1, 0, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory)))
        goto RETURN;

    if (factory->CreateStream(&stream))
        goto RETURN;

    if (stream->InitializeFromIStream(streamIn))
        goto RETURN;

    if (factory->CreateEncoder(GUID_ContainerFormatJpeg, 0, &encoder))
        goto RETURN;

    if (encoder->Initialize(stream, WICBitmapEncoderNoCache))
        goto RETURN;

    if (encoder->CreateNewFrame(&frame, &propertybag))
        goto RETURN;

    if (propertybag->Write(1, &option, &varValue))
        goto RETURN;

    if (frame->Initialize(propertybag))
        goto RETURN;
    
    if (factory->CreateBitmapScaler(&scale))
        goto RETURN;

    if (factory->CreateBitmapFromMemory(w, h, format, stride, stride * h, (BYTE*)bgrdata, &bitmap2))
        goto RETURN;

    if (scale->Initialize(bitmap2, toW, toH, WICBitmapInterpolationModeFant))
        goto RETURN;

    if (frame->SetSize((UINT)toW, (UINT)toH))
        goto RETURN;

    if (frame->SetPixelFormat(&format))
        goto RETURN;

    if (!IsEqualGUID(format, GUID_WICPixelFormat24bppBGR))
        goto RETURN;

    //if (frame->WritePixels(h, stride, h * stride, (BYTE *)bgrdata))
        //goto RETURN;
    if (frame->WriteSource(scale, NULL))
        goto RETURN;

    if (frame->Commit())
        goto RETURN;

    if (encoder->Commit())
        goto RETURN;
    
    ret = 1;

RETURN:

    if (ret == 1) {
        STATSTG sts;
        stream->Stat(&sts, STATFLAG_DEFAULT);
        ULARGE_INTEGER uli = sts.cbSize;
        LARGE_INTEGER zero;
        zero.QuadPart = 0;
        unsigned long outSize2 = (ULONG)uli.QuadPart;

        int insertSize = 12;
        int modelSize = insertSize + 4;
        out = malloc(outSize2 + modelSize);
        outSize = outSize2 + modelSize;

        unsigned long start = 20;
        unsigned long end = outSize2 - start;

        ULONG written;
        stream->Seek(zero, STREAM_SEEK_SET, NULL);
        stream->Read(out, start, &written);

        UINT16 modelSize2 = insertSize + 2;
        ReverseBytes(modelSize2);

        unsigned char insertData[12];
        memcpy(insertData, (unsigned char*)&modelIndex, 4);
        memcpy(insertData + 4, (unsigned char*)&oldW, 4);
        memcpy(insertData + 8, (unsigned char*)&oldH, 4);

        UINT16 head = 0xfeff;

        memcpy(((unsigned char*)out + start), (unsigned char*)&head, 2);
        memcpy(((unsigned char*)out + start + 2), (unsigned char*)&modelSize2, 2);
        memcpy(((unsigned char*)out + start + 4), insertData, insertSize);
        stream->Read(((unsigned char*)out + start + 4 + insertSize), end, &written);
    }
    if (scale) scale->Release();
    if (bitmap2) bitmap2->Release();
    if (encoder) encoder->Release();
    if (frame) frame->Release();
    if (propertybag) propertybag->Release();
    if (stream) stream->Release();
    if (streamIn) streamIn->Release();
    if (factory) factory->Release();

    return ret;
}

#endif // WIC_IMAGE_H
