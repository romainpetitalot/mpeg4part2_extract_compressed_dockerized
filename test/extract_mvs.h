#include <libavformat/avformat.h>
#include <libavutil/motion_vector.h>

void extract_mvs(char *filename, int **out, int *out_dim1, int *out_dim2,
                 int *out_dim3, int *out_dim4, int **out_source,
                 int *out_source_dim1, int *out_source_dim2,
                 int *out_source_dim3, int *out_source_dim4);

void av_register_all_w();
void avcodec_register_all_w();
AVFormatContext* AVFormatContext_w();
int avformat_open_input_w(char *filename, AVFormatContext *fmt_ctx);
void avformat_find_stream_info_w(AVFormatContext *fmt_ctx);