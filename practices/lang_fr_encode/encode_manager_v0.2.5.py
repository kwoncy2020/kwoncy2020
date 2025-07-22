# For ffmpeg
# https://ffmpeg.org/ffmpeg.html

# For AV1
# https://gitlab.com/AOMediaCodec/SVT-AV1/-/blob/master/Docs/Parameters.md
# https://gitlab.com/AOMediaCodec/SVT-AV1/-/issues/1753
# https://gitlab.com/AOMediaCodec/SVT-AV1/-/blob/master/Docs/CommonQuestions.md#what-presets-do

# FOR VVENC
# https://github.com/fraunhoferhhi/vvenc

import subprocess, os, sys, re, time, json, copy
from dataclasses import dataclass, asdict

a = time.time()
t_module=time

# Check working directory.
FILE_DIR_PATH = re.sub("[a-zA-Z0-9_\.]+.py","",__file__)
CURRENT_WORKING_DIR = os.getcwd()
if FILE_DIR_PATH != CURRENT_WORKING_DIR:
    print(f"FILE_DIR_PATH - {FILE_DIR_PATH}")
    print(f"CURRENT_WORKING_DIR - {CURRENT_WORKING_DIR}")
    
    print("Working directory mismatch. Working directory will be changed to the directory of __file__")
    os.chdir(FILE_DIR_PATH)
    print(f"FILE_DIR_PATH - {FILE_DIR_PATH}")
    print(f"changed CURRENT_WORKING_DIR - {os.getcwd()}")
    
    # raise Exception("Woking directory must match with FILE_DIR_PATH")

def run_command(command, comment, log_file):
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
    whisperx_large_v2 = "whisperx_large_v2"

@dataclass
class VInfo:
    season:str = "01"
    epnum:str = "1"
    input_extension:str = "mp4"
    encode_library_type:str = MethodType.svtapp
    passnum:str = "2"
    preset:str = "5"
    video_encode_format_depth:str = "10"
    target_width:str = "640"
    target_height:str = "360"
    # les simson 100kbps, one piece 125kbps-150kbps, solo leveling 350kbps
    target_bitrate:str = "150" # this target bitrate only for video not total bitrate of video+audio. ffmpeg -b (-b bitrate might be included audio bitrate)
    vf_filter_denoise_apply:bool = True
    
    audio_encode_library:str = "libopus"
    audio_bits:str = "64"
    audio_filters_apply:bool = False  # for french audio, preserving original audio will help a listening well.
    audio_filters:str = "highpass=f=100,lowpass=f=6000,acompressor,aresample=16000"
    
    
    #SVTAV1 settings
    # tune=0:film-grain=2:rc=1:min-qp=%MINQP%:max-qp=%MAXQP%:aq-mode=2
    svtav1_tune:str = "0"
    svtav1_film_grain:str = "2"
    svtav1_rc:str = "1"  # 1 for VBR, 2 for CBR
    svtav1_min_qp:str = "26"
    svtav1_max_qp:str = "63"
    svtav1_aq_mode:str = "2"
    svtav1_lookahead:str = "60"
    svtav1_scene_change_detection:bool = True  # If true, it cause 60% increase of encoding time. 3.5h -> 5.5h
    # to enable VBR mode and set the flexibility of the effective bitrate using --bias-pct. 
    svtav1_use_bias_pct:bool = False # not working. it might be wrong flag. it seems for another library.
    #In the VBR mode, the rate control algorithm matches the rate over the sequence. Use --gop-constraint-rc 1 to enable rate matching over each gop. This feature is currently supported with VBR mode when Gop size is greater than 119.
    # --gop-constraint-rc 1
    svtav1_use_gop_constraint:bool = False # set default. not to --gop-constraint-rc 0
    svtav1_maxsection_pct:str = "7000" # GOP max bitrate (expressed as a percentage of the target rate). It seems 10000 => 100%. 2000(default)
    svtav1_undershoot_pct:str = "10" # Allowable datarate undershoot (min) target (%), default depends on the rate control mode (25 for CBR, 50 for VBR)
    svtav1_overshoot_pct:str = "100" # Allowable datarate overshoot (max) target (%), default depends on the rate control mode
    svtav1_enable_qm:bool = True

    #VVENC settings
    vvenc_threads:str = "12"
    # medium also super slow. svtapp 2-pass of task with preset 4 for 1.2h == vvenc medium with 8threads over 12h. 
    # may need to look into the options carefully.
    vvenc_preset:str = "fast"
    
    #Wisper settings
    # whisper medium may work not correctly on my labtop and turbo stuck many times weird words such as sous titrage? CT' 501 ... 
    # even turbo and large-v3-turbo also stuck many times but small model less likely produce weird words this may be cause by the lack of my labtop resources.
    # whisper_model:str = WhisperModelType.small

    #whisperx much better fast
    whisper_model:str = WhisperModelType.whisperx_large_v2
    

    # dependent settings and targets names
    input_video_file:str = ""
    default_input_video_file:str = ""
    fps:str = "" # For ambiguous rate 23.98.... Will be extracted from the input video. 
    frameratenumerator:str = ""
    frameratedenominator:str = ""
    inter_video_encode_format = ""
    target_max_bitrate:str = ""
    default_target_max_bitrate:str = ""
    vf_filters:str = ""

    padded_epnum:str = ""
    suffix_s_e:str = ""

    def __post_init__(self):
        self.update_values()


    def update_values(self,input_v_dict={}):
        for k,v in input_v_dict.items():
            setattr(self,k,v)
        
        # set defaults values including input_file_name etc..
        self._set_dependent_default_values_targets()  

        # if input_v_dict has some values such as input_file_name or target_max_bitrate, then update again for the values.
        # or erase below duplicated update after separate some values with its default values
        for k,v in input_v_dict.items():
            setattr(self,k,v)

    def _set_dependent_default_values_targets(self):
        self.padded_epnum:str =  f"{self.epnum:>02}"
        self.suffix_s_e:str = f"s{self.season}_e{self.padded_epnum}"
        self.vf_filters:str = f"hqdn3d=4:3:6:6,scale={self.target_width}:{self.target_height}" if self.vf_filter_denoise_apply else f"scale={self.target_width}:{self.target_height}"
        self.inter_video_encode_format:str = "yuv420p" if self.video_encode_format_depth == "8" else "yuv420p10le"
        self.target_max_bitrate:str =  str(int(int(self.target_bitrate)*1.7))+"k"

        # setting defaults targets
        self.default_input_video_file = f"Episode {self.epnum}.{self.input_extension}"
        
        self.inter_yuv_video_file = f"{self.suffix_s_e}_intermediate.yuv"
        self.inter_stats_file = f"{self.suffix_s_e}_{self.encode_library_type}_stats.stat"
        self.inter_stats_vvenc_file = f"{self.suffix_s_e}_{self.encode_library_type}_stats.json"

        self.output_vvenc_266_file = f"{self.suffix_s_e}_{self.encode_library_type}_encoded.266"
        self.output_ivf_video_file = f"{self.suffix_s_e}_{self.encode_library_type}_encoded.ivf"
        self.output_video_file = \
            f"{self.suffix_s_e}_{self.encode_library_type}_pa{self.passnum}_p{self.preset}_{"8bit" if self.video_encode_format_depth == "8" else "10bit"}_s{self.target_width}_{self.target_height}_b{self.target_bitrate}k_" \
            + f"{'vf4366_' if self.vf_filter_denoise_apply else ''}" \
            + f"a{self.audio_bits}k_" 
        if self.encode_library_type in [MethodType.svtapp, MethodType.libsvtav1]:
            if self.svtav1_scene_change_detection:
                self.output_video_file += "scd1_"
            self.output_video_file += f"t{self.svtav1_tune}fg{self.svtav1_film_grain}qp{self.svtav1_min_qp}-{self.svtav1_max_qp}_aq{self.svtav1_aq_mode}_"
            self.output_video_file += f"o{self.svtav1_overshoot_pct}u{self.svtav1_undershoot_pct}_"
            self.output_video_file += f"lad{self.svtav1_lookahead}_"
        elif self.encode_library_type == MethodType.vvenc:
            self.output_video_file += f"t{self.vvenc_threads}_{self.vvenc_preset}_"
        self.output_video_file += ".mkv"

        self.output_orig_audio_file = f"{self.suffix_s_e}_orig_audio.m4a"
        self.output_encoded_audio_file = f"{self.suffix_s_e}_encoded_audio.opus"
        self.output_srt_file = f"{self.suffix_s_e}_orig_audio.srt"
        self.output_video_srt_merged_file = f"{self.output_video_file.replace('.mkv','_with_srt.mkv')}"
        self.output_video_srt_meta_merged_file = f"{self.suffix_s_e}.mkv"

        self.debug_log_file = f"{self.suffix_s_e}_LOG.txt"
        self.debug_log_whisper_file = f"{self.suffix_s_e}_LOG_WHISPER.txt"
        self.debug_log_whisperx_file = f"{self.suffix_s_e}_LOG_WHISPERX.txt"

    
    
class EncodeManager:
    def __init__(self,initial_v_dict:dict={}):
        self.v_info = VInfo()
        if len(initial_v_dict):
            self.v_info.update_values(initial_v_dict)
        
    def update_v_info(self,v_dict={}):
        self.v_info.update_values(v_dict)

    def run_command_dict_single_command(self,command,comment,debug_log_file,command_cli):
        start_time = time.time()
        run_command(command,comment,debug_log_file)
        time_info = f"command done. {time.time()-start_time:.2f}"

        with open(debug_log_file, 'a') as log:
            log.write("\n")
            log.write(command_cli)
            log.write("\n")
            log.write(comment)
            log.write("\n")
            log.write(time_info)
            log.write("\n")
        print(command_cli)
        print(comment)
        print(time_info)

    def run_command_dict(self,command_dict:dict,is_save_v_info=False):
        # command_dict["clear_files"] = [inter_yuv_video_file]
        
        if "command_sets" in command_dict:
            for i in range(len(command_dict["command_sets"])):
                self.run_command_dict_single_command(command_dict["command_sets"][i],command_dict["comment_sets"][i],command_dict["debug_log_file"],command_dict["command_sets_cli"][i])
        else:
            self.run_command_dict_single_command(command_dict["command"],command_dict["comment"],command_dict["debug_log_file"],command_dict["command_cli"])
        
        if is_save_v_info:
            with open(command_dict["debug_log_file"],'a') as log:
                log.write(f"All of settings : ")
                log.write("\n")
                log.write(f"{self.v_info}")
        
        if "clear_files" in command_dict:
            for i in command_dict["clear_files"]:
                if os.path.exists(i):
                    os.remove(i)



    def get_input_video_info(self, input_video_file):
        """ffprobe를 사용하여 비디오 해상도와 프레임 속도를 추출"""
        # 해상도 추출
        resolution = subprocess.check_output([
            'ffprobe', '-v', 'error', '-select_streams', 'v:0',
            '-show_entries', 'stream=width,height', '-of', 'default=noprint_wrappers=1:nokey=1', input_video_file
        ]).decode().strip().split('\n')
        width, height = resolution[0], resolution[1]
        
        # 프레임 속도 추출
        fps = subprocess.check_output([
            'ffprobe', '-v', 'error', '-select_streams', 'v:0',
            '-show_entries', 'stream=r_frame_rate', '-of', 'default=noprint_wrappers=1:nokey=1', input_video_file
        ]).decode().strip()

        # def modify_global_variables(v_dict={}):
        # input_video_file = v_dict["input_video_file"] if "input_video_file" in v_dict else INPUT_VIDEO_FILE

        # orig_width, orig_height, orig_fps = get_video_info(INPUT_VIDEO_FILE)
        print(f"input video info from : {input_video_file}")
        print(f"input width :{width}")
        print(f"input height :{height}")
        print(f"input fps :{fps}")

        out_ = {"width" : width,
                "height" : height,
                "fps" : fps}
        if "/" in fps:
            frameratenumerator,frameratedenominator= fps.split("/")
            out_["frameratenumerator"] = frameratenumerator
            out_["frameratedenominator"] = frameratedenominator
            # self.v_dict["frameratenumerator"], self.v_dict["frameratedenominator"] = fps.split("/")
        # else:
        #     self.v_dict['fps'] = fps 

        # orig_fps value is 24000/1001 then FPS=23.9870..... this may cause unsync with audio.
        # should try again for FPS=24000/1001 
        # There's bug. The app cannot accept correctly. It truncate value from 23.976... to 23. That's the reason the video length changed. To fix it. encoding with the value 24. 
        # You need to modify the result of encoding to fix the length when you merged it with ffmpeg.
        # print(f"encoding WIDTH : {self.target_width}")
        # print(f"encoding HEIGHT : {self.target_height}")
        # print(f"encoding FPS : {fps}")
        print(f"encoding frameratenumerator : {out_['frameratenumerator'] if 'frameratenumerator' in out_ else '0'}")
        print(f"encoding frameratedenominator : {out_['frameratedenominator'] if 'frameratedenominator' in out_ else '0'}")
        return out_

    def get_num_audio_from_video(self, input_video_file:str):
        cmd = [
                'ffprobe',
                '-v', 'error',
                '-show_streams',
                '-select_streams', 'a',  # 오디오 스트림만 선택
                '-print_format', 'json',
                input_video_file
            ]
            
            # ffprobe 실행 및 출력 캡처
        result = subprocess.run(cmd, capture_output=True, text=True, check=True)
        streams_data = json.loads(result.stdout)
        
        # 오디오 스트림 개수 계산
        audio_streams = streams_data.get('streams', [])
        return len(audio_streams)

    def get_command_dict_for_audio_orig_extract(self,input_dict:dict={}):
        command_dict:dict = copy.deepcopy(input_dict)
        input_video_file = command_dict.get("input_video_file",self.v_info.input_video_file)
        output_orig_audio_file = command_dict.get("output_orig_audio_file",self.v_info.output_orig_audio_file)
        debug_log_file = command_dict.get("debug_log_file",self.v_info.debug_log_file)

        if input_video_file == '':
            input_video_file = self.v_info.default_input_video_file

        command_dict["command"] = [
            "ffmpeg", "-y",
            "-i", input_video_file,
            "-vn", "-acodec", "copy", output_orig_audio_file
        ]
        command_dict["comment"] = f"Save extracted original audio.  - {command_dict["command"]} \n"
        command_dict["command_cli"] = " ".join(command_dict["command"])
        command_dict["debug_log_file"] = debug_log_file
        command_dict["orig_audio_alredy_exist"] = os.path.isfile(output_orig_audio_file)
        return command_dict

    def get_command_dict_for_audio_orig_extract_all(self,input_dict:dict={},num_audio=0):
        command_dict:dict = copy.deepcopy(input_dict)
        input_video_file = command_dict.get("input_video_file",self.v_info.input_video_file)
        output_orig_audio_file = command_dict.get("output_orig_audio_file",self.v_info.output_orig_audio_file)
        debug_log_file = command_dict.get("debug_log_file",self.v_info.debug_log_file)

        if input_video_file == '':
            input_video_file = self.v_info.default_input_video_file

        if num_audio == 0:
                # ffprobe 명령어로 스트림 정보 가져오기
            num_audio = self.get_num_audio_from_video(input_video_file)
        if num_audio == 1:
            return self.get_command_dict_for_audio_orig_extract(input_dict)
        
        command_sets = []
        comment_sets = []
        for i in range(num_audio):
            output_orig_audio_file_i = re.sub(r"(\..+)$",lambda m : f"_{i}"+m.group(1),output_orig_audio_file)
            command = [
            "ffmpeg", "-y",
            "-i", input_video_file,
            "-vn",
            "-map",
            # f"0:{i+1}",
            f"0:a:{i}",
            f"-c:a:{i}",
            "copy",
            output_orig_audio_file_i
            ]
            command_sets.append(command)
            comment_sets.append(f"Save extracted original audio {i}.  - {command} \n")
        
        command_dict["command_sets"] = command_sets
        command_dict["comment_sets"] = comment_sets
        command_dict["command_sets_cli"] = [" ".join(i) for i in command_sets]
        command_dict["debug_log_file"] = debug_log_file
        return command_dict

    
    def get_command_dict_for_encoding_audio(self,input_dict:dict={}):
        command_dict:dict = copy.deepcopy(input_dict)
        input_video_file = command_dict.get("input_video_file",self.v_info.input_video_file)
        audio_encode_library = command_dict.get("audio_encode_library",self.v_info.audio_encode_library)
        audio_bits = command_dict.get("audio_bits",self.v_info.audio_bits)
        output_encoded_audio_file = command_dict.get("output_encoded_audio_file",self.v_info.output_encoded_audio_file)
        debug_log_file = command_dict.get("debug_log_file",self.v_info.debug_log_file)

        if input_video_file == '':
            input_video_file = self.v_info.default_input_video_file

        command_dict["command"]=[
                "ffmpeg", "-y"
                "-i", input_video_file,
                "-vn",
                # "-af", A_FILTERS,  # It's quite hard to listen even original audio. Uncertain this is cuase by audio filter or just opus. Disable filter.
                "-c:a", 
                audio_encode_library, 
                "-b:a", 
                f"{audio_bits}k", 
                # "-application", "voip", 
                output_encoded_audio_file
            ]
        command_dict["comment"] = f"encoding audio - {command_dict["command"]} \n"
        command_dict["command_cli"] = " ".join(command_dict["command"])
        command_dict["debug_log_file"] = debug_log_file
        return command_dict
    
    def get_command_dict_for_encoding_audio_all(self,input_dict:dict={},num_audio=0):
        command_dict:dict = copy.deepcopy(input_dict)
        input_video_file = command_dict.get("input_video_file",self.v_info.input_video_file)
        audio_encode_library = command_dict.get("audio_encode_library",self.v_info.audio_encode_library)
        audio_bits = command_dict.get("audio_bits",self.v_info.audio_bits)
        output_encoded_audio_file = command_dict.get("output_encoded_audio_file",self.v_info.output_encoded_audio_file)
        debug_log_file = command_dict.get("debug_log_file",self.v_info.debug_log_file)

        if input_video_file == '':
            input_video_file = self.v_info.default_input_video_file
        if num_audio == 0:
            num_audio = self.get_num_audio_from_video(input_video_file)

        command_sets=[]
        comment_sets=[]
        for i in range(num_audio):
            output_encoded_audio_file_i = re.sub(r"(\..+)$",lambda m : f"_{i}"+m.group(1),output_encoded_audio_file)
            command = [
                "ffmpeg", "-y",
                "-i", 
                input_video_file, 
                "-vn",
                # "-af", A_FILTERS,  # It's quite hard to listen even original audio. Uncertain this is cuase by audio filter or just opus. Disable filter.
                "-map",
                # f"0:{i+1}",
                f"0:a:{i}",
                f"-c:a:{i}", 
                audio_encode_library, 
                # f"-b:a:{i}", 
                f"-b:a", 
                f"{audio_bits}k", 
                # "-application", "voip", 
                # self.v_dict["output_encoded_audio_file"]
                output_encoded_audio_file_i
            ]
            command_sets.append(command)
            comment_sets.append(f"encoding audio - {command} \n")

        command_dict["command_sets"] = command_sets
        command_dict["comment_sets"] = comment_sets
        command_dict["command_sets_cli"] = [" ".join(i) for i in command_sets]
        command_dict["debug_log_file"] = debug_log_file
        return command_dict


    def get_command_dict_for_yuv_video(self,input_dict:dict={}):
        command_dict:dict = copy.deepcopy(input_dict)
        input_video_file = command_dict.get("input_video_file",self.v_info.input_video_file)
        # vf_filter_denoise_apply = getattr(command_dict,"vf_filter_denoise_apply",self.v_info.vf_filter_denoise_apply)
        vf_filters = command_dict.get("vf_filters",self.v_info.vf_filters)
        inter_video_encode_format = command_dict.get("inter_video_encode_format",self.v_info.inter_video_encode_format)
        inter_yuv_video_file = command_dict.get("inter_yuv_video_file",self.v_info.inter_yuv_video_file)
        debug_log_file = command_dict.get("debug_log_file",self.v_info.debug_log_file)

        if input_video_file == '':
            input_video_file = self.v_info.default_input_video_file

        command_dict["command"] = [
                "ffmpeg", "-y",
                "-i", 
                input_video_file, 
                # "-vf" if vf_filter_denoise_apply else "",
                "-vf",
                # vf_filters if vf_filter_denoise_apply else "",
                vf_filters,
                "-pix_fmt", inter_video_encode_format,
                "-f", "rawvideo",
                inter_yuv_video_file
            ]
        command_dict["comment"] = f"Making intermediate video - {command_dict["command"]} \n"
        command_dict["command_cli"] = " ".join(command_dict["command"])
        command_dict["debug_log_file"] = debug_log_file
        return command_dict
    
    def get_command_dict_for_mkv_with_inter_video_audio1(self,input_dict:dict={}):
        command_dict:dict = copy.deepcopy(input_dict)
        # input_video_file = command_dict.get("input_video_file",self.v_info.input_video_file)
        output_ivf_video_file = command_dict.get("output_ivf_video_file",self.v_info.output_ivf_video_file)
        encode_library_type = command_dict.get("encode_library_type",self.v_info.encode_library_type)
        output_vvenc_266_file = command_dict.get("output_vvenc_266_file",self.v_info.output_vvenc_266_file)
        output_encoded_audio_file = command_dict.get("output_encoded_audio_file",self.v_info.output_encoded_audio_file)
        output_video_file = command_dict.get("output_video_file",self.v_info.output_video_file)
        debug_log_file = command_dict.get("debug_log_file",self.v_info.debug_log_file)

        command_dict["command"]=[
            "ffmpeg", "-y",
            "-i", 
            output_ivf_video_file if encode_library_type in [MethodType.svtapp, MethodType.libsvtav1] else output_vvenc_266_file, 
            "-i",
            output_encoded_audio_file, 
            "-c:v", "copy",
            "-c:a", "copy",
            output_video_file
        ]
        command_dict["comment"]=f"Make MKV video with single audio - {command_dict["command"]}"
        command_dict["command_cli"] = " ".join(command_dict["command"])
        command_dict["debug_log_file"] = debug_log_file
        return command_dict
    
    def get_command_dict_for_mkv_with_srt(self,input_dict:dict={}):
        command_dict:dict = copy.deepcopy(input_dict)
        output_video_file = command_dict.get("output_video_file",self.v_info.output_video_file)
        output_srt_file = command_dict.get("output_srt_file",self.v_info.output_srt_file)
        output_video_srt_meta_merged_file = command_dict.get("output_video_srt_meta_merged_file",self.v_info.output_video_srt_meta_merged_file)
        debug_log_file = command_dict.get("debug_log_file",self.v_info.debug_log_file)

        command_dict["command"]=[
            "ffmpeg", "-y",
            "-i", 
            output_video_file,
            "-i",
            output_srt_file, 
            "-map", "0:v:0", "-map", "0:a:0", "-map", "1:s",
            "-c:v", "copy",
            "-c:a", "copy",
            # "-c:s", "copy",
            # "-c:s", "mov_text",
            "-c:s", "srt",
            "-c:t", "copy",
            "-metadata", f"comment=encode-{re.sub("\.[a-zA-Z0-9]+$","",output_video_file)}",  
            "-metadata:s:v:0", f"title=encode-{re.sub("\.[a-zA-Z0-9]+$","",output_video_file)}",  
            # "-disposition:s:2", "default",
            "-metadata:s:a:0", "language=fre", "-metadata:s:a:0", f'title="French Audio"', 
            "-metadata:s:s:0", "language=fre", "-metadata:s:s:0", f'title="French {self.v_info.whisper_model}"',
            output_video_srt_meta_merged_file
        ]
        # ffmpeg -i input.mp4 -vf subtitles=subtitles.srt output.mp4
        command_dict["comment"]=f"Make MKV video with srt - {command_dict["command"]}"
        command_dict["command_cli"] = " ".join(command_dict["command"])
        command_dict["debug_log_file"] = debug_log_file
        return command_dict


    def get_command_dict_for_mkv_with_inter_video_audio_all(self,input_dict:dict={},num_audio=0):
        command_dict:dict = copy.deepcopy(input_dict)
        input_video_file = command_dict.get("input_video_file",self.v_info.input_video_file)
        output_ivf_video_file = command_dict.get("output_ivf_video_file",self.v_info.output_ivf_video_file)
        encode_library_type = command_dict.get("encode_library_type",self.v_info.encode_library_type)
        output_vvenc_266_file = command_dict.get("output_vvenc_266_file",self.v_info.output_vvenc_266_file)
        output_encoded_audio_file = command_dict.get("output_encoded_audio_file",self.v_info.output_encoded_audio_file)
        output_video_file = command_dict.get("output_video_file",self.v_info.output_video_file)
        debug_log_file = command_dict.get("debug_log_file",self.v_info.debug_log_file)

        if input_video_file == '':
            input_video_file = self.v_info.default_input_video_file

        if num_audio == 0:
            num_audio = self.get_num_audio_from_video(input_video_file)
        
        command_dict["command"]=[
            "ffmpeg", "-y",
            "-i", 
            output_ivf_video_file if encode_library_type in [MethodType.svtapp, MethodType.libsvtav1] else output_vvenc_266_file, 
        ]
        for i in range(num_audio):
            output_encoded_audio_file_i = re.sub(r"(\..+)$",lambda m : f"_{i}"+m.group(1),output_encoded_audio_file)
            command_dict["command"] += [
                "-i",
                output_encoded_audio_file_i,
            ]
        command_dict["command"] += [
            "-map", "0:v", "-c:v:0", "copy",
        ]
        for i in range(num_audio):
            command_dict["command"] += [
                "-map",f"{i+1}:a",
                # f"-c:a:{i+1}", "copy"
                f"-c:a:{i}", "copy"
            ]
        command_dict["command"] += [
            "-c:s", "copy", "-c:t", "copy"
        ]
        command_dict["command"] += [output_video_file]
        #     "-i",
        #     self.c_dict["output_encoded_audio_file"], 
        #     "-c:a", "copy",
        #     self.v_dict["output_video_file"]
        # ]
        command_dict["comment"]=f"Make MKV video with all of audio - {command_dict["command"]}"
        command_dict["command_cli"] = " ".join(command_dict["command"])
        command_dict["debug_log_file"] = debug_log_file
        return command_dict
    
    def get_command_dict_add_meta_info_with_video_name(self,input_dict:dict={}):
        command_dict:dict = copy.deepcopy(input_dict)
        output_video_file = command_dict.get("output_video_file",self.v_info.output_video_file)
        output_video_with_meta_file = command_dict.get("output_video_with_meta_file",self.v_info.output_video_srt_meta_merged_file)
        debug_log_file = command_dict.get("debug_log_file",self.v_info.debug_log_file)

        command_dict["command"]=[
            "ffmpeg", "-y",
            "-i", 
            output_video_file,
            "-map", "0:v:0", "-map", "0:a",
            "-c:v", "copy",
            "-c:a", "copy",
            "-c:s", "copy",
            # "-c:s", "mov_text",
            # "-c:s", "srt",
            "-c:t", "copy",
            "-metadata", f"comment=encode-{re.sub("\.[a-zA-Z0-9]+$","",output_video_file)}",  
            "-metadata:s:v:0", f"title=encode-{re.sub("\.[a-zA-Z0-9]+$","",output_video_file)}",  
            # "-disposition:s:2", "default",
            output_video_with_meta_file
        ]
        # ffmpeg -i input.mp4 -vf subtitles=subtitles.srt output.mp4
        command_dict["comment"]=f"Make mkv video having meta info from video name - {command_dict["command"]}"
        command_dict["command_cli"] = " ".join(command_dict["command"])
        command_dict["debug_log_file"] = debug_log_file
        return command_dict

    
    def get_command_dict_encode_video_svtapp(self,input_dict:dict={}, num_passes=2):
        # 3-pass not working correctly need more test.
        command_dict:dict = copy.deepcopy(input_dict)
        input_video_file = command_dict.get("input_video_file",self.v_info.input_video_file)
        inter_yuv_video_file = command_dict.get("inter_yuv_video_file",self.v_info.inter_yuv_video_file)
        video_encode_format_depth = command_dict.get("video_encode_format_depth",self.v_info.video_encode_format_depth)
        inter_stats_file = command_dict.get("inter_stats_file",self.v_info.inter_stats_file)
        output_ivf_video_file = command_dict.get("output_ivf_video_file",self.v_info.output_ivf_video_file)
        debug_log_file = command_dict.get("debug_log_file",self.v_info.debug_log_file)

        if input_video_file == '':
            input_video_file = self.v_info.default_input_video_file

        # (width,height,fps,frameratenumerator,frameratedenominator)
        input_video_info = self.get_input_video_info(input_video_file)

        command_sets = []
        comment_sets = []
        command = [
            "SvtAv1EncApp", "-i", inter_yuv_video_file,
            "-w", self.v_info.target_width, "-h", self.v_info.target_height,
            "--input-depth", video_encode_format_depth,
            # '--fps', FPS,   # There's bug. The app cannot accept correctly. It truncate value from 23.976... to 23. That's the reason the video length changed. To fix it. encoding with the value 24. 
            # --fps should be 24 not 23.976... You need to modify the result of encoding to fix the length when you merged it with ffmpeg.
            '--rc', self.v_info.svtav1_rc, #--rc 1 for VBA not CBR
            '--tbr', f"{self.v_info.target_bitrate}k", 
            "--scd", "1" if self.v_info.svtav1_scene_change_detection else "0", 
            "--lookahead", self.v_info.svtav1_lookahead,
            "--tune", self.v_info.svtav1_tune,
            "--min-qp", self.v_info.svtav1_min_qp,
            "--max-qp", self.v_info.svtav1_max_qp,
            "--aq-mode", self.v_info.svtav1_aq_mode,
            "--film-grain", self.v_info.svtav1_film_grain,
            "--maxsection-pct", self.v_info.svtav1_maxsection_pct,
            "--undershoot-pct", self.v_info.svtav1_undershoot_pct,
            "--overshoot-pct", self.v_info.svtav1_overshoot_pct,
            "--enable-qm", "1" if self.v_info.svtav1_enable_qm else "0",
            "--progress", "0", # The log file would be dirty when print the progress. just assume from the file sizes.
            "--fps-num", input_video_info["frameratenumerator"],
            "--fps-denom", input_video_info["frameratedenominator"],
            # "--bias-pct" if self.v_info.svtav1_use_bias_pct else "", not working
            "--preset", self.v_info.preset
        ]

        if num_passes == 2:
            command1 = command + ["--pass", "1", "--stats", inter_stats_file]
            command2 = command + ["--pass", "2", "--stats", inter_stats_file, "-b", output_ivf_video_file]
            command_sets.append(command1)
            command_sets.append(command2)
            comment_sets.append(f"SVTAV1APP: encoding-2pass-1 with next command - {command1}")
            comment_sets.append(f"SVTAV1APP: encoding-2pass-2 with next command - {command2}")
            command_dict["command_sets"] = command_sets
            command_dict["comment_sets"] = comment_sets
            command_dict["command_sets_cli"] = [" ".join(i) for i in command_sets]
        else:
            command += ["-b", output_ivf_video_file]
            command_dict["command"] = command
            command_dict["comment"] = f"Encode video with SvtAv1App - {command_dict["command"]}"
            command_dict["command_cli"] = " ".join(command_dict["command"])
        
        command_dict["debug_log_file"] = debug_log_file
        command_dict["clear_files"] = [inter_yuv_video_file]
        return command_dict
    
    def get_command_dict_encode_video_libsvtav1(self, input_dict:dict={}):
        # ffmpeg's encoding using libsvtav1 library seems bitrate control not work properly or it depends on the version.
        # when I setting bitrate as -b 150k the final video has the bitrate including audio bitrate.
        # unlike this integrated library, when using seperately the apps then the target bitrate only consider video.
        # ffmpeg -loglevel info -i "%INPUT%" -vf "%FILTERS%" -c:v libsvtav1 -b:v %BITRATE% -bufsize %BUFSIZE%  -preset %PRESET1% -g %GOP% -svtav1-params "tune=0:film-grain=2:rc=1:min-qp=%MINQP%:max-qp=%MAXQP%:aq-mode=2" -c:a libopus -b:a %AUDIOBR% "%OUTPUT%" 
        # 2-pass not working correctly, on my ffmpeg version 2024 dec. Uncertain the version of march of 2025 would be fixed.
        command_dict:dict = copy.deepcopy(input_dict)
        input_video_file = command_dict.get("input_video_file",self.v_info.input_video_file)
        encode_library_type = command_dict.get("encode_library_type",self.v_info.encode_library_type)
        audio_encode_library = command_dict.get("audio_encode_library",self.v_info.audio_encode_library)
        audio_bits = command_dict.get("audio_bits",self.v_info.audio_bits)
        video_encode_format_depth = command_dict.get("video_encode_format_depth",self.v_info.video_encode_format_depth)
        output_video_file = command_dict.get("output_video_file",self.v_info.output_video_file)
        debug_log_file = command_dict.get("debug_log_file",self.v_info.debug_log_file)

        if input_video_file == '':
            input_video_file = self.v_info.default_input_video_file

        num_audio = self.get_num_audio_from_video(input_video_file)

        command = [
            "ffmpeg", "-y", "-i", input_video_file,
        ]
        for i in range(num_audio):
            command += [
                "-map",f"0:a:{i}",
                # f"-c:a:{i+1}", "copy"
                f"-c:a:{i}", audio_encode_library, f"-b:a:{i}", audio_bits] 
                # "-c:a", "libopus", "-b:a", f"{self.v_dict["audio_bits"]}k", 
        command += [
            "-c:s", "copy", "-c:t", "copy",
            "-vf", f"{self.v_info.vf_filters}", # handle scale as well.
            "-pix_fmt", f"{self.v_info.inter_video_encode_format}", 
            "-c:v", encode_library_type, 
            # "-b:v", BITRATE,  # use -tbr otherwise the bitrate of the video contain audio bitrate.
            # "-preset", self.v_dict["preset"], # preset in ffmpeg may differ with svtav1 so put the preset into svtav1-params 
            # "-g", LIBSVTAV1_VARIABLES["gop"],
            "-svtav1-params", 
            # :w={self.v_dict["target_width"]} \
            # :h={self.v_dict["target_height"]}   \
            f"preset={self.v_info.preset}"\
            +f":input-depth={video_encode_format_depth}"\
            +f":rc={self.v_info.svtav1_rc}"\
            +f":tbr={self.v_info.target_bitrate}"\
            +f'{":scd=1" if self.v_info.svtav1_scene_change_detection else ""}'\
            +f":lookahead={self.v_info.svtav1_lookahead}"\
            +f":tune={self.v_info.svtav1_tune}"\
            +f":min-qp={self.v_info.svtav1_min_qp}"\
            +f":max-qp={self.v_info.svtav1_max_qp}"\
            +f":aq-mode={self.v_info.svtav1_aq_mode}"\
            +f":film-grain={self.v_info.svtav1_film_grain}"\
            +f":maxsection-pct={self.v_info.svtav1_maxsection_pct}"\
            +f":undershoot-pct={self.v_info.svtav1_undershoot_pct}" \
            +f":overshoot-pct={self.v_info.svtav1_overshoot_pct}"\
            +f":enable-qm={"1" if self.v_info.svtav1_enable_qm else "0"}"\
            +f":progress=0"\
            +f"{':bias-pct' if self.v_info.svtav1_use_bias_pct else ''}",
            output_video_file
        ]

        command_dict["command"] = command
        command_dict["comment"] = f"libsvtav1: encoding-2pass-1 with next command - {command_dict["command"]}"
        command_dict["command_cli"] = " ".join(command_dict["command"])
        command_dict["debug_log_file"] = debug_log_file
        return command_dict
    
    def get_command_dict_encode_video_vvenc(self,input_dict:dict={}, num_passes=2):
        command_dict:dict = copy.deepcopy(input_dict)
        input_video_file = command_dict.get("input_video_file",self.v_info.input_video_file)
        inter_yuv_video_file = command_dict.get("inter_yuv_video_file",self.v_info.inter_yuv_video_file)
        inter_stats_vvenc_file = command_dict.get("inter_stats_vvenc_file",self.v_info.inter_stats_vvenc_file)
        vvenc_preset = command_dict.get("vvenc_preset",self.v_info.vvenc_preset)
        video_encode_format_depth = command_dict.get("video_encode_format_depth",self.v_info.video_encode_format_depth)
        output_vvenc_266_file = command_dict.get("output_vvenc_266_file",self.v_info.output_vvenc_266_file)
        debug_log_file = command_dict.get("debug_log_file",self.v_info.debug_log_file)

        if input_video_file == '':
            input_video_file = self.v_info.default_input_video_file
        input_video_info = self.get_input_video_info(input_video_file)

        command_sets = []
        comment_sets = []
        command = [
            'vvencFFapp', '-i', inter_yuv_video_file, 
            "--preset", vvenc_preset,
            '-s', f"{self.v_info.target_width}x{self.v_info.target_height}", 
            "--TargetBitrate", f"{self.v_info.target_bitrate}k", "--MaxBitrate", self.v_info.target_max_bitrate,
            "--InputBitDepth", video_encode_format_depth,
            "--InternalBitDepth", video_encode_format_depth,
            "--Threads", self.v_info.vvenc_threads,
            "--TicksPerSecond", f"{int(int(input_video_info["frameratenumerator"])*4)}",
            "--FrameRate", input_video_info["frameratenumerator"], "--FrameScale", input_video_info["frameratedenominator"],
            "--PerceptQPA", "1",
            "-b", output_vvenc_266_file
        ]

        if num_passes == 2:
            command1 = command + ["--RCStatsFile", inter_stats_vvenc_file, "--Pass", "1"]
            command2 = command + ["--RCStatsFile", inter_stats_vvenc_file, "--Pass", "2"]
            command_sets += [command1,command2]
            comment_sets.append(f"vvencFFapp: encoding-2pass-1 with next command - {command1}")
            comment_sets.append(f"vvencFFapp: encoding-2pass-2 with next command - {command2}")
            command_dict["command_sets"] = command_sets
            command_dict["comment_sets"] = comment_sets
            command_dict["command_sets_cli"] = [" ".join(i) for i in command_sets]
        else:
            command_dict["command"] = command
            command_dict["comment"] = f"vvencFFapp: encoding-1pass with next command - {command}"
            command_dict["command_cli"] = " ".join(command_dict["command"])

        command_dict["debug_log_file"] = debug_log_file
        command_dict["clear_files"] = [inter_yuv_video_file]
        return command_dict
    

    def get_command_dict_subtitle_whisper(self,input_dict:dict={}):
        command_dict:dict = copy.deepcopy(input_dict)
        whisper_model = command_dict.get("whisper_model",self.v_info.whisper_model)
        # output_encoded_audio_file = getattr(command_dict,"output_encoded_audio_file",self.v_info.output_encoded_audio_file)
        output_orig_audio_file = command_dict.get("output_orig_audio_file",self.v_info.output_orig_audio_file)
        debug_log_whisper_file = command_dict.get("debug_log_whisper_file",self.v_info.debug_log_whisper_file)

        command_dict["command"] = [
            "whisper", output_orig_audio_file, "--language", "French", "--output_format","srt", "--model", whisper_model
        ]
        command_dict["comment"] = f"Making subtitle with whisper AI ({whisper_model})- {command_dict["command"]}"
        command_dict["command_cli"] = " ".join(command_dict["command"])
        command_dict["debug_log_file"] = debug_log_whisper_file
        return command_dict

    def get_command_dict_subtitle_whisper_x(self,input_dict:dict={}):
        command_dict:dict = copy.deepcopy(input_dict)
        whisper_model = command_dict.get("whisper_model",self.v_info.whisper_model)
        output_orig_audio_file = command_dict.get("output_orig_audio_file",self.v_info.output_orig_audio_file)
        debug_log_whisper_x_file = command_dict.get("debug_log_whisper_x_file",self.v_info.debug_log_whisperx_file)

        command_dict["command"] = [
            # currently support only large-v2 for multi language
            "whisperx", "--model", "large-v2","--language", "fr", "--compute_type", "int8", "--highlight_words", "True", "--output_format","srt", output_orig_audio_file
        ]
        command_dict["comment"] = f"Making subtitle with whisper_x AI ({whisper_model})- {command_dict["command"]}"
        command_dict["command_cli"] = " ".join(command_dict["command"])
        command_dict["debug_log_file"] = debug_log_whisper_x_file
        return command_dict

    def run_libsvtav1(self, input_dict={}):
        command_dict = self.get_command_dict_encode_video_libsvtav1(input_dict)
        self.run_command_dict(command_dict,is_save_v_info=True)
        

    def run_svtav1app(self, input_dict={}):
        times=[]
        start_time = time.time()
        temp_time = start_time
        
        command_dict = self.get_command_dict_for_audio_orig_extract_all(input_dict)
        self.run_command_dict(command_dict)
        times.append(f"{command_dict["comment"] if "comment" in command_dict else command_dict["comment_sets"][-1]} \n done. "+f" - {time.time()-temp_time:.2f} \n")
        command_dict = self.get_command_dict_for_encoding_audio_all(input_dict)
        self.run_command_dict(command_dict)
        times.append(f"{command_dict["comment"] if "comment" in command_dict else command_dict["comment_sets"][-1]} \n done. "+f" - {time.time()-temp_time:.2f} \n")
        temp_time = time.time()
        command_dict = self.get_command_dict_for_yuv_video(input_dict)
        self.run_command_dict(command_dict)
        times.append(f"{command_dict["comment"] if "comment" in command_dict else command_dict["comment_sets"][-1]} \n done. "+f" - {time.time()-temp_time:.2f} \n")
        temp_time = time.time()
        command_dict = self.get_command_dict_encode_video_svtapp(input_dict)
        self.run_command_dict(command_dict)
        times.append(f"{command_dict["comment"] if "comment" in command_dict else command_dict["comment_sets"][-1]} \n done. "+f" - {time.time()-temp_time:.2f} \n")
        temp_time = time.time()
        command_dict = self.get_command_dict_for_mkv_with_inter_video_audio_all(input_dict)
        self.run_command_dict(command_dict,is_save_v_info=True)
        times.append(f"{command_dict["comment"] if "comment" in command_dict else command_dict["comment_sets"][-1]} \n done. "+f" - {time.time()-temp_time:.2f} \n")
        temp_time = time.time()
        times.append(f"Total time : {temp_time-start_time:.2f} \n")

        with open(command_dict["debug_log_file"], 'a') as log:
            for i in times:
                log.write(i)
                print(i)

    def run_vvenc(self, input_dict={}):
        command_dict = self.get_command_dict_for_encoding_audio_all(input_dict)
        self.run_command_dict(command_dict)
        command_dict = self.get_command_dict_for_yuv_video(input_dict)
        self.run_command_dict(command_dict)
        command_dict = self.get_command_dict_encode_video_vvenc(input_dict)
        self.run_command_dict(command_dict)
        command_dict = self.get_command_dict_for_mkv_with_inter_video_audio_all(input_dict)
        self.run_command_dict(command_dict,is_save_v_info=True)
        

    def run_whisper(self, input_dict={}):
        command_dict = self.get_command_dict_for_audio_orig_extract(input_dict)
        self.run_command_dict(command_dict)
        command_dict = self.get_command_dict_subtitle_whisper(input_dict)
        self.run_command_dict(command_dict)

    def run_whisper_x(self, input_dict={}):
        command_dict = self.get_command_dict_for_audio_orig_extract(input_dict)
        if command_dict["orig_audio_alredy_exist"] == False:
            self.run_command_dict(command_dict)
        command_dict = self.get_command_dict_subtitle_whisper_x(input_dict)
        self.run_command_dict(command_dict)
    
    def run_merge_encoded_video_subtitle(self,input_dict={}):
        command_dict = self.get_command_dict_for_mkv_with_srt(input_dict)
        self.run_command_dict(command_dict)

    def run_make_video_with_metadata_from_video_name(self,input_dict={}):
        command_dict = self.get_command_dict_add_meta_info_with_video_name(input_dict)
        self.run_command_dict(command_dict)

def main():
    def run_make_video_with_metadata(info_dict={}):
        start:int = info_dict["start"]
        end:int = info_dict["end"]
        for i in range(start,end):
            info_dict["epnum"] = str(i)
            em = EncodeManager(info_dict)
            em.run_make_video_with_metadata_from_video_name(info_dict)

    def run_merge(info_dict={}):
        start:int = info_dict["start"]
        end:int = info_dict["end"]
        for i in range(start,end):
            info_dict["epnum"] = str(i)
            em = EncodeManager(info_dict)
            em.run_merge_encoded_video_subtitle(info_dict)

    def run_svt(info_dict={}):
        start:int = info_dict["start"]
        end:int = info_dict["end"]
        sleep_time:int = info_dict.get('sleep_time',600)
        
        for i in range(start,end):
            info_dict["epnum"] = str(i)
            em = EncodeManager(info_dict)
            em.run_svtav1app()
            time.sleep(sleep_time)
        
    def run_whisper_x(info_dict:dict={}):
        start:int = info_dict["start"]
        end:int = info_dict["end"]
        sleep_time:int = info_dict.get('sleep_time',300)
        
        for i in range(start,end):
            info_dict["epnum"] = str(i)
            em = EncodeManager(info_dict)
            em.run_whisper_x()
            time.sleep(sleep_time)

    # 640x360,  960x540 (qHD), 1280x720 (HD), 1600x900 (HD+), 1920x1080 (FHD)
    info_ = {
        "start" : 8,
        "end" : 15,
        # "output_video_file" : "s02_e143_svtapp_pa2_p5_10bit_s640_360_b150k_vf4366_a64k_scd1_t0fg2qp26-63_aq2_o100u10_lad60_.mkv",    
        "season" : "01",
        "epnum" : "",
        # "encode_library_type" : MethodType.svtapp,
        "encode_library_type" : MethodType.whisper,
        "whisper_model" : WhisperModelType.whisperx_large_v2,
        # "whisper_model" : WhisperModelType.large_v3_turbo,
        "sleep_time" : 150,
        "input_extension" : "mp4",
        "preset" : "3",
        "target_bitrate" : "150",
        "target_width" : "640",
        "target_height" : "360",
        "svtav1_scene_change_detection" : True,
        "svtav1_film_grain" : "2",
        }
    run_whisper_x(info_)
    # run_svt(info_)
    # run_merge(info_)
    # run_make_video_with_metadata(info_)

if __name__ == "__main__":
    main()