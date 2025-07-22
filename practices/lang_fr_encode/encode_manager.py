# For ffmpeg
# https://ffmpeg.org/ffmpeg.html

# For AV1
# https://gitlab.com/AOMediaCodec/SVT-AV1/-/blob/master/Docs/Parameters.md
# https://gitlab.com/AOMediaCodec/SVT-AV1/-/issues/1753
# https://gitlab.com/AOMediaCodec/SVT-AV1/-/blob/master/Docs/CommonQuestions.md#what-presets-do

# FOR VVENC
# https://github.com/fraunhoferhhi/vvenc

import subprocess, os, re, time
# import whisper

a = time.time()
t_module=time

# Check working directory.
FILE_DIR_PATH = re.sub("[a-zA-Z0-9_]+.py","",__file__)
CURRENT_WORKING_DIR = os.getcwd()
if FILE_DIR_PATH != CURRENT_WORKING_DIR:
    print(f"FILE_DIR_PATH - {FILE_DIR_PATH}")
    print(f"CURRENT_WORKING_DIR - {CURRENT_WORKING_DIR}")
    
    print("Working directory mismatch. Working directory will be changed to the directory of __file__")
    os.chdir(FILE_DIR_PATH)
    print(f"FILE_DIR_PATH - {FILE_DIR_PATH}")
    print(f"changed CURRENT_WORKING_DIR - {os.getcwd()}")
    
    # raise Exception("Woking directory must match with FILE_DIR_PATH")

def run_command(command, log_file, comment):
    """명령어 실행 및 출력 실시간 캡처, 로그 파일에 기록"""
    with open(log_file, 'a', encoding='utf-8') as log:
        log.write("\n")
        log.write("\n //////////////////new command start////////////////////////\n")
        log.write(comment)
        log.write("\n //////////////////new command start////////////////////////\n")
        log.write("\n")
    print(f"start new process with command - {command}\n")
    
    
    
    with open(log_file, 'a', encoding='utf-8') as log:
        try:
            # subprocess.run()을 사용하여 커맨드를 실행하고, 완료되었는지 확인합니다.
            result = subprocess.run(
                command, 
                check=True,  # 명령이 실패할 경우 예외를 발생시킴
                stdout=subprocess.PIPE,  # 표준 출력을 파이프에 저장
                stderr=subprocess.PIPE   # 표준 에러를 파이프에 저장
            )

            # 표준 출력(stdout)을 실시간으로 콘솔에 출력하고 로그 파일에 기록
            log.write(result.stdout.decode())  # 로그 파일에 표준 출력 기록
            log.write(result.stderr.decode())  # 로그 파일에 표준 오류 기록

            # 실시간으로 콘솔에 출력
            print(result.stdout.decode())  # 표준 출력을 화면에 출력
            print(result.stderr.decode())  # 표준 오류를 화면에 출력

            log.write("\n")
            log.write("\n//////////////////new command end//////////////////////// \n")
            log.write(comment)
            log.write("\n//////////////////new command end//////////////////////// \n")
            log.write("\n")
        
            return result
        except subprocess.CalledProcessError as e:
            # subprocess가 에러를 발생시키면 이를 잡아서 처리합니다.
            print(f"Error executing command: {e}")
            print(f"stderr: {e.stderr.decode()}")
            return None
        
        # process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        
        # # 실시간 출력 및 로그 파일 기록
        # for stdout_line in process.stdout:
        #     print(stdout_line.strip())  # 콘솔에 출력
        #     log.write(stdout_line)      # 로그 파일에 기록

        # for stderr_line in process.stderr:
        #     print(stderr_line.strip(), file=sys.stderr)  # 콘솔에 출력
        #     log.write(stderr_line)  # 로그 파일에 기록
        # process.wait()


process_others={}
metadata_dict={}
class MethodType:
    svtapp = "svtapp"
    libsvtav1 = "libsvtav1"
    vvenc = "vvenc"
    whisper = "whisper"
    merge = "merge"

class WhisperModelType:
    small = "small"
    medium = "medium"
    turbo = "turbo"
    large_v3_turbo = "large-v3-turbo"

class EncodeManager:
    def __init__(self,initial_v_dict={}):
        # 사용자 설정
        self.v_dict={
            # whisper medium may work not correctly on my labtop and turbo stuck many times weird words such as sous titrage? CT' 501 ... 
            # even turbo and large-v3-turbo also stuck many times but small model less likely produce weird words this may be cause by the lack of my labtop resources.
            "whisper_model":WhisperModelType.small, 
            # basic variable setting almost non dependent.
            "season" : "01",
            "epnum" : "2",
            "extension" : "mp4",
            "library_type" : MethodType.svtapp,
            "passnum" : "2", # 3-pass seems has bug. the encoded video not enough data bytes.
            "preset" : "5",
            "input_video_encode_format_depth" : "10",
            "input_video_file" : "", # f"Episode {self.v_dict['epnum']}.mp4", # INPUT_VIDEO_FILE = f"{FILE_DIR_PATH}Episode {EPNUM}.mp4" # if the log file contain of full file path would look bad for debugging.
            "target_width" : "640",
            "target_height" : "360",
            "video_bitrate" : "125",  # les simson 100kbps, one piece 125kbps, solo leveling 350kbps
            "fps" : "0", # For ambiguous rate 23.98.... Will be extracted from the input video. 
            "frameratenumerator" : "0",
            "frameratedenominator" : "0",
            "vf_filter_denoise" : True,
            "audio_bits" : "64",
            "audio_filters" : "highpass=f=100,lowpass=f=6000,acompressor,aresample=16000",
            "audio_filters_apply" : False,

            #SVTAV1 settings
            # tune=0:film-grain=2:rc=1:min-qp=%MINQP%:max-qp=%MAXQP%:aq-mode=2
            "svtav1_tune" : "0",
            "svtav1_film_grain" : "2",
            "svtav1_rc" : "1",  # 1 for VBR, 2 for CBR
            "svtav1_min_qp" : "26",
            "svtav1_max_qp" : "63",
            "svtav1_aq_mode" : "2",
            "svtav1_lookahead" : "60",
            "svtav1_scene_change_detection" : False,
            # to enable VBR mode and set the flexibility of the effective bitrate using --bias-pct. 
            "svtav1_use_bias_pct" : False,
            #In the VBR mode, the rate control algorithm matches the rate over the sequence. Use --gop-constraint-rc 1 to enable rate matching over each gop. This feature is currently supported with VBR mode when Gop size is greater than 119.
            # --gop-constraint-rc 1
            "svtav1_use_gop_constraint" : False, # set default. not to --gop-constraint-rc 0
            "svtav1_maxsection_pct" : "7000", # GOP max bitrate (expressed as a percentage of the target rate). It seems 10000 => 100%. 2000(default)
            "svtav1_undershoot_pct" : "10", # Allowable datarate undershoot (min) target (%), default depends on the rate control mode (25 for CBR, 50 for VBR)
            "svtav1_overshoot_pct" : "100", # Allowable datarate overshoot (max) target (%), default depends on the rate control mode
            "svtav1_enable_qm" : True,
        
            #VVENC settings
            "vvenc_threads":"12",
            # medium also super slow. svtapp 2-pass of task with preset 4 for 1.2h == vvenc medium with 8threads over 12h. 
            # may need to look into the options carefully.
            "vvenc_preset":"fast" 
        }
        self.update_values(initial_v_dict)

    def final_update_without_other_update(self,v_dict={},is_input_changed=False):
        for k,v in v_dict.items():
            self.v_dict[k] = v
        if is_input_changed:  # change input video name must change dependent name.
            self.update_input_video_name()

    def update_input_video_name(self):
        self._set_original_video_info()

    def update_values(self,v_dict={}):
        for k,v in v_dict.items():
            self.v_dict[k] = v

        self._set_dependent_values()
        self._set_original_video_info()
        self._set_targets_name()
        self._set_base_commands()

    def _set_dependent_values(self):
        # self.v_dict["input_video_file"] = f"Episode {self.v_dict['epnum']}.mkv"
        self.v_dict["input_video_file"] = f"Episode {self.v_dict['epnum']}.{self.v_dict['extension']}" if self.v_dict["input_video_file"] == "" else self.v_dict["input_video_file"]
        self.v_dict["padded_epnum"] = f"{self.v_dict['epnum']:>02}"
        self.v_dict["suffix_s_e"] =  f"s{self.v_dict['season']}_e{self.v_dict['padded_epnum']}"
        self.v_dict["inter_video_format"] = "yuv420p" if self.v_dict["input_video_encode_format_depth"] == "8" else "yuv420p10le"
        self.v_dict["vf_filters"] = f"hqdn3d=4:3:6:6,scale={self.v_dict['target_width']}:{self.v_dict['target_height']}" if self.v_dict["vf_filter_denoise"] else f"scale={self.v_dict['target_width']}:{self.v_dict['target_height']}"
        self.v_dict["max_video_bitrate"] = str(int(int(self.v_dict["video_bitrate"])*1.7))+"k"

    def _set_targets_name(self):
        # 중간 파일
        self.v_dict["inter_yuv_video_file"] = f"{self.v_dict['suffix_s_e']}_intermediate.yuv"
        self.v_dict["inter_stats_file"] = f"{self.v_dict['suffix_s_e']}_{self.v_dict['library_type']}_stats.stat"
        self.v_dict["inter_stats_vvenc_file"] = f"{self.v_dict['suffix_s_e']}_{self.v_dict['library_type']}_stats.json"
        # 출력 파일
        self.v_dict["output_vvenc_266_file"] =  f"{self.v_dict['suffix_s_e']}_{self.v_dict['library_type']}_encoded.266"
        self.v_dict["output_ivf_video_file"] =  f"{self.v_dict['suffix_s_e']}_{self.v_dict['library_type']}_encoded.ivf"
        self.v_dict["output_video_file"] = \
            f"{self.v_dict['suffix_s_e']}_{self.v_dict['library_type']}_{self.v_dict['passnum']}-p_p{self.v_dict['preset']}_{"8bit" if self.v_dict['input_video_encode_format_depth'] == "8" else "10bit"}_s{self.v_dict['target_width']}_{self.v_dict['target_height']}_b{self.v_dict['video_bitrate']}k_" \
            + f"vf{'4366_' if self.v_dict['vf_filter_denoise'] else ''}" \
            + f"a{self.v_dict['audio_bits']}k_" 
        if self.v_dict['library_type'] in [MethodType.svtapp, MethodType.libsvtav1]:
            if self.v_dict['svtav1_scene_change_detection']:
                self.v_dict["output_video_file"] += "scd1_"
            self.v_dict["output_video_file"] += f"t{self.v_dict['svtav1_tune']}qp{self.v_dict['svtav1_min_qp']}-{self.v_dict['svtav1_max_qp']}_aq{self.v_dict['svtav1_aq_mode']}_"
            self.v_dict["output_video_file"] += f"o{self.v_dict['svtav1_overshoot_pct']}u{self.v_dict['svtav1_undershoot_pct']}_"
            self.v_dict["output_video_file"] += f"lad{self.v_dict['svtav1_lookahead']}_"
        self.v_dict["output_video_file"] += ".mkv"
        
        self.v_dict["output_orig_audio_file"] = f"{self.v_dict['suffix_s_e']}_orig_audio.m4a"
        self.v_dict["output_encoded_audio_file"] = f"{self.v_dict['suffix_s_e']}inter_audio.opus"
        self.v_dict["debug_log_file"] = f"{self.v_dict['suffix_s_e']}_LOG.txt"
        self.v_dict["debug_log_whisper_file"] = f"{self.v_dict['suffix_s_e']}_LOG_WHISPER.txt"
        self.v_dict["output_srt_file"] = f"{self.v_dict['suffix_s_e']}_orig_audio_srt.srt"
        self.v_dict["output_video_srt_merged_file"] = f"self.v_dict['output_video_file'].replace('.mkv','_with_srt.mkv')"


    
    def _set_original_video_info(self):
        """ffprobe를 사용하여 비디오 해상도와 프레임 속도를 추출"""
        # 해상도 추출
        resolution = subprocess.check_output([
            'ffprobe', '-v', 'error', '-select_streams', 'v:0',
            '-show_entries', 'stream=width,height', '-of', 'default=noprint_wrappers=1:nokey=1', self.v_dict["input_video_file"]
        ]).decode().strip().split('\n')
        width, height = resolution[0], resolution[1]

        # 프레임 속도 추출
        fps = subprocess.check_output([
            'ffprobe', '-v', 'error', '-select_streams', 'v:0',
            '-show_entries', 'stream=r_frame_rate', '-of', 'default=noprint_wrappers=1:nokey=1', self.v_dict["input_video_file"]
        ]).decode().strip()

        # def modify_global_variables(v_dict={}):
        # input_video_file = v_dict["input_video_file"] if "input_video_file" in v_dict else INPUT_VIDEO_FILE

        # orig_width, orig_height, orig_fps = get_video_info(INPUT_VIDEO_FILE)
        print(f"original video info from : {self.v_dict['input_video_file']}")
        print(f"original width :{width}")
        print(f"original height :{height}")
        print(f"original fps :{fps}")

        if "/" in fps:
            self.v_dict["frameratenumerator"], self.v_dict["frameratedenominator"] = fps.split("/")
        else:
            self.v_dict['fps'] = fps 

        # orig_fps value is 24000/1001 then FPS=23.9870..... this may cause unsync with audio.
        # should try again for FPS=24000/1001 
        # There's bug. The app cannot accept correctly. It truncate value from 23.976... to 23. That's the reason the video length changed. To fix it. encoding with the value 24. 
        # You need to modify the result of encoding to fix the length when you merged it with ffmpeg.
        print(f"encoding WIDTH : {self.v_dict["target_width"]}")
        print(f"encoding HEIGHT : {self.v_dict["target_height"]}")
        print(f"encoding FPS : {self.v_dict["fps"]}")
        print(f"encoding frameratenumerator : {self.v_dict["frameratenumerator"]}")
        print(f"encoding frameratedenominator : {self.v_dict["frameratedenominator"]}")


    def _set_base_commands(self):
        self.v_dict["command_encoding_main"] = "" # Will be set when the main command once be made.
        
        self.v_dict["command_audio_orig_extract"] = ["ffmpeg", "-y","-i", self.v_dict["input_video_file"], "-vn", "-acodec", "copy", self.v_dict["output_orig_audio_file"]]
        self.v_dict["comment_log_audio_orig_extract"] = f"Save extracted original audio.  - {self.v_dict["command_audio_orig_extract"]} \n"

        self.v_dict["command_make_encoded_audio"] = [
                "ffmpeg", "-y", "-i", self.v_dict["input_video_file"], "-vn",
                # "-af", A_FILTERS,  # It's quite hard to listen even original audio. Uncertain this is cuase by audio filter or just opus. Disable filter.
                "-c:a", "libopus", "-b:a", f"{self.v_dict["audio_bits"]}k", 
                # "-application", "voip", 
                self.v_dict["output_encoded_audio_file"]
            ]
        self.v_dict["comment_log_make_inter_audio"] = f"Making intermediate audio - {self.v_dict["command_make_encoded_audio"]} \n"

        self.v_dict["command_make_inter_yuv_video"] = [
                "ffmpeg", "-y", "-i", self.v_dict["input_video_file"],
                "-vf", f"{self.v_dict["vf_filters"]}",
                # "-threads", "14",
                "-pix_fmt", f"{self.v_dict["inter_video_format"]}", 
                # "-vsync", "passthrough",  # Unlike using ffmpeg-libsv1... the result of video length increased. Uncertain this is the problem or not.
                # "-vsync", "0",   # Use frame drop test.
                # "-copyts", 
                "-f", "rawvideo",
                # "-r", FPS,
                # "-loglevel", "debug",
                self.v_dict["inter_yuv_video_file"]
            ]
        self.v_dict["comment_log_make_inter_yuv_video"] = f"Making intermediate video - {self.v_dict["command_make_inter_yuv_video"]} \n"
         
        self.v_dict["command_make_mkv_with_results"] = ["ffmpeg", "-y", 
                        # "--framerate", orig_fps,   # fps == 24000/1001. This will match the audio and encoded video.                  
                        # "-r", orig_fps,   # fps == 24000/1001. This will match the audio and encoded video.                  
                        # if FRAMERATENUMERATOR/FRAMERATEDENUMERATOR's set then this -r shouldn't be required.
                        "-i", self.v_dict["output_ivf_video_file"] if self.v_dict["library_type"] in [MethodType.svtapp, MethodType.libsvtav1] else self.v_dict["output_vvenc_266_file"],
                        "-i", self.v_dict["output_encoded_audio_file"],
                        "-c:v", "copy",
                        "-c:a", "copy",
                        self.v_dict["output_video_file"]
                        ]
        self.v_dict["comment_log_make_mkv_with_results"] = f"Make final MKV video with next command - {self.v_dict["command_make_mkv_with_results"]}"
         


    def extract_orig_audio(self,v_dict={}):
        # It doesn't require encoding so this method able to set just target files. 
        # Or use update methods to use dependent names. 
        # It would be better set season and episode number to make a suffix by update with input v_dict and then use final update to fix targets name
        # ex) v_dict = {"season" : "02", "episode" : "10", "input_video_file" : "xxxxex.mkv"}

        # Update suffix if v_dict has another season, episode number.
        self.update_values(v_dict)  # change another suffix of target names.

        # This will set target names without updatin other dependencies.
        # ex)v_dict = {"output_orig_audio_file" : "xyz.wav"}
        self.final_update_without_other_update(v_dict)




        
    def make_variable_set_for_svtav1(self):
        VARIABLES_FOR_SVTAV1 = [
                            "SEASON", self.v_dict["season"], "EPNUM", self.v_dict["epnum"], 
                            "LIBRARY_TYPE", self.v_dict["library_type"],"PASSNUM", self.v_dict["passnum"],
                            "PRESET", self.v_dict["preset"],
                            "INPUT_VIDEO_ENCODE_FORMAT_DEPTH", self.v_dict["input_video_encode_format_depth"],
                            "INPUT_VIDEO_FILE", self.v_dict["input_video_file"],
                            "WIDTH", self.v_dict["target_width"], "HEIGHT", self.v_dict["target_height"],
                            "BITRATE", self.v_dict["video_bitrate"], "FPS", self.v_dict["fps"],
                            "FRAMERATENUMERATOR", self.v_dict["frameratenumerator"],
                            "FRAMERATEDENOMINATOR", self.v_dict["frameratedenominator"],
                            "VF_FILTER_DENOISE", self.v_dict["vf_filter_denoise"],
                            "A_BITS", self.v_dict["audio_bits"], "A_FILTERS", self.v_dict["audio_filters"], 
                            "A_FILTERS_APPLY", self.v_dict["audio_filters_apply"],
                            "INTER_VIDEO_FORMAT", self.v_dict["inter_video_format"],
                            "VF_FILTERS", self.v_dict["vf_filters"],

                            #SVTAV1 settings
                            # tune=0:film-grain=2:rc=1:min-qp=%MINQP%:max-qp=%MAXQP%:aq-mode=2  
                            "TUNE", self.v_dict["svtav1_tune"], "FILM_GRAIN", self.v_dict["svtav1_film_grain"],
                            "RC", self.v_dict["svtav1_rc"], "MIN_QP", self.v_dict["svtav1_min_qp"], "MAX_QP", self.v_dict["svtav1_max_qp"],
                            "AQ_MODE", self.v_dict["svtav1_aq_mode"], "LOOKAHEAD", self.v_dict["svtav1_lookahead"],
                            "SCENE_CHANGE_DETECTION", self.v_dict["svtav1_scene_change_detection"],
                            "USE_BIAS_PCT", self.v_dict["svtav1_use_bias_pct"], 
                            "USE_GOP_CONSTRAINT", self.v_dict["svtav1_use_gop_constraint"],
                            "MAXSECTION_PCT", self.v_dict["svtav1_maxsection_pct"],
                            "UNDERSHOOT_PCT", self.v_dict["svtav1_undershoot_pct"],
                            "OVERSHOOT_PCT", self.v_dict["svtav1_overshoot_pct"],
                            "ENABLE_QM", self.v_dict["svtav1_enable_qm"], 
                            "INTER_STAT_FILE", self.v_dict["inter_stats_file"]
                            ]
        return VARIABLES_FOR_SVTAV1

    def run_svtapp(self, is_merged=True):
        COMMAND_SVTAV1_BASE = [
        'SvtAv1EncApp', '-i', self.v_dict["inter_yuv_video_file"], 
        '-w', self.v_dict["target_width"], '-h', self.v_dict["target_height"], 
        "--input-depth", self.v_dict["input_video_encode_format_depth"],
        # '--fps', FPS,   # There's bug. The app cannot accept correctly. It truncate value from 23.976... to 23. That's the reason the video length changed. To fix it. encoding with the value 24. 
        # --fps should be 24 not 23.976... You need to modify the result of encoding to fix the length when you merged it with ffmpeg.
        '--rc', self.v_dict["svtav1_rc"], #--rc 1 for VBA not CBR
        '--tbr', f"{self.v_dict['video_bitrate']}k", 
        "--scd", "1" if self.v_dict["svtav1_scene_change_detection"] else "0", 
        "--lookahead", self.v_dict["svtav1_lookahead"],
        "--tune", self.v_dict["svtav1_tune"],
        "--min-qp", self.v_dict["svtav1_min_qp"],
        "--max-qp", self.v_dict["svtav1_max_qp"],
        "--aq-mode", self.v_dict["svtav1_aq_mode"],
        "--film-grain", self.v_dict["svtav1_film_grain"],
        "--maxsection-pct", self.v_dict["svtav1_maxsection_pct"],
        "--undershoot-pct", self.v_dict["svtav1_undershoot_pct"],
        "--overshoot-pct", self.v_dict["svtav1_overshoot_pct"],
        "--enable-qm", "1" if self.v_dict["svtav1_enable_qm"] else "0",
        "--progress", "0" # The log file would be dirty when print the progress. just assume from the file sizes.
        ]  \
        + ["--fps-num", self.v_dict["frameratenumerator"],   # if FPS is integer, else set FrameRateNumerator and FrameRateDenominator
        "--fps-denom", self.v_dict["frameratedenominator"]] if self.v_dict["fps"] == "0" else ["-fps", self.v_dict["fps"]] \
        +["--bias-pct" if self.v_dict["svtav1_use_bias_pct"] else ""]
        
        # 3-pass not working correctly need more test.
        # COMMAND_SVTAV1_ENCODING_FOR_PASS3 = [COMMAND_SVTAV1_BASE \
        #             +["--pass", "1", "--preset", str(PRESET_PRE_FIRST), "--stats", STATS_FILE] \
        #             ,COMMAND_SVTAV1_BASE \
        #             +["--pass", "2", "--preset", str(PRESET_FIRST), "--stats", STATS_FILE] \
        #             ,COMMAND_SVTAV1_BASE \
        #             +["--pass", "3", "--preset", str(PRESET_LAST), "--stats", STATS_FILE] \
        #             + ['-b', IVF_VIDEO_FILE]]
        # COMMENT_LOG_SVTAV1_ENCODING_FOR_PASS3 = [
        #     f"Encoding-3pass-1 with next command - {COMMAND_SVTAV1_ENCODING_FOR_PASS3[0]}",
        #     f"Encoding-3pass-2 with next command - {COMMAND_SVTAV1_ENCODING_FOR_PASS3[1]}",
        #     f"Encoding-3pass-3 with next command - {COMMAND_SVTAV1_ENCODING_FOR_PASS3[2]}",
        #                                         ]

        COMMAND_SVTAV1_ENCODING_FOR_PASS2 = [
                    COMMAND_SVTAV1_BASE \
                    +["--pass", "1", "--preset", self.v_dict["preset"], "--stats", self.v_dict["inter_stats_file"]], \
                    COMMAND_SVTAV1_BASE \
                    +["--pass", "2", "--preset", self.v_dict["preset"], "--stats", self.v_dict["inter_stats_file"]] \
                    +['-b', self.v_dict["output_ivf_video_file"]]
                    ]
        COMMENT_LOG_SVTAV1_ENCODING_FOR_PASS2 = [
            f"SVTAV1APP: encoding-2pass-1 with next command - {COMMAND_SVTAV1_ENCODING_FOR_PASS2[0]}",
            f"SVTAV1APP: encoding-2pass-2 with next command - {COMMAND_SVTAV1_ENCODING_FOR_PASS2[1]}"
            ]

        COMMAND_SVTAV1_ENCODING_FOR_PASS1 = COMMAND_SVTAV1_BASE \
                    +["--preset", self.v_dict["preset"], "-b", self.v_dict["output_ivf_video_file"]]
        COMMENT_LOG_SVTAV1_ENCODING_FOR_PASS1 = f"SVTAV1APP: encoding-1pass with next command - {COMMAND_SVTAV1_ENCODING_FOR_PASS1}"

        


        time = t_module
        start_time = time.time()
        temp_time = start_time

        command_sets = [
            [self.v_dict["command_audio_orig_extract"],self.v_dict["comment_log_audio_orig_extract"],"COMMAND_AUDIO_ORIG_EXTRACT end. : "],
            [self.v_dict["command_make_encoded_audio"],self.v_dict["comment_log_make_inter_audio"],"COMMAND_MAKE_INTER_AUDIO end. : "],
            [self.v_dict["command_make_inter_yuv_video"],self.v_dict["comment_log_make_inter_yuv_video"],"COMMAND_MAKE_INTER_VIDEO end. : "],
        ]

        if self.v_dict['passnum'] == "3" :
            # for i in range(3):
            #     command_sets += [[COMMAND_SVTAV1_ENCODING_FOR_PASS3[i],COMMENT_LOG_SVTAV1_ENCODING_FOR_PASS3[i],f"COMMAND_SVTAV1_ENCODING_FOR_PASS3_{i} end. : "]]
            pass
        elif self.v_dict['passnum'] == "2" :
            for i in range(2):
                command_sets += [[COMMAND_SVTAV1_ENCODING_FOR_PASS2[i],COMMENT_LOG_SVTAV1_ENCODING_FOR_PASS2[i],f"COMMAND_SVTAV1_ENCODING_FOR_PASS2_{i} end. : "]]
        else:
            command_sets += [[COMMAND_SVTAV1_ENCODING_FOR_PASS1,COMMENT_LOG_SVTAV1_ENCODING_FOR_PASS1,f"COMMAND_SVTAV1_ENCODING_FOR_PASS1 end. : "]]

        main_command = command_sets[-1][0]
        self.v_dict["command_encoding_main"] = " ".join(main_command)

        if is_merged:
            command_sets += [[self.v_dict["command_make_mkv_with_results"],self.v_dict["comment_log_make_mkv_with_results"],f"COMMAND_MAKE_MKV_WITH_RESULTS end. : "]]
        
        for i,(command, comment_log, command_time_prefix) in enumerate(command_sets):
            run_command(command,self.v_dict["debug_log_file"],comment_log)
            command_finish_time = time.time()
            command_time_prefix += f"{command_finish_time-temp_time:.2f}"
            command_sets[i][-1] = command_time_prefix
            print(command_time_prefix)
            temp_time = time.time()
            
        
        VARIABLES_FOR_SVTAV1 = self.make_variable_set_for_svtav1()

        
        with open(self.v_dict["debug_log_file"],'a') as log:
            for command, _, time in command_sets:
                cstr_ = " ".join(command)
                log.write("\n")
                log.write(cstr_)
                print(cstr_)
                log.write("\n")
                log.write(time)
                print(time)
                log.write("\n")
                log.write("\n")
            log.write(f"All of settings : ")
            log.write("\n")
            log.write(f"{VARIABLES_FOR_SVTAV1}")
        

        # (선택) 중간 파일 제거
        # for f in [YUV_FILE, AUDIO_FILE, STATS_FILE, VIDEO_FILE]:

        if os.path.exists(self.v_dict["inter_yuv_video_file"]):
            os.remove(self.v_dict["inter_yuv_video_file"])


    def run_libsvtav1(self):
        # ffmpeg's encoding using libsvtav1 library seems bitrate control not work properly or it depends on the version.
        # when I setting bitrate as -b 150k the final video has the bitrate including audio bitrate.
        # unlike this integrated library, when using seperately the apps then the target bitrate only consider video.
        # ffmpeg -loglevel info -i "%INPUT%" -vf "%FILTERS%" -c:v libsvtav1 -b:v %BITRATE% -bufsize %BUFSIZE%  -preset %PRESET1% -g %GOP% -svtav1-params "tune=0:film-grain=2:rc=1:min-qp=%MINQP%:max-qp=%MAXQP%:aq-mode=2" -c:a libopus -b:a %AUDIOBR% "%OUTPUT%" 
        # 2-pass not working correctly, on my ffmpeg version 2024 dec. Uncertain the version of march of 2025 would be fixed.
        if self.v_dict["library_type"] != MethodType.libsvtav1:
            print(f"corrent library_type ({self.v_dict["library_type"]}) must be changed to {MethodType.libsvtav1}")
            # self.v_dict["library_type"] = MethodType.libsvtav1
            return
        LIBSVTAV1_VARIABLES = {
            "gop" : "240",
        }
        # "tune=0:film-grain=2:rc=1:min-qp=%MINQP%:max-qp=%MAXQP%:aq-mode=2" -c:a libopus -b:a %AUDIOBR% "%OUTPUT%" 
        COMMAND_LIBSVTAV1_MAKE_ENCODED_VIDEO_FILE_BASE = [
            "ffmpeg", "-y", "-i", self.v_dict["input_video_file"], 
            # "-c:a", "libopus", "-b:a", f"{self.v_dict["audio_bits"]}k", 
            "-c:a", "copy", "-c:s", "copy", 
            "-vf", f"{self.v_dict["vf_filters"]}",
            "-pix_fmt", f"{self.v_dict["inter_video_format"]}", 
            "-c:v", self.v_dict["library_type"], 
            # "-b:v", BITRATE,  # use -tbr otherwise the bitrate of the video contain audio bitrate.
            # "-preset", self.v_dict["preset"], # preset in ffmpeg may differ with svtav1 so put the preset into svtav1-params 
            # "-g", LIBSVTAV1_VARIABLES["gop"],
            "-svtav1-params", 
            # :w={self.v_dict["target_width"]} \
            # :h={self.v_dict["target_height"]}   \
            f"preset={self.v_dict["preset"]}"\
            +f":input-depth={self.v_dict["input_video_encode_format_depth"]}"\
            +f":rc={self.v_dict["svtav1_rc"]}"\
            +f":tbr={self.v_dict["video_bitrate"]}"\
            +f'{":scd=1" if self.v_dict["svtav1_scene_change_detection"] else ""}'\
            +f":lookahead={self.v_dict["svtav1_lookahead"]}"\
            +f":tune={self.v_dict["svtav1_tune"]}"\
            +f":min-qp={self.v_dict["svtav1_min_qp"]}"\
            +f":max-qp={self.v_dict["svtav1_max_qp"]}"\
            +f":aq-mode={self.v_dict["svtav1_aq_mode"]}"\
            +f":film-grain={self.v_dict["svtav1_film_grain"]}"\
            +f":maxsection-pct={self.v_dict["svtav1_maxsection_pct"]}"\
            +f":undershoot-pct={self.v_dict["svtav1_undershoot_pct"]}" \
            +f":overshoot-pct={self.v_dict["svtav1_overshoot_pct"]}"\
            +f":enable-qm={"1" if self.v_dict["svtav1_enable_qm"] else "0"}"\
            +f":progress=0"\
            +f"{':bias-pct' if self.v_dict["svtav1_use_bias_pct"] else ''}"
            ]
        COMMAND_LIBSVTAV1_MAKE_ENCODED_VIDEO_FILE_FOR_PASS2 = [
            COMMAND_LIBSVTAV1_MAKE_ENCODED_VIDEO_FILE_BASE[:-1]\
            + [COMMAND_LIBSVTAV1_MAKE_ENCODED_VIDEO_FILE_BASE[-1] + f":pass=1:stats={self.v_dict["inter_stats_file"]}"]
            + [f"{self.v_dict["output_video_file"]}"],
            COMMAND_LIBSVTAV1_MAKE_ENCODED_VIDEO_FILE_BASE[:-1]\
            + [COMMAND_LIBSVTAV1_MAKE_ENCODED_VIDEO_FILE_BASE[-1] + f":pass=2:stats={self.v_dict["inter_stats_file"]}"]
            + [f" {self.v_dict["output_video_file"]}"],
        ]
        COMMENT_LOG_LIBSVTAV1_MAKE_ENCODED_VIDEO_FILE_FOR_PASS2 =[
            f"libsvtav1: encoding-2pass-1 with next command - {COMMAND_LIBSVTAV1_MAKE_ENCODED_VIDEO_FILE_FOR_PASS2[0]}",
            f"libsvtav1: encoding-2pass-2 with next command - {COMMAND_LIBSVTAV1_MAKE_ENCODED_VIDEO_FILE_FOR_PASS2[1]}"
            ]
        
        COMMAND_LIBSVTAV1_MAKE_ENCODED_VIDEO_FILE_FOR_PASS1 = COMMAND_LIBSVTAV1_MAKE_ENCODED_VIDEO_FILE_BASE + [f"{self.v_dict["output_video_file"]}"]
        COMMENT_LOG_LIBSVTAV1_MAKE_ENCODED_VIDEO_FILE_FOR_PASS1 = f"libsvtav1: encoding-1pass with next command - {COMMAND_LIBSVTAV1_MAKE_ENCODED_VIDEO_FILE_FOR_PASS1}"

        time = t_module
        start_time = time.time()
        temp_time = start_time
        command_sets = []

        if self.v_dict["passnum"] == "2":
            for i in range(2):
                command_sets += [[
                    COMMAND_LIBSVTAV1_MAKE_ENCODED_VIDEO_FILE_FOR_PASS2[i],
                    COMMENT_LOG_LIBSVTAV1_MAKE_ENCODED_VIDEO_FILE_FOR_PASS2[i],
                    f"COMMAND_LIBSVTAV1_MAKE_ENCODED_VIDEO_FILE_FOR_PASS2_{i} end. : "
                    ]]
        else:
            command_sets += [[
                COMMAND_LIBSVTAV1_MAKE_ENCODED_VIDEO_FILE_FOR_PASS1,
                COMMENT_LOG_LIBSVTAV1_MAKE_ENCODED_VIDEO_FILE_FOR_PASS1,
                f"COMMAND_LIBSVTAV1_MAKE_ENCODED_VIDEO_FILE_FOR_PASS1 end. : "
            ]]
        temp_debug_command = " ".join(command_sets[-1][-1])
        print(temp_debug_command)

        main_command = command_sets[-1][0]
        self.v_dict["command_encoding_main"] = " ".join(main_command)

        i = 0
        for command, comment_log, command_time_prefix in command_sets:
            run_command(command,self.v_dict["debug_log_file"],comment_log)
            command_finish_time = time.time()
            command_time_prefix += f"{command_finish_time-temp_time:.2f}"
            command_sets[i][-1] = command_time_prefix
            print(command_time_prefix)
            temp_time = time.time()
            i+=1

        VARIABLES_FOR_SVTAV1 = self.make_variable_set_for_svtav1()

        
        with open(self.v_dict["debug_log_file"],'a') as log:
            for command, _, time in command_sets:
                cstr_ = " ".join(command)
                log.write("\n")
                log.write(cstr_)
                print(cstr_)
                log.write("\n")
                log.write(time)
                print(time)
                log.write("\n")
                log.write("\n")
            log.write(f"All of settings : ")
            log.write("\n")
            log.write(f"{VARIABLES_FOR_SVTAV1}")
        
    def make_variable_set_for_vvenc(self):
        VARIABLES_FOR_VVENC = [
            "SEASON", self.v_dict["season"], "EPNUM", self.v_dict["epnum"], 
            "LIBRARY_TYPE", self.v_dict["library_type"],"PASSNUM", self.v_dict["passnum"],
            "PRESET", self.v_dict["preset"],
            "INPUT_VIDEO_ENCODE_FORMAT_DEPTH", self.v_dict["input_video_encode_format_depth"],
            "INPUT_VIDEO_FILE", self.v_dict["input_video_file"],
            "WIDTH", self.v_dict["target_width"], "HEIGHT", self.v_dict["target_height"],
            "BITRATE", self.v_dict["video_bitrate"], "FPS", self.v_dict["fps"],
            "FRAMERATENUMERATOR", self.v_dict["frameratenumerator"],
            "FRAMERATEDENOMINATOR", self.v_dict["frameratedenominator"],
            "VF_FILTER_DENOISE", self.v_dict["vf_filter_denoise"],
            "A_BITS", self.v_dict["audio_bits"], "A_FILTERS", self.v_dict["audio_filters"], 
            "A_FILTERS_APPLY", self.v_dict["audio_filters_apply"],
            "INTER_VIDEO_FORMAT", self.v_dict["inter_video_format"],
            "VF_FILTERS", self.v_dict["vf_filters"],

            "THREADS", self.v_dict["vvenc_threads"],
            "INTER_STAT_FILE", self.v_dict["inter_stats_vvenc_file"]
        ]
        return VARIABLES_FOR_VVENC

    def run_vvenc(self):
        
        COMMAND_VVENC_BASE = [
            'vvencFFapp', '-i', self.v_dict["inter_yuv_video_file"], 
            "--preset", self.v_dict["vvenc_preset"],
            '-s', f"{self.v_dict["target_width"]}x{self.v_dict["target_height"]}", 
            "--TargetBitrate", f"{self.v_dict["video_bitrate"]}k", "--MaxBitrate", self.v_dict["max_video_bitrate"],
            "--InputBitDepth", self.v_dict["input_video_encode_format_depth"],
            "--InternalBitDepth", self.v_dict["input_video_encode_format_depth"],
            "--Threads", self.v_dict["vvenc_threads"],
            "--TicksPerSecond", f"{int(int(self.v_dict["frameratenumerator"])*4)}",
            "--FrameRate", self.v_dict["frameratenumerator"], "--FrameScale", self.v_dict["frameratedenominator"],
            "--PerceptQPA", "1",
            "-b", self.v_dict["output_vvenc_266_file"]
        ]

        COMMAND_VVENC_ENCODING_FOR_PASS2 = [
            COMMAND_VVENC_BASE \
            + ["--RCStatsFile", self.v_dict["inter_stats_vvenc_file"], "--Pass", "1"],
            COMMAND_VVENC_BASE \
            + ["--RCStatsFile", self.v_dict["inter_stats_vvenc_file"], "--Pass", "2"],
        ]
        COMMENT_LOG_VVENC_ENCODING_FOR_PASS2 = [
            f"vvencFFapp: encoding-2pass-1 with next command - {COMMAND_VVENC_ENCODING_FOR_PASS2[0]}",
            f"vvencFFapp: encoding-2pass-2 with next command - {COMMAND_VVENC_ENCODING_FOR_PASS2[1]}",
        ]
        COMMAND_VVENC_ENCODING_FOR_PASS1 = COMMAND_VVENC_BASE
        COMMENT_LOG_VVENC_ENCODING_FOR_PASS1 = f"vvencFFapp: encoding-1pass with next command - {COMMAND_VVENC_ENCODING_FOR_PASS1}"

        time = t_module
        start_time = time.time()
        temp_time = start_time

        command_sets = [
            [self.v_dict["command_audio_orig_extract"],self.v_dict["comment_log_audio_orig_extract"],"COMMAND_AUDIO_ORIG_EXTRACT end. : "],
            [self.v_dict["command_make_encoded_audio"],self.v_dict["comment_log_make_inter_audio"],"COMMAND_MAKE_INTER_AUDIO end. : "],
            [self.v_dict["command_make_inter_yuv_video"],self.v_dict["comment_log_make_inter_yuv_video"],"COMMAND_MAKE_INTER_VIDEO end. : "],
        ]
        if self.v_dict['passnum'] == "2" :
            for i in range(2):
                command_sets += [[COMMAND_VVENC_ENCODING_FOR_PASS2[i],COMMENT_LOG_VVENC_ENCODING_FOR_PASS2[i],f"COMMAND_VVENC_ENCODING_FOR_PASS2_{i} end. : "]]
        else:
            command_sets += [[COMMAND_VVENC_ENCODING_FOR_PASS1,COMMENT_LOG_VVENC_ENCODING_FOR_PASS1,f"COMMAND_VVENC_ENCODING_FOR_PASS1 end. : "]]

        main_command = command_sets[-1][0]
        self.v_dict["command_encoding_main"] = " ".join(main_command)

        command_sets += [[self.v_dict["command_make_mkv_with_results"],self.v_dict["comment_log_make_mkv_with_results"],f"COMMAND_MAKE_MKV_WITH_RESULTS end. : "]]

        for i,(command, comment_log, command_time_prefix) in enumerate(command_sets):
            print(f"command will be start : {' '.join(command)}")
            run_command(command,self.v_dict["debug_log_file"],comment_log)
            command_finish_time = time.time()
            command_time_prefix += f"{command_finish_time-temp_time:.2f}"
            command_sets[i][-1] = command_time_prefix
            print(command_time_prefix)
            temp_time = time.time()

        VARIABLES_FOR_VVENC = self.make_variable_set_for_vvenc()

        with open(self.v_dict["debug_log_file"],'a') as log:
            for command, _, time in command_sets:
                cstr_ = " ".join(command)
                log.write("\n")
                log.write(cstr_)
                print(cstr_)
                log.write("\n")
                log.write(time)
                print(time)
                log.write("\n")
                log.write("\n")
            log.write(f"All of settings : ")
            log.write("\n")
            log.write(f"{VARIABLES_FOR_VVENC}")

        # if os.path.exists(self.v_dict["inter_yuv_video_file"]):
            # os.remove(self.v_dict["inter_yuv_video_file"])



    def run_whisper(self):
        # if len(process_others) ==0:
        #     input_audio = OUTPUT_ORIG_AUDIO_FILE
        #     debug_log = DEBUG_LOG_FILE
        #     input_video = OUTPUT_VIDEO_FILE
        #     output_srt = OUTPUT_SRT_FILE
        # else:
        #     input_audio, debug_log, input_video,output_srt = process_others

            
        # model = whisper.load_model("turbo")
        start_time = time.time()
        # m4a 파일 처리
        # result = model.transcribe(input_audio, language="French")
        COMMAND_WHISPER_TRANSCRIBE = ["whisper", self.v_dict["output_encoded_audio_file"], "--language", "French", "--output_format","srt", "--model", self.v_dict["whisper_model"]]
        COMMENT_LOG_WHISPER_TRANSCRIBE = f"Making subtitle with whisper AI ({self.v_dict["whisper_model"]})- {COMMAND_WHISPER_TRANSCRIBE}"
        run_command(COMMAND_WHISPER_TRANSCRIBE,self.v_dict["debug_log_whisper_file"],COMMENT_LOG_WHISPER_TRANSCRIBE)
        
        with open(self.v_dict["debug_log_whisper_file"],"a") as log:
            log.write(f"whisper AI ({self.v_dict["whisper_model"]}) translate done. time : {time.time()-start_time}")
        with open(self.v_dict["debug_log_file"],"a") as log:
            log.write(f"{' '.join(COMMAND_WHISPER_TRANSCRIBE)}")
            log.write(f"whisper AI ({self.v_dict["whisper_model"]}) translate done. time : {time.time()-start_time}")
        
        # integrate_time = time.time()
        # 자막 출력

        # with open(debug_log,"a") as log:
            # log.write(f"translate done. time : {time.time()-integrate_time}")
        # run_command(["ffmpeg","-i",input_video])

    def run_merge_mkv_srt(self,process_others={}, metadata_dict=None):
        # if len(process_others) ==0:
        #     input_video = OUTPUT_VIDEO_FILE
        #     input_audio = OUTPUT_ORIG_AUDIO_FILE
        #     debug_log = DEBUG_LOG_FILE
        #     input_srt = OUTPUT_SRT_FILE
        # else:
        #     process_others={
        #     'output_orig_audio_file' : OUTPUT_ORIG_AUDIO_FILE,
        #     'debug_log_file' : DEBUG_LOG_FILE,
        #     'output_video_file' : OUTPUT_VIDEO_FILE,
        #     'output_srt_file' : OUTPUT_SRT_FILE,
        #     'output_encoded_audio_file' : OUTPUT_ENCODED_AUDIO_FILE 
        # }
            # input_video = process_others["output_video_file"]
            # input_audio = process_others["output_encoded_audio_file"]
            # debug_log = process_others["debug_log_file"]
            # intput_srt = process_others["output_srt_file"]


        # if metadata_dict == None:
        #     meta_main_encoding_command = MAIN_ENCODING_COMMAND
        # else:
            # meta_main_encoding_command = metadata_dict['main_encoding_command']

        meta_main_encoding_command = metadata_dict['main_encoding_command']
        COMMAND_MERGE_OUTPUTS = [
            "ffmpeg", "-i", self.v_dict["output_video_file"], "-i", self.v_dict["output_encoded_audio_file"], "-i", self.v_dict["output_srt_file"],
            "-map", "0", "-map", "1", "-map", "2",
            "-c:v:0", "copy", "-c:a:1", "copy"," -c:s:2", "copy"
            # "-c:v", "copy", "-c:a", "copy", "-c:s", "srt", "-c:t", "copy",
            "-disposition:s:2", "default",
            "-metadata:s:s:0", "language=fre", "-metadata:s:s:0", "title=french sub",
            "-metadata" f"main_command={meta_main_encoding_command}",
            "-metadata" f"main_command={meta_main_encoding_command}",
            self.v_dict["output_video_srt_merged_file"]                     
                                ]
        COMMENT_LOG_MERGE_OUTPUTS = f"merge: encoded video, audio and srt with next command - {COMMAND_MERGE_OUTPUTS}"

    def run_merge_inter_yuv_with_orig_all(self):
        COMMAND_MERGE_OUTPUTS = [
            "ffmpeg", "-i", self.v_dict["input_video_file"], "-i", self.v_dict["output_ivf_video_file"], 
            "-map", "0:a", "-map", "0:s?", "-map", "0:t", "-map", "1:v",
            "-c:a", "copy", "-c:v", "copy", "-c:s", "srt", "-c:t", "copy",
            # "-c:v", "copy", "-c:a", "copy", "-c:s", "srt", "-c:t", "copy",
            self.v_dict["output_video_file"]                     
                                ]
        COMMENT_LOG_MERGE_OUTPUTS = f"merge: encoded video, audio and srt with next command - {COMMAND_MERGE_OUTPUTS}"

        time = t_module
        start_time = time.time()
        temp_time = start_time

        command_sets = [
            [COMMAND_MERGE_OUTPUTS,COMMENT_LOG_MERGE_OUTPUTS,"COMMAND_MERGE_OUTPUTS end. : "],
        ]

        for i,(command, comment_log, command_time_prefix) in enumerate(command_sets):
            run_command(command,self.v_dict["debug_log_file"],comment_log)
            command_finish_time = time.time()
            command_time_prefix += f"{command_finish_time-temp_time:.2f}"
            command_sets[i][-1] = command_time_prefix
            print(command_time_prefix)
            temp_time = time.time()
        
        with open(self.v_dict["debug_log_file"],'a') as log:
            for command, _, time in command_sets:
                cstr_ = " ".join(command)
                log.write("\n")
                log.write(cstr_)
                print(cstr_)
                log.write("\n")
                log.write(time)
                print(time)
                log.write("\n")
                log.write("\n")
            log.write(f"All of settings : ")
            log.write("\n")


def main():
    
    def run_merge():
        em = EncodeManager()
        em.update_values({
            "season":"01",
            "epnum":"1",
            # "library_type":MethodType.vvenc
        })
        em.final_update_without_other_update({
            # "input_video_file":"new_file_name"
            "input_video_file":"new_file_name"   # when input file changed fps also should be changed.
        },is_input_changed=True)
        em.run_merge_mkv_srt()
    
    def run_vvenc():
        em = EncodeManager()
        em.update_values({
            "season":"01",
            "epnum":"40",
            "library_type":MethodType.vvenc,
            "vvenc_threads":"8",
            "vvenc_preset":"medium",
        })
        em.final_update_without_other_update({
            # "input_video_file":"new_file_name"
            # "input_video_file":"new_file_name"
        })
        # em.update_values_related_input_video_name()
        em.run_vvenc()
    
    # run_merge()
    run_vvenc()
    # EncodeManager().run_svtapp()
    # EncodeManager().run_libsvtav1()


if __name__ == "__main__":
    main()