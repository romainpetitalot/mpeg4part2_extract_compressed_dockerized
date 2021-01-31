from mpeg42compressed.numpy.extract_mvs import open_video, extract_frame, close_video 

class VideoMVS(object):

    def __init__(self, file_path):
        """ Class to read a mp4 file and extract the motion vectors frame by frame.

        # Argument:
            - file_path: The file to extract.
        """
        self.fmt_ctx = None
        self.video_dec_ctx = None
        self.video_stream = None
        self.video_frame_idx = None
        self.frame = None
        self.video_frame_count = None
        self.pkt = None


        _av_register_all()
        _av_codec_register_all()

        self.fmt_ctx = _avformat_open_input(file_path)

        _avformat_find_stream_info(self.fmt_ctx)

        stream_idx = _av_find_best_stream(self.fmt_ctx)

        st = _get_stream(self.fmt_ctx, self.stream_idx)

        dec_ctx = _avcodec_alloc_context3



        open_codec_context_w(self.fmt_ctx)

        self.frame = av_frame_alloc_w()

        

    
    def read(self):
        """ Read the next frame from the video stream and returns it.

        # Return
            ret, frame: ret tells if more frames are to be extracted and frames is a list of two numpy arrays, the first contains the mvs and the second the reference frame position.
        """
        ret, a, b = extract_frame(self.video_dec_ctx, self.fmt_ctx, self.pkt)

    def close(self):
        """ Clean up the opened stuff."""
        close_video(self.video_dec_ctx, self.fmt_ctx, self.frame)


