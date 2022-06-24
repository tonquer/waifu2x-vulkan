#ifndef WEBP_IMAGE_H
#define WEBP_IMAGE_H

// webp image decoder and encoder with libwebp
#include <stdio.h>
#include <stdlib.h>
#include "webp/decode.h"
#include "webp/encode.h"
#include "webp/demux.h"
#include "webp/mux.h"

bool webp_load(Task &v)
{
    unsigned char* pixeldata = 0;

    WebPDecoderConfig config;
    WebPInitDecoderConfig(&config);

    if (WebPGetFeatures((uint8_t *)v.fileDate, v.fileSize, &config.input) != VP8_STATUS_OK)
        return false;

    if (config.input.has_animation)
    {
        return false;
    }

    int width = config.input.width;
    int height = config.input.height;
    int channels = config.input.has_alpha ? 4 : 3;

    pixeldata = (unsigned char*)malloc(width * height * channels);

    config.output.colorspace = channels == 4 ? MODE_RGBA : MODE_RGB;

    config.output.u.RGBA.stride = width * channels;
    config.output.u.RGBA.size = width * height * channels;
    config.output.u.RGBA.rgba = pixeldata;
    config.output.is_external_memory = 1;
    ncnn::Mat * inimage = new ncnn::Mat();
    inimage->create(width, height, (size_t)channels, channels);

    VP8StatusCode code = WebPDecode((uint8_t *)inimage->data, inimage->total()*channels, &config);
    if (code != VP8_STATUS_OK)
    {
        free(pixeldata);
        inimage->release();
        delete inimage;
        return false;
    }
    v.inImage.push_back(inimage);
    return true;
}

bool webp_load_ani(Task& v)
{
    bool ok = false;
    int dump_ok = 1;
    uint32_t frame_index = 0;
    int prev_frame_timestamp = 0;
    WebPAnimDecoder* dec;
    WebPAnimInfo anim_info;
    WebPAnimDecoderOptions opt;
    const int kNumChannels = 4;
    int webp_status = 0;
    WebPDecoderConfig decoder_config;
    int duration, timestamp;

    memset(&opt, 0, sizeof(opt));
    const WebPData webp_data = {(unsigned char*) v.fileDate, v.fileSize };

    opt.color_mode = MODE_RGBA;
    opt.use_threads = 0;
    dec = WebPAnimDecoderNew(&webp_data, &opt);
    if (dec == NULL) {
        //WFPRINTF(stderr, "Error parsing image: %s\n", (const W_CHAR*)filename);
        goto End;
    }
    // Main object storing the configuration for advanced decoding
    // Initialize the configuration as empty
    // This function must always be called first, unless WebPGetFeatures() is to be called
    if (!WebPInitDecoderConfig(&decoder_config)) {
        goto End;
    }
    webp_status = WebPGetFeatures(webp_data.bytes, webp_data.size, &decoder_config.input);
    if (webp_status != VP8_STATUS_OK) {
        goto End;
    }
    if (!WebPAnimDecoderGetInfo(dec, &anim_info)) {
        fprintf(stderr, "Error getting global info about the animation\n");
        goto End;
    }
    while (WebPAnimDecoderHasMoreFrames(dec)) {
        //DecodedFrame* curr_frame;
        uint8_t* frame_rgba;

        if (!WebPAnimDecoderGetNext(dec, &frame_rgba, &timestamp)) {
            fprintf(stderr, "Error decoding frame #%u\n", frame_index);
            goto End;
        }
        // assert(frame_index < anim_info.frame_count);
        duration = timestamp - prev_frame_timestamp;
        
        ncnn::Mat* inimage = new ncnn::Mat();
        inimage->create(anim_info.canvas_width, anim_info.canvas_height, (size_t)kNumChannels, kNumChannels);
        memcpy(inimage->data, frame_rgba,
            anim_info.canvas_width * kNumChannels * anim_info.canvas_height);

        v.inImage.push_back(inimage);
        v.inFrame.push_back(duration);

        ++frame_index;
        prev_frame_timestamp = timestamp;
    }
    ok = true;
End:
    WebPAnimDecoderDelete(dec);
    return ok;
}


bool webp_save_ani(Task &v)
{
    bool ok = false;
    int dump_ok = 1;
    uint32_t frame_index = 0;
    int prev_frame_timestamp = 0;
    const int kNumChannels = 4;
    WebPMuxError err;

    WebPMuxFrameInfo frame;
    frame.x_offset = 0;
    frame.y_offset = 0;
    frame.id = WEBP_CHUNK_ANMF;
    frame.dispose_method = WEBP_MUX_DISPOSE_NONE;
    frame.blend_method = WEBP_MUX_NO_BLEND;
    
    uint8_t* outb = 0;
    size_t size = 0;
    WebPMuxAnimParams params;
    WebPData outputData;
    std::list<int>::iterator j;
    std::list<ncnn::Mat*>::iterator i;
    WebPMux* mux = WebPMuxNew();
    if (!mux) {
        goto End;
    }
    for (i = v.outImage.begin(), j = v.inFrame.begin(); i != v.outImage.end(); i++, j++)
    {
        ncnn::Mat & inimage = **i;
        int duration= *j;

        size = WebPEncodeLosslessRGBA((unsigned char*)inimage.data, inimage.w, inimage.h, inimage.w * inimage.elemsize, &outb);

        const WebPData webp_data = { outb, size };
        frame.duration = duration;
        frame.bitstream = webp_data;
        err = WebPMuxPushFrame(mux, &frame, 0);
        if(err != WEBP_MUX_OK)
        {
            goto End;
        }
    }
    err = WebPMuxSetCanvasSize(mux, v.toW, v.toH);
    if (err != WEBP_MUX_OK) goto End;

    params.bgcolor = 0;
    params.loop_count = 0;
    WebPMuxSetAnimationParams(mux, &params);
    err = WebPMuxAssemble(mux, &outputData);
    if (err != WEBP_MUX_OK) {
        goto End;
    }

    v.out = (void *)outputData.bytes;
    v.outSize = outputData.size;
    ok = true;
End:
    WebPMuxDelete(mux);
    return ok;
}

bool webp_save(Task &v)
{
    bool ok = false;

    size_t length = 0;
    unsigned char* output = 0;
    ncnn::Mat& outimage = **v.outImage.begin();
    
    if (outimage.elemsize == 3)
    {
        length = WebPEncodeLosslessRGB((unsigned char*)outimage.data, outimage.w, outimage.h, outimage.w * outimage.elemsize, &output);
    }
    else if (outimage.elemsize == 4)
    {
        length = WebPEncodeLosslessRGBA((unsigned char*)outimage.data, outimage.w, outimage.h, outimage.w * outimage.elemsize, &output);
    }
    else
    {
        // unsupported channel type
    }

    if (length == 0)
        goto RETURN;

    v.out = (void*)output;
    v.outSize = length;
    ok = true;
    
RETURN:
    return ok;
}

#endif // WEBP_IMAGE_H
