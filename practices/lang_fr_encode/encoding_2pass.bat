::the 2-pass not working properly on the input of ffmpeg
::scen change detection -scd and -lookahead 120 also should be try
@echo off
set SEASON=02
set EPNUM=8
set "INPUT=Episode %EPNUM%.mp4"
set PADDED=00%EPNUM%
set PADDEDEPNUM=%PADDED:~-2%
set BITRATE=100k
set BUFSIZE=300k
set AUDIOBR=64k
set PRESET1=4
set PRESET2=2
set GOP=240
set FILTERS=hqdn3d=4:3:6:6,scale=640:-2
set AFILTERS=highpass=f=100,lowpass=f=6000,acompressor,aresample=16000
set MINQP=15
set MAXQP=63
@REM set OUTPUT=s%SEASON%_e%PADDEDEPNUM%_2pass_p%PRESET1%%PRESET2%_b%BITRATE%_bs%BUFSIZE%_s640_4366f_g%GOP%_sc40_opus_a%AUDIOBR%_f100-6000_ac_ar16000.mkv
@REM it seems without cs would be better? or it need to require more research
:: In AV1, declare CRB may apply bufsize correctly, otherwise bufsize may not working.

:: -rc 2 <- ffmpeg for VBR? but in the AV1-libsvtav1 -> rc 1 == VBR, rc 2 == CBR
set SURFIXSEASONEP=s%SEASON%_e%PADDEDEPNUM%
set OUTPUT=%SURFIXSEASONEP%_no2pass_p%PRESET2%_b%BITRATE%_s640_4366f_g%GOP%_t0fg2rc1minqp%MINQP%maxqp%MAXQP%aq2_opus_a%AUDIOBR%_f100-6000_ac_ar16000.mkv

set LOGPASS1 = %SURFIXSEASONEP%
@REM set LOGPASS1 = 1_pass1_debug.txt
set LOGPASS2 = %SURFIXSEASONEP%
@REM set LOGPASS2 = 2_pass2_debug.txt

@REM echo "[PASS 1] analyzing..."
::ffmpeg -i "%INPUT%" -vf "%FILTERS%" -c:v libsvtav1 -b:v %BITRATE% -bufsize %BUFSIZE% -sc_threshold 40 -pass 1 -preset %PRESET1% -g %GOP% -aq-mode 2 -an -f null NUL
@REM ffmpeg -loglevel debug -i "%INPUT%" -vf "%FILTERS%" -c:v libsvtav1 -b:v %BITRATE% -bufsize %BUFSIZE% -rc 2 -pass 1 -preset %PRESET1% -g %GOP% -aq-mode 2 -svtav1-params tune=0:film-grain=2 -an -f null NUL > "%LOGPASS1%" 2>&1
@REM ffmpeg -loglevel info -i "%INPUT%" -vf "%FILTERS%" -c:v libsvtav1 -b:v %BITRATE% -bufsize %BUFSIZE% -pass 1 -preset %PRESET1% -g %GOP% -aq-mode 2 -svtav1-params tune=0:film-grain=2:rc=2 -an -f null NUL > log1.txt 2>&1
@REM ffmpeg -loglevel info -i "%INPUT%" -vf "%FILTERS%" -c:v libsvtav1 -b:v %BITRATE% -bufsize %BUFSIZE% -passlogfile %SURFIXSEASONEP%_pass_log -preset %PRESET1% -g %GOP% -svtav1-params "tune=0:film-grain=2:rc=1:min-qp=%MINQP%:max-qp=%MAXQP%:aq-mode=2:passes=2:pass=1:stats=%SURFIXSEASONEP%_svtav1_pass1" -an -f null NUL > _pass1_info_debug.txt 2>&1
@REM ffmpeg -loglevel info -i "%INPUT%" -vf "%FILTERS%" -c:v libsvtav1 -b:v %BITRATE% -bufsize %BUFSIZE% -preset %PRESET1% -g %GOP% -svtav1-params "tune=0:film-grain=2:rc=1:min-qp=%MINQP%:max-qp=%MAXQP%:aq-mode=2:pass=1" -an -f null NUL > _pass1_info_debug.txt 2>&1

echo "[EP%PADDEDEPNUM% with no PASS 2] incoding..."
::ffmpeg -i "%INPUT%" -vf "%FILTERS%" -c:v libsvtav1 -b:v %BITRATE% -bufsize %BUFSIZE% -sc_threshold 40 -pass 2 -preset %PRESET2% -g %GOP% -aq-mode 2 -af %AFILTERS% -c:a libopus -b:a %AUDIOBR% -application voip %OUTPUT%
@REM ffmpeg -loglevel debug -i "%INPUT%" -vf "%FILTERS%" -c:v libsvtav1 -b:v %BITRATE% -bufsize %BUFSIZE% -rc 2 -pass 2 -preset %PRESET2% -g %GOP% -aq-mode 2 -svtav1-params tune=0:film-grain=2 -af %AFILTERS% -c:a libopus -b:a %AUDIOBR% -application voip "%OUTPUT%" > "%LOGPASS2%" 2>&1
@REM ffmpeg -loglevel info -i "%INPUT%" -vf "%FILTERS%" -c:v libsvtav1 -b:v %BITRATE% -bufsize %BUFSIZE% -rc 2 -pass 2 -preset %PRESET2% -g %GOP% -aq-mode 2 -svtav1-params tune=0:film-grain=2 -af %AFILTERS% -c:a libopus -b:a %AUDIOBR% -application voip "%OUTPUT%" > log2.txt 2>&1
ffmpeg -loglevel info -i "%INPUT%" -vf "%FILTERS%" -c:v libsvtav1 -b:v %BITRATE% -bufsize %BUFSIZE%  -preset %PRESET2% -g %GOP% -svtav1-params "tune=0:film-grain=2:rc=1:min-qp=%MINQP%:max-qp=%MAXQP%:aq-mode=2" -af %AFILTERS% -c:a libopus -b:a %AUDIOBR% -application voip "%OUTPUT%" > 0_no_pass2_info_debug.txt 2>&1

echo "encoding done."
@REM pause