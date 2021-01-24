/*
 * Copyright (c) 2012 Stefano Sabatini
 * Copyright (c) 2014 Clément Bœsch
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <libavformat/avformat.h>
#include <libavutil/motion_vector.h>

static AVFormatContext *fmt_ctx = NULL;
static AVCodecContext *video_dec_ctx = NULL;
static AVStream *video_stream = NULL;
static const char *src_filename = NULL;

static int video_stream_idx = -1;
static AVFrame *frame = NULL;
static int video_frame_count = 0;

// static int decode_packet(const AVPacket *pkt) {
//   int ret = avcodec_send_packet(video_dec_ctx, pkt);
//   if (ret < 0) {
//     fprintf(stderr, "Error while sending a packet to the decoder: %s\n",
//             av_err2str(ret));
//     return ret;
//   }

//   while (ret >= 0) {
//     ret = avcodec_receive_frame(video_dec_ctx, frame);
//     if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
//       break;
//     } else if (ret < 0) {
//       fprintf(stderr, "Error while receiving a frame from the decoder: %s\n",
//               av_err2str(ret));
//       return ret;
//     }

//     if (ret >= 0) {
//       int i;
//       AVFrameSideData *sd;

//       video_frame_count++;
//       sd = av_frame_get_side_data(frame, AV_FRAME_DATA_MOTION_VECTORS);
//       if (sd) {
//         const AVMotionVector *mvs = (const AVMotionVector *)sd->data;
//         for (i = 0; i < sd->size / sizeof(*mvs); i++) {
//           const AVMotionVector *mv = &mvs[i];
//           printf("%d,%2d,%2d,%2d,%4d,%4d,%4d,%4d,0x%" PRIx64 "\n",
//                  video_frame_count, mv->source, mv->w, mv->h, mv->src_x,
//                  mv->src_y, mv->dst_x, mv->dst_y, mv->flags);
//         }
//       }
//       av_frame_unref(frame);
//     }
//   }

//   return 0;
// }

static int decode_packet_v2(const AVPacket *pkt, int **out, int **out_source,
                            int width, int height) {

  int ret = avcodec_send_packet(video_dec_ctx, pkt);

  width = (width + 15) / 16;
  height = (height + 15) / 16;

  if (ret < 0) {
    fprintf(stderr, "Error while sending a packet to the decoder: %s\n",
            av_err2str(ret));
    return ret;
  }

  while (ret >= 0) {
    ret = avcodec_receive_frame(video_dec_ctx, frame);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
      break;
    } else if (ret < 0) {
      fprintf(stderr, "Error while receiving a frame from the decoder: %s\n",
              av_err2str(ret));
      return ret;
    }

    if (ret >= 0) {
      int i;
      AVFrameSideData *sd;

      video_frame_count++;

      // Realloc the full vector to the new video size
      *out = (int *)realloc(
          *out, video_frame_count * width * height * 2 * sizeof(int));
      *out_source = (int *)realloc(
          *out_source, video_frame_count * width * height * sizeof(int));

      // Set all the new values to 0
      for (size_t k = (video_frame_count - 1) * width * height * 2;
           k < video_frame_count * width * height * 2; ++k) {
        (*out)[k] = 0;
      }

      for (size_t k = (video_frame_count - 1) * width * height;
           k < video_frame_count * width * height; ++k) {
        (*out_source)[k] = 0;
      }

      sd = av_frame_get_side_data(frame, AV_FRAME_DATA_MOTION_VECTORS);
      if (sd) {
        const AVMotionVector *mvs = (const AVMotionVector *)sd->data;
        // Set all the values to
        for (i = 0; i < sd->size / sizeof(*mvs); i++) {
          const AVMotionVector *mv = &mvs[i];

          // Set the motion vector
          (*out)[(video_frame_count - 1) * width * height * 2 +
                 (mv->dst_y / 16) * height * 2 + (mv->dst_x / 16) * 2] =
              mv->dst_x - mv->src_x;
          (*out)[(video_frame_count - 1) * width * height * 2 +
                 (mv->dst_y / 16) * height * 2 + (mv->dst_x / 16) * 2 + 1] =
              mv->dst_y - mv->src_y;

          // Set the relative frames
          (*out_source)[(video_frame_count - 1) * width * height +
                        (mv->dst_y / 16) * height + (mv->dst_x / 16)] =
              mv->source;
        }
      }
      av_frame_unref(frame);
    }
  }
  return 0;
}

static int open_codec_context(AVFormatContext *fmt_ctx, enum AVMediaType type) {
  int ret;
  AVStream *st;
  AVCodecContext *dec_ctx = NULL;
  AVCodec *dec = NULL;
  AVDictionary *opts = NULL;

  ret = av_find_best_stream(fmt_ctx, type, -1, -1, &dec, 0);
  if (ret < 0) {
    fprintf(stderr, "Could not find %s stream in input file '%s'\n",
            av_get_media_type_string(type), src_filename);
    return ret;
  } else {
    int stream_idx = ret;
    st = fmt_ctx->streams[stream_idx];

    dec_ctx = avcodec_alloc_context3(dec);
    if (!dec_ctx) {
      fprintf(stderr, "Failed to allocate codec\n");
      return AVERROR(EINVAL);
    }

    ret = avcodec_parameters_to_context(dec_ctx, st->codecpar);
    if (ret < 0) {
      fprintf(stderr, "Failed to copy codec parameters to codec context\n");
      return ret;
    }

    /* Init the video decoder */
    av_dict_set(&opts, "flags2", "+export_mvs", 0);
    if ((ret = avcodec_open2(dec_ctx, dec, &opts)) < 0) {
      fprintf(stderr, "Failed to open %s codec\n",
              av_get_media_type_string(type));
      return ret;
    }

    video_stream_idx = stream_idx;
    video_stream = fmt_ctx->streams[video_stream_idx];
    video_dec_ctx = dec_ctx;
  }

  return 0;
}

void extract_mvs(char *filename, int **out, int *out_dim1, int *out_dim2,
                 int *out_dim3, int *out_dim4, int **out_source,
                 int *out_source_dim1, int *out_source_dim2,
                 int *out_source_dim3, int *out_source_dim4) {
  int ret = 0, save = 0, count = -1;

  int width = -1, height = -1;

  fmt_ctx = NULL;
  video_dec_ctx = NULL;
  video_stream = NULL;

  video_stream_idx = -1;
  frame = NULL;
  video_frame_count = 0;

  AVPacket pkt = {0};

  av_register_all();
  avcodec_register_all();

  if (avformat_open_input(&fmt_ctx, filename, NULL, NULL) < 0) {
    fprintf(stderr, "%d\n",
            avformat_open_input(&fmt_ctx, filename, NULL, NULL));
    fprintf(stderr, "Could not open source file %s\n", filename);
    exit(1);
  }

  if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
    fprintf(stderr, "Could not find stream information\n");
    exit(1);
  }

  open_codec_context(fmt_ctx, AVMEDIA_TYPE_VIDEO);

  // av_dump_format(fmt_ctx, 0, filename, 0);

  if (!video_stream) {
    fprintf(stderr, "Could not find video stream in the input, aborting\n");
    ret = 1;
    goto end;
  }

  frame = av_frame_alloc();
  if (!frame) {
    fprintf(stderr, "Could not allocate frame\n");
    ret = AVERROR(ENOMEM);
    goto end;
  }

  width = (video_dec_ctx->width + 15) / 16;
  height = (video_dec_ctx->height + 15) / 16;

  /* read frames from the file */
  while (av_read_frame(fmt_ctx, &pkt) >= 0) {
    
    if (pkt.stream_index == video_stream_idx) {
      ret = decode_packet_v2(&pkt, out, out_source, video_dec_ctx->width,
                             video_dec_ctx->height);
    }
    av_packet_unref(&pkt);
    if (ret < 0) {
      break;
    }
  }
  /* flush cached frames */
  decode_packet_v2(NULL, out, out_source, video_dec_ctx->width,
                   video_dec_ctx->height);
end:
  avcodec_free_context(&video_dec_ctx);
  avformat_close_input(&fmt_ctx);
  av_frame_free(&frame);

  *out_dim1 = video_frame_count;
  *out_dim2 = height;
  *out_dim3 = width;
  *out_dim4 = 2;

  *out_source_dim1 = video_frame_count;
  *out_source_dim2 = height;
  *out_source_dim3 = width;
  *out_source_dim4 = 1;
}