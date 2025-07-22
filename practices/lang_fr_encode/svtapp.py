import subprocess
import sys
import os
import pathlib,re


# 사용자 설정
SEASON = "02"
EPNUM = "10"
PADDEDEPNUM = f"{EPNUM:>02}"
SUFFIX_S_E = f"s{SEASON}_e{PADDEDEPNUM}"
FILE_DIR_PATH = re.sub("[a-z0-9]+.py","",__file__)
CURRENT_WORKING_DIR = os.getcwd()
if FILE_DIR_PATH != CURRENT_WORKING_DIR:
    print(f"FILE_DIR_PATH - {FILE_DIR_PATH}")
    print(f"CURRENT_WORKING_DIR - {CURRENT_WORKING_DIR}")
    raise Exception("Woking directory must match with FILE_DIR_PATH")
# INPUT_VIDEO_FILE = f"{FILE_DIR_PATH}Episode {EPNUM}.mp4" # if the log file contain of full file path would look bad for debugging.
INPUT_VIDEO_FILE = f"Episode {EPNUM}.mp4"
INTER_FORMAT = "yuv420p"
WIDTH = 640
HEIGHT = 360
BITRATE = 300  # kbps
FPS = 0  # For ambiguous rate 23.98.... Will be extracted from the input video. 
INPUT_VIDEO_DEPTH = 8
VF_FILTER_DENOISE = False
VF_FILTERS= f"hqdn3d=4:3:6:6,scale={WIDTH},{HEIGHT}" if VF_FILTER_DENOISE else f"scale={WIDTH},{HEIGHT}"

PASSNUM=3
PRESET1=4
PRESET2=4
PRESET3=2
PRESET_TOTAL = f"{PRESET1}{PRESET2}{PRESET3}" if PASSNUM==3 else f"{PRESET1}{PRESET2}"
if PASSNUM == 1:
    PRESET_TOTAL=PRESET1


A_BITS = 64
A_FILTERS = "highpass=f=100,lowpass=f=6000,acompressor,aresample=16000"
# 중간 파일
YUV_VIDEO_FILE = f"{SUFFIX_S_E}intermediate.yuv"
INTER_AUDIO_FILE = f"{SUFFIX_S_E}inter_audio.opus"
STATS_FILE = f"{SUFFIX_S_E}stats.log"
VIDEO_FILE = f"{SUFFIX_S_E}encoded.ivf"

OUTPUT_FILE = f"{SUFFIX_S_E}_pass{PASSNUM}p{PRESET_TOTAL}_vf{"4:3:6:6" if VF_FILTER_DENOISE else ""}s{WIDTH}_{HEIGHT}_b{BITRATE}k.mkv"
OUTPUT_ORIG_AUDIO_FILE = f"{SUFFIX_S_E}_orig_audio.m4a"
DEBUG_LOG_FILE = f"{SUFFIX_S_E}_LOG.txt"

COMMAND_AUDIO_ORIG_EXTRACT=["ffmpeg", "-i", INPUT_VIDEO_FILE, "-vn", "-acodec", "copy", OUTPUT_ORIG_AUDIO_FILE]
COMMENT_LOG_AUDIO_ORIG_EXTRACT = f"Save extracted original audio.  - {COMMAND_AUDIO_ORIG_EXTRACT} \n"

COMMAND_MAKE_INTER_VIDEO = [
        "ffmpeg", "-y", "-i", INPUT_VIDEO_FILE,
        "-vf", f"{VF_FILTERS}",
        "-pix_fmt", f"{INTER_FORMAT}", "-vsync", "passthrough",
        YUV_VIDEO_FILE
    ]
COMMENT_LOG_MAKE_INTER_VIDEO = f"Making intermediate video - {COMMAND_MAKE_INTER_VIDEO} \n"

COMMAND_MAKE_INTER_AUDIO = [
        "ffmpeg", "-y", "-i", INPUT_VIDEO_FILE, "-vn",
        "-af", A_FILTERS, "-c:a", "libopus", "-b:a", f"{A_BITS}k", "-application", "voip", INTER_AUDIO_FILE
    ]
COMMENT_LOG_MAKE_INTER_AUDIO = f"Making intermediate audio - {COMMAND_MAKE_INTER_AUDIO} \n"

def get_video_info(input_video):
    """ffprobe를 사용하여 비디오 해상도와 프레임 속도를 추출"""
    # 해상도 추출
    resolution = subprocess.check_output([
        'ffprobe', '-v', 'error', '-select_streams', 'v:0',
        '-show_entries', 'stream=width,height', '-of', 'default=noprint_wrappers=1:nokey=1', input_video
    ]).decode().strip().split('\n')
    width, height = resolution[0], resolution[1]

    # 프레임 속도 추출
    fps = subprocess.check_output([
        'ffprobe', '-v', 'error', '-select_streams', 'v:0',
        '-show_entries', 'stream=r_frame_rate', '-of', 'default=noprint_wrappers=1:nokey=1', input_video
    ]).decode().strip()

    return width, height, fps



# orig_width, orig_height, orig_fps = get_video_info(INPUT_VIDEO_FILE)
orig_width, orig_height, orig_fps = get_video_info(INPUT_VIDEO_FILE)
print(f"original width :{orig_width}")
print(f"original height :{orig_height}")
print(f"original fps :{orig_fps}")

FPS = orig_fps

COMMAND_SVTAV1_ENCODING_1 = []
COMMENT_LOG_SVTAV1_ENCODING_1 =""
COMMAND_SVTAV1_ENCODING_2 = []
COMMAND_SVTAV1_ENCODING_3 = []


# def convert_to_yuv(input_video, output_yuv):
#     """ffmpeg로 비디오를 YUV420 포맷으로 변환"""
#     subprocess.run([
#         'ffmpeg', '-i', input_video, '-pix_fmt', 'yuv420p', '-f', 'rawvideo', '-y', output_yuv
#     ])

# def encode_with_svtav1(input_yuv, width, height, fps, output_file):
#     """SvtAv1EncApp으로 YUV 파일을 AV1로 인코딩"""
#     subprocess.run([
#         'SvtAv1EncApp', '-i', input_yuv, '-w', width, '-h', height,
#         '-fps', fps, '-b', output_file
#     ])

def run_command(command, log_file, comment):
    """명령어 실행 및 출력 실시간 캡처, 로그 파일에 기록"""
    with open(log_file, 'a') as log:
        log.write(comment)
        log.write("//////////////////new command start////////////////////////")
        log.write("//////////////////new command start////////////////////////")
        process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        
        # 실시간 출력 및 로그 파일 기록
        for stdout_line in process.stdout:
            print(stdout_line.strip())  # 콘솔에 출력
            log.write(stdout_line)      # 로그 파일에 기록

        for stderr_line in process.stderr:
            print(stderr_line.strip(), file=sys.stderr)  # 콘솔에 출력
            log.write(stderr_line)  # 로그 파일에 기록
        
        log.write(comment)
        log.write("//////////////////new command end////////////////////////")
        log.write("//////////////////new command end////////////////////////")
        process.wait()

def main():
    # 1. YUV 및 오디오 추출
    run_command()



    # 2. 1st Pass
    run([
        "SvtAv1EncApp.exe",
        "--input", YUV_FILE,
        "--input-width", str(WIDTH),
        "--input-height", str(HEIGHT),
        "--fps", str(FRAMERATE),
        "--input-depth", str(INPUT_VIDEO_DEPTH),
        "--pass", "1",
        "--rc", "1",
        "--target-bitrate", str(BITRATE),
        "--output-stat-file", STATS_FILE
    ], "1-PASS 분석")

    # 3. 2nd Pass
    run([
        "SvtAv1EncApp.exe",
        "--input", YUV_FILE,
        "--input-width", str(WIDTH),
        "--input-height", str(HEIGHT),
        "--fps", str(FRAMERATE),
        "--input-depth", str(INPUT_VIDEO_DEPTH),
        "--pass", "2",
        "--rc", "1",
        "--target-bitrate", str(BITRATE),
        "--input-stat-file", STATS_FILE,
        "--output", VIDEO_FILE
    ], "2-PASS 인코딩")

    # 4. MKV 병합
    run([
        "ffmpeg", "-y",
        "-i", VIDEO_FILE,
        "-i", AUDIO_FILE,
        "-c:v", "copy",
        "-c:a", "copy",
        OUTPUT_FILE
    ], "비디오 + 오디오 병합")

    print("\n✅ 완료되었습니다:", OUTPUT_FILE)

    # (선택) 중간 파일 제거
    # for f in [YUV_FILE, AUDIO_FILE, STATS_FILE, VIDEO_FILE]:
    #     if os.path.exists(f):
    #         os.remove(f)

if __name__ == "__main__":
    main()