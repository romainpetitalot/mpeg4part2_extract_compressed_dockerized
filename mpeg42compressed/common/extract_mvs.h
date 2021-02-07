#include <libavformat/avformat.h>
#include <libavutil/motion_vector.h>

void extract_mvs(char *filename, int **out, int *out_dim1, int *out_dim2,
                 int *out_dim3, int *out_dim4, int **out_source,
                 int *out_source_dim1, int *out_source_dim2,
                 int *out_source_dim3, int *out_source_dim4);

void av_register_all_w();
void avcodec_register_all_w();

AVFormatContext **init_AVFormatContext();
AVCodec **init_AVCodec();
AVStream **init_AVStream();
AVCodecContext **init_AVCodecContext();
AVFrame **init_AVFrame();
AVDictionary **init_AVDictionary();
AVPacket* init_AVPacket();

void free_AVFormatContext(AVFormatContext **);
void free_AVCodec(AVCodec **);
void free_AVStream(AVStream **);
void free_AVCodecContext(AVCodecContext **);
void free_AVFrame(AVFrame **);
void free_AVDictionary(AVDictionary **);
void free_AVPacket(AVPacket *object);

int avformat_open_input_w(char *filename, AVFormatContext **fmt_ctx);
int avformat_find_stream_info_w(AVFormatContext **fmt_ctx);
int av_find_best_stream_w(AVFormatContext **fmt_ctx, AVCodec **dec);
void av_get_stream(AVFormatContext **fmt_ctx, int stream_idx, AVStream **st);
void avcodec_alloc_context3_w(AVCodec **dec, AVCodecContext **dec_ctx);
int avcodec_parameters_to_context_w(AVCodecContext **dec_ctx, AVStream **st);
void av_dict_set_w(AVDictionary **opts);
int avcodec_open2_w(AVCodecContext **dec_ctx, AVCodec **dec,
                    AVDictionary **opts);
AVFrame * av_frame_alloc_w();

int av_read_frame_w(AVFormatContext **fmt_ctx, AVPacket *pkt);
int av_get_packet_stream_idx(AVPacket *pkt);
int avcodec_send_packet_w(AVCodecContext **dec_ctx, AVPacket *pkt);
int avcodec_receive_frame_w(AVCodecContext **dec_ctx, AVFrame *frame);
int av_is_error(int ret);
void av_packet_unref_w(AVPacket *pkt);
void read_frame(AVFrame *frame, int width, int height, int **out,
                int *out_dim1, int *out_dim2, int *out_dim3, int **out_source,
                int *out_source_dim1, int *out_source_dim2,
                int *out_source_dim3);

int av_get_width(AVCodecContext **dec_ctx);
int av_get_height(AVCodecContext **dec_ctx);
