#include <libavformat/avformat.h>
#include <libavutil/motion_vector.h>

void extract_mvs(char *filename, int **out, int *out_dim1, int *out_dim2,
                 int *out_dim3, int *out_dim4, int **out_source,
                 int *out_source_dim1, int *out_source_dim2,
                 int *out_source_dim3, int *out_source_dim4);

void av_register_all_w();
void avcodec_register_all_w();

AVFormatContext* init_AVFormatContext();
AVCodecContext* init_AVCodecContext();
AVStream* init_AVStream();
AVFrame* init_AVFrame();
AVDictionary* init_AVDictionary();

int avformat_open_input_w(char *filename, AVFormatContext *fmt_ctx);
int avformat_find_stream_info_w(AVFormatContext *fmt_ctx);
int av_find_best_stream_w(AVFormatContext *fmt_ctx, AVCodec *dec);
AVCodecContext * avcodec_alloc_context3_w(AVCodec *dec);
int avcodec_parameters_to_context(AVCodecContext *dec_ctx, AVStream* st);
void av_dict_set_w(AVDictionary* opts);
int avcodec_open2_w(AVCodecContext *dec_ctx, AVCodec *dec, AVDictionary *opts);
AVFrame* av_frame_alloc_w();
void close(AVCodecContext* video_dec_ctx, AVFormatContext *fmt_ctx, AVFrame* frame);

AVStream* get_stream(AVFormatContext *fmt_ctx, int stream_idx);