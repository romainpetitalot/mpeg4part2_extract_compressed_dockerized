# from mpeg42compressed.numpy.extract_mvs import av_register_all_w, avcodec_register_all_w

from mpeg42compressed.numpy.extract_mvs import init_AVFormatContext, init_AVCodec, init_AVStream, init_AVCodecContext, init_AVFrame, init_AVDictionary, init_AVPacket

from mpeg42compressed.numpy.extract_mvs import free_AVFormatContext, free_AVCodec, free_AVStream, free_AVCodecContext, free_AVFrame, free_AVDictionary, free_AVPacket

from mpeg42compressed.numpy.extract_mvs import avformat_open_input_w, avformat_find_stream_info_w, av_find_best_stream_w, av_get_stream, avcodec_alloc_context3_w, avcodec_parameters_to_context_w, av_dict_set_w, avcodec_open2_w, av_frame_alloc_w, av_get_width, av_get_height

from mpeg42compressed.numpy.extract_mvs import av_read_frame_w, av_get_packet_stream_idx, avcodec_send_packet_w, avcodec_receive_frame_w, av_is_error, av_packet_unref_w, read_frame

class MVSVideoReader(object):

    def __init__(self, file):
        """ Initialize the video stream to be ready to extract frames.
        
        # Arguments:
            -file: The mp4 file to be read.
        """
        self.is_done = False

        # av_register_all_w()
        # avcodec_register_all_w()

        self.fmt_ctx = init_AVFormatContext()
        self.dec = init_AVCodec()
        self.stream_idx = None
        self.st = init_AVStream()
        self.dec_ctx = init_AVCodecContext()
        self.opts = init_AVDictionary()
        self.pkt = init_AVPacket()

        ret = avformat_open_input_w(file, self.fmt_ctx)

        if ret < 0:
            raise RuntimeError("Could not open source file {}.".format(file))

        ret = avformat_find_stream_info_w(self.fmt_ctx)

        if ret < 0:
            raise RuntimeError("Could not find stream information.")

        ret = av_find_best_stream_w(self.fmt_ctx, self.dec)

        if ret < 0:
            raise RuntimeError("Could not find stream in input file {}".format(file))
        
        self.stream_idx = ret

        av_get_stream(self.fmt_ctx, self.stream_idx, self.st)

        avcodec_alloc_context3_w(self.dec, self.dec_ctx)

        ret = avcodec_parameters_to_context_w(self.dec_ctx, self.st)

        if ret < 0:
            raise RuntimeError("Failed to copy codec parameters to codec context.")

        av_dict_set_w(self.opts)

        ret = avcodec_open2_w(self.dec_ctx, self.dec, self.opts)

        if ret < 0:
            raise RuntimeError("Failed to open codec.")

        self.frame = av_frame_alloc_w()

        self.width = av_get_width(self.dec_ctx)
        self.height = av_get_height(self.dec_ctx)
    
    def close(self):
        """ Deallocate all the av stuff that was allocated."""
        free_AVFormatContext(self.fmt_ctx)
        free_AVCodec(self.dec)
        free_AVStream(self.st)
        free_AVCodecContext(self.dec_ctx)
        free_AVDictionary(self.opts)
        free_AVPacket(self.pkt)

    def read(self):
        """ Reads a frame from the stream. """
        if self.is_done:
            return False, None

        ret = av_read_frame_w(self.fmt_ctx, self.pkt)

        # If < 0, eof or error
        if ret < 0:
            self.is_done = True
            av_packet_unref_w(self.pkt)

            ret = avcodec_send_packet_w(self.dec_ctx, None)

            if ret < 0:
                raise RuntimeError("Error while sending a packet to the decoder: {}.".format(ret))

            while (ret >= 0):
                ret = avcodec_receive_frame_w(self.dec_ctx, self.frame)

                if av_is_error(ret):
                    self.is_done = True
                    return False, None
                elif ret < 0:
                    raise RuntimeError("Error while receiving a frame from the decoder: {}".format(ret))

                if ret >= 0:
                    mv_frame, mv_target = read_frame(self.frame, self.width, self.height)
                    return True, [mv_frame, mv_target]
                
        else:
            if av_get_packet_stream_idx(self.pkt) == self.stream_idx:
                ret = avcodec_send_packet_w(self.dec_ctx, self.pkt)

                if ret < 0:
                    raise RuntimeError("Error while sending a packet to the decoder: {}.".format(ret))

                while (ret >= 0):
                    ret = avcodec_receive_frame_w(self.dec_ctx, self.frame)
                    if av_is_error(ret):
                        self.is_done = True
                        return False, None
                    elif ret < 0:
                        raise RuntimeError("Error while receiving a frame from the decoder: {}".format(ret))

                    if ret >= 0:
                        mv_frame, mv_target = read_frame(self.frame, self.width, self.height)
                        av_packet_unref_w(self.pkt)
                        
                        return True, [mv_frame, mv_target]
