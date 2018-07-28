#include "SDL.h"
#include "SDL_gpu.h"

#ifndef SDL_GPU_BUILD_VIDEO_TEST

int main(int argc, char* argv[])
{
    GPU_LogError("Sorry, this demo needs to be specifically enabled because it requires FFMPEG.\n\nEither enable the SDL_gpu_BUILD_VIDEO_TEST CMake flag or else define SDL_GPU_BUILD_VIDEO_TEST to build this demo manually.\n");
    return 0;
}

#else

// Based on http://dranger.com/ffmpeg/ffmpeg.html but updated for latest ffmpeg.
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"

#ifdef SDL_GPU_USE_SDL2

int main(int argc, char* argv[])
{
    GPU_SetDebugLevel(GPU_DEBUG_LEVEL_MAX);
    GPU_Log("register_all\n");
    
    av_register_all();
    
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
        return -1;
    
    const char* filename = "video.avi";
    //if(argc > 1)
    //    filename = argv[1];
    
    AVFormatContext* pFormatCtx = NULL;
    GPU_Log("open_input\n");
    if(avformat_open_input(&pFormatCtx, filename, NULL, NULL)!=0)
        return -2;
    
    GPU_Log("find_stream_info\n");
    if(avformat_find_stream_info(pFormatCtx, NULL) < 0)
        return -2;
    
    int videoStream = -1;
    int i;
    for(i = 0; i < pFormatCtx->nb_streams; i++)
    {
        if(pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStream = i;
            break;
        }
    }
    
    if(videoStream == -1)
        return -3;

    // Get a pointer to the codec context for the video stream
    AVCodecContext* pCodecCtx = pFormatCtx->streams[videoStream]->codec;
    
    // Find the decoder for the video stream
    AVCodec* pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if(pCodec == NULL)
    {
        fprintf(stderr, "Unsupported codec!\n");
        return -1; // Codec not found
    }

    // Open codec
    if(avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
        return -1; // Could not open codec

    // Allocate video frame
    AVFrame* pFrame = av_frame_alloc();

    // Make a screen to put our video
    SDL_Window* window = SDL_CreateWindow("",
                                  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                  800, 600,
                                  0);
    
    if(window == NULL)
    {
        fprintf(stderr, "SDL: could not create window - exiting\n");
        exit(1);
    }
    
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_Texture* bmp;
    
    if(renderer == NULL)
    {
        fprintf(stderr, "SDL: could not create renderer - exiting\n");
        exit(1);
    }

    // Allocate a place to put our YUV image on that screen
    bmp = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_YV12, SDL_TEXTUREACCESS_STATIC, pCodecCtx->width, pCodecCtx->height);


    // Read frames and save first five frames to disk
    AVPacket packet;
    int frameFinished;
    SDL_Event event;
    SDL_Rect rect;
    i = 0;
    
    int playing = (av_read_frame(pFormatCtx, &packet) >= 0);
    Uint8 done = 0;
    while(!done)
    {
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_QUIT:
                    done = 1;
                    break;
                case SDL_KEYDOWN:
                    if(event.key.keysym.sym == SDLK_ESCAPE)
                        done = 1;
                    else if(event.key.keysym.sym == SDLK_SPACE)
                    {
                        av_seek_frame(pFormatCtx, 0, 0, 0);
                        playing = (av_read_frame(pFormatCtx, &packet) >= 0);
                    }
                    break;
                default:
                    break;
            }
        }
        
        if(playing)
        {
            // Is this a packet from the video stream?
            if(packet.stream_index == videoStream)
            {
                // Decode video frame
                avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, 
                       &packet);
              
                // Did we get a video frame?
                if(frameFinished)
                {
                    SDL_UpdateYUVTexture(bmp, NULL, pFrame->data[0], pFrame->linesize[0], 
                                pFrame->data[1], pFrame->linesize[0], 
                                pFrame->data[2], pFrame->linesize[0]);  // FIXME: Why are linesize[1] and linesize[2] the wrong values?

                    rect.x = 0;
                    rect.y = 0;
                    rect.w = pCodecCtx->width;
                    rect.h = pCodecCtx->height;
                    
                    SDL_RenderClear(renderer);
                    SDL_RenderCopy(renderer, bmp, NULL, &rect);
                    SDL_RenderPresent(renderer);
                }
            }

            // Free the packet that was allocated by av_read_frame
            av_free_packet(&packet);
            
            // Get next frame
            playing = (av_read_frame(pFormatCtx, &packet) >= 0);
        }
        
        SDL_Delay(30);
    }

    av_free_packet(&packet);
    
    // Free the YUV frame
    av_free(pFrame);

    // Close the codec
    avcodec_close(pCodecCtx);

    // Close the video file
    avformat_close_input(&pFormatCtx);
    
    SDL_Quit();
    
	return 0;
}

#else

int main(int argc, char* argv[])
{
    GPU_SetDebugLevel(GPU_DEBUG_LEVEL_MAX);
    GPU_Log("register_all\n");
    av_register_all();
    
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
        return -1;
    
    const char* filename = "video.avi";
    //if(argc > 1)
    //    filename = argv[1];
    
    AVFormatContext* pFormatCtx = NULL;
    GPU_Log("open_input\n");
    if(avformat_open_input(&pFormatCtx, filename, NULL, NULL)!=0)
        return -2;
    
    GPU_Log("find_stream_info\n");
    if(avformat_find_stream_info(pFormatCtx, NULL) < 0)
        return -2;
    
    int videoStream = -1;
    int i;
    for(i = 0; i < pFormatCtx->nb_streams; i++)
    {
        if(pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStream = i;
            break;
        }
    }
    
    if(videoStream == -1)
        return -3;

    // Get a pointer to the codec context for the video stream
    AVCodecContext* pCodecCtx = pFormatCtx->streams[videoStream]->codec;
    
    // Find the decoder for the video stream
    AVCodec* pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if(pCodec == NULL)
    {
        fprintf(stderr, "Unsupported codec!\n");
        return -1; // Codec not found
    }

    // Open codec
    if(avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
        return -1; // Could not open codec

    // Allocate video frame
    AVFrame* pFrame = av_frame_alloc();

    // Make a screen to put our video
    SDL_Surface* screen = SDL_SetVideoMode(800, 600, 0, SDL_SWSURFACE);
    SDL_Overlay* bmp;
    
    if(screen == NULL)
    {
        fprintf(stderr, "SDL: could not set video mode - exiting\n");
        exit(1);
    }

    // Allocate a place to put our YUV image on that screen
    bmp = SDL_CreateYUVOverlay(pCodecCtx->width,
                 pCodecCtx->height,
                 SDL_YV12_OVERLAY,
                 screen);


    // Read frames and save first five frames to disk
    AVPacket packet;
    int frameFinished;
    SDL_Event event;
    SDL_Rect rect;
    i = 0;
    
    int playing = (av_read_frame(pFormatCtx, &packet) >= 0);
    Uint8 done = 0;
    while(!done)
    {
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_QUIT:
                    done = 1;
                    break;
                case SDL_KEYDOWN:
                    if(event.key.keysym.sym == SDLK_ESCAPE)
                        done = 1;
                    else if(event.key.keysym.sym == SDLK_SPACE)
                    {
                        av_seek_frame(pFormatCtx, 0, 0, 0);
                        playing = (av_read_frame(pFormatCtx, &packet) >= 0);
                    }
                    break;
                default:
                    break;
            }
        }
        
        if(playing)
        {
            // Is this a packet from the video stream?
            if(packet.stream_index == videoStream)
            {
                // Decode video frame
                avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, 
                       &packet);
              
                // Did we get a video frame?
                if(frameFinished)
                {
                    SDL_LockYUVOverlay(bmp);

                    AVPicture pict;
                    pict.data[0] = bmp->pixels[0];
                    pict.data[1] = bmp->pixels[2];
                    pict.data[2] = bmp->pixels[1];
                    
                    pict.linesize[0] = bmp->pitches[0];
                    pict.linesize[1] = bmp->pitches[2];
                    pict.linesize[2] = bmp->pitches[1];
                    
                    // Convert the image into YUV format that SDL uses
                    {
                        struct SwsContext *img_convert_ctx;
                        int w = pCodecCtx->width;
                        int h = pCodecCtx->height;
                        img_convert_ctx = sws_getContext(w, h,
                        pCodecCtx->pix_fmt,
                        w, h, PIX_FMT_YUV420P, SWS_BICUBIC,
                        NULL, NULL, NULL);

                        sws_scale(img_convert_ctx,
                            (const uint8_t *const*)pFrame->data, pFrame->linesize,
                            0, pCodecCtx->height,
                            pict.data, pict.linesize);
                        
                        sws_freeContext(img_convert_ctx);
                    }

                    SDL_UnlockYUVOverlay(bmp);

                    rect.x = 0;
                    rect.y = 0;
                    rect.w = pCodecCtx->width;
                    rect.h = pCodecCtx->height;
                    SDL_DisplayYUVOverlay(bmp, &rect);
                }
            }

            // Free the packet that was allocated by av_read_frame
            av_free_packet(&packet);
            
            // Get next frame
            playing = (av_read_frame(pFormatCtx, &packet) >= 0);
        }
        
        SDL_Delay(30);
    }

    av_free_packet(&packet);
    
    // Free the YUV frame
    av_free(pFrame);

    // Close the codec
    avcodec_close(pCodecCtx);

    // Close the video file
    avformat_close_input(&pFormatCtx);
    
    SDL_Quit();
    
	return 0;
}

#endif

#endif
