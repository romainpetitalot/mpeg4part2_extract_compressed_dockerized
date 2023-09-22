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
#include <libavcodec/avcodec.h>


static AVFormatContext *fmt_ctx = NULL;
static AVCodecContext *video_dec_ctx = NULL;
static AVStream *video_stream = NULL;
static const char *src_filename = NULL;

static int video_stream_idx = -1;
static AVFrame *frame = NULL;
static int video_frame_count = 0;

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
                 (mv->dst_y / 16) * width * 2 + (mv->dst_x / 16) * 2] =
              mv->dst_x - mv->src_x;
          (*out)[(video_frame_count - 1) * width * height * 2 +
                 (mv->dst_y / 16) * width * 2 + (mv->dst_x / 16) * 2 + 1] =
              mv->dst_y - mv->src_y;

          // Set the relative frames
          (*out_source)[(video_frame_count - 1) * width * height +
                        (mv->dst_y / 16) * width + (mv->dst_x / 16)] =
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

  // av_register_all(); //https://github.com/leandromoreira/ffmpeg-libav-tutorial/issues/29 cette fonction est deprecated et on a plus besoin de l'appeler
  // avcodec_register_all(); idem

  if (avformat_open_input(&fmt_ctx, filename, NULL, NULL) < 0) {
    fprintf(stderr, "Could not open source file %s\n", filename);
    exit(1);
  }

  if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
    fprintf(stderr, "Could not find stream information\n");
    exit(1);
  }

  open_codec_context(fmt_ctx, AVMEDIA_TYPE_VIDEO);

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

// void av_register_all_w() { av_register_all(); }

// void avcodec_register_all_w() { avcodec_register_all(); }

AVFormatContext **init_AVFormatContext() {
  AVFormatContext **ret = NULL;
  ret = malloc(sizeof(AVFormatContext *));
  *ret = NULL;
  return ret;
}

AVCodec **init_AVCodec() {
  AVCodec **ret = NULL;
  ret = malloc(sizeof(AVCodec *));
  *ret = NULL;
  return ret;
}

AVStream **init_AVStream() {
  AVStream **ret = NULL;
  ret = malloc(sizeof(AVStream *));
  *ret = NULL;
  return ret;
}

AVCodecContext **init_AVCodecContext() {
  AVCodecContext **ret = NULL;
  ret = malloc(sizeof(AVCodecContext *));
  *ret = NULL;
  return ret;
}

AVFrame **init_AVFrame() {
  AVFrame **ret = NULL;
  ret = malloc(sizeof(AVFrame *));
  *ret = NULL;
  return ret;
}

AVDictionary **init_AVDictionary() {
  AVDictionary **ret = NULL;
  ret = malloc(sizeof(AVDictionary *));
  *ret = NULL;
  return ret;
}

AVPacket *init_AVPacket() {
  AVPacket *ret = NULL;
  ret = (AVPacket *)malloc(sizeof(AVPacket));
  memset(ret, 0, sizeof(AVPacket));
  return ret;
}

void free_AVFormatContext(AVFormatContext **object) { free(object); }

void free_AVCodec(AVCodec **object) { free(object); }

void free_AVStream(AVStream **object) { free(object); }

void free_AVCodecContext(AVCodecContext **object) { free(object); }

void free_AVFrame(AVFrame **object) { free(object); }

void free_AVDictionary(AVDictionary **object) { free(object); }

void free_AVPacket(AVPacket *object) { free(object); }

int avformat_open_input_w(char *filename, AVFormatContext **fmt_ctx) {
  int ret = avformat_open_input(fmt_ctx, filename, NULL, NULL);
  return ret;
}

int avformat_find_stream_info_w(AVFormatContext **fmt_ctx) {
  int ret = avformat_find_stream_info(*fmt_ctx, NULL);
  return ret;
}

int av_find_best_stream_w(AVFormatContext **fmt_ctx, AVCodec **dec) {
  int ret = av_find_best_stream(*fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, dec, 0);
  return ret;
}

void av_get_stream(AVFormatContext **fmt_ctx, int stream_idx, AVStream **st) {
  *st = (*fmt_ctx)->streams[stream_idx];
}

void avcodec_alloc_context3_w(AVCodec **dec, AVCodecContext **dec_ctx) {
  *dec_ctx = avcodec_alloc_context3(*dec);
}

int avcodec_parameters_to_context_w(AVCodecContext **dec_ctx, AVStream **st) {
  int ret = avcodec_parameters_to_context(*dec_ctx, (*st)->codecpar);
  return ret;
}

void av_dict_set_w(AVDictionary **opts) {
  av_dict_set(opts, "flags2", "+export_mvs", 0);
}

int avcodec_open2_w(AVCodecContext **dec_ctx, AVCodec **dec,
                    AVDictionary **opts) {
  int ret = avcodec_open2(*dec_ctx, *dec, opts);
  return ret;
}

AVFrame * av_frame_alloc_w() {
  return av_frame_alloc();
}

void close_w(AVCodecContext *video_dec_ctx, AVFormatContext *fmt_ctx,
             AVFrame *frame) {}

int av_read_frame_w(AVFormatContext **fmt_ctx, AVPacket *pkt) {
  int ret;
  ret = av_read_frame(*fmt_ctx, pkt);
  return ret;
}

int av_get_packet_stream_idx(AVPacket *pkt) { return pkt->stream_index; }

int avcodec_send_packet_w(AVCodecContext **dec_ctx, AVPacket *pkt) {
  return avcodec_send_packet(*dec_ctx, pkt);
}

int avcodec_receive_frame_w(AVCodecContext **dec_ctx, AVFrame *frame) {
  return avcodec_receive_frame(*dec_ctx, frame);
}

int av_is_error(int ret) {
  return (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF);
}
void av_packet_unref_w(AVPacket *pkt) { av_packet_unref(pkt); }

void read_frame(AVFrame *frame, int width, int height, int **out,
                int *out_dim1, int *out_dim2, int *out_dim3, int **out_source,
                int *out_source_dim1, int *out_source_dim2,
                int *out_source_dim3) {
  int i;
  AVFrameSideData *sd;

  *out_dim1 = height;
  *out_dim2 = width;
  *out_dim3 = 2;

  *out_source_dim1 = height;
  *out_source_dim2 = width;
  *out_source_dim3 = 1;

  // Realloc the full vector to the new video size
  *out = (int *)malloc(width * height * 2 * sizeof(int));
  *out_source = (int *)malloc(width * height * sizeof(int));

  // Set all the new values to 0
  for (size_t k = 0; k < width * height * 2; ++k) {
    (*out)[k] = 0;
  }

  for (size_t k = 0; k < width * height; ++k) {
    (*out_source)[k] = 0;
  }

  sd = av_frame_get_side_data(frame, AV_FRAME_DATA_MOTION_VECTORS);
  if (sd) {
    const AVMotionVector *mvs = (const AVMotionVector *)sd->data;

    for (i = 0; i < sd->size / sizeof(*mvs); i++) {
      const AVMotionVector *mv = &mvs[i];
      // Set the motion vector
      (*out)[(mv->dst_y / 16) * width * 2 + (mv->dst_x / 16) * 2] =
          mv->dst_x - mv->src_x;
      (*out)[(mv->dst_y / 16) * width * 2 + (mv->dst_x / 16) * 2 + 1] =
          mv->dst_y - mv->src_y;

      // Set the relative frames
      (*out_source)[(mv->dst_y / 16) * width + (mv->dst_x / 16)] = mv->source;
    }
  }
  av_frame_unref(frame);
}

int av_get_width(AVCodecContext **dec_ctx) {
  return ((*dec_ctx)->width + 15) / 16;
}
int av_get_height(AVCodecContext **dec_ctx) {
  return ((*dec_ctx)->height + 15) / 16;
}

int main() {
    return 0;
}
