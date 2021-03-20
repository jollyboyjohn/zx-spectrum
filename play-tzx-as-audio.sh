#!/bin/sh
#
# Play a TZX file to audio out
# 
# Features:
# - No intermediate storage used during processing
# - Uses inverted stereo which works very well
# - Tested with speedloaders (Silk Worm)
# 
# Tested with a ZX Spectrum +128k and +3
# 
# Requirements:
# - A line-level output: 	A DAC works best (PhatDAC on RPI 2)
# - Stereo lead:		Plugged into EAR / TAPE-IN
# - fuse-emulator-utils:	Package for interpreting TZX files
# - gstreamer1.0-tools:		Command-line audio stream manipulation
# - gstreamer1.0-alsa:		ALSA audio output via alsasink
# - gstreamer1.0-plugins-good:	audioinvert/capssetter/deinterleave/interleave
#
# TODO:
# - gstreamers fdsrc truncates a few ms at the end of WAVs - having to use RAW
# - Noise at start & end with RAW
# - Better verificationBetter verificationBetter verification
#
# Credit to mcleod_ideafix + scruss for inversion idea:
#     https://retrocomputing.stackexchange.com/a/774/439

# Check the TZX for a CRC pass
if [ $(tzxlist $1 | grep -i -c pass) -eq 0 ]; then
    echo "$1 is not a TZX file"
    exit
fi

# MAIN PROCESS
#
# Convert the TZX to a inverted stereo WAV:
# - 1. Use tape2wav to convert TZX to WAV on stdout
# - 2. Ingest the WAV from stdin, upmixing to stereo
# - 3. Deinterleave to seperate channels (d)
# 
# - 4a. Ignore L channel (d.src_0)
# - 4b. Invert R channel (d.src_1)
# 
# - 5. Interleave L+R channels (i.sink_0 + i.sink_1)
# - 6. Set capability to two channels (2 bits = 0x03)
# - 7. Output to ALSA

tape2wav $1 - | \
    gst-launch-1.0 -v \
    fdsrc fd=0 \
    	! queue \
        ! rawaudioparse num-channels=1 pcm-format=u8 \
	! audioconvert \
	! audio/x-raw,channels=2 \
	! volume volume=0.99 \
	! deinterleave name=d \
    interleave name=i \
	! capssetter caps='audio/x-raw,channel-mask=(bitmask)0x03' \
	! alsasink \
    d.src_0 \
	! queue \
	! audioconvert \
	! i.sink_0 \
    d.src_1 \
	! queue \
	! audioconvert \
	! audioinvert degree=1 \
	! i.sink_1
