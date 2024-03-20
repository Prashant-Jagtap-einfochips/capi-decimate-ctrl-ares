This example will work with following installations
1. Install Qualcomm Package Manager 3
2. Install Hexagon sdk Version 5.4.0.3
3. Install  Audio1x Version 2.0.0.0


Build Steps:
Build decimate module:
1. cd /capi-ares/capi-decimate-ctrl-ares/capi-decimate-ctrl
2. build_cmake hexagonsim DSP_ARCH=v68 VERBOSE=0


Build decimate control module:
1. cd /capi-ares/capi-decimate-ctrl-ares/capi-ctrl
2. build_cmake hexagonsim DSP_ARCH=v68 VERBOSE=0


Signing process:
1. python <SDK 5.4.0.3 PATH>/utils/scripts/signer.py sign -t <Device Serial No.> -d adsp -i libcapi_decimate.so
2. python <SDK 5.4.0.3 PATH>/utils/scripts/signer.py sign -t <Device Serial No.> -d adsp -i libcontrol_tx.so


Steps to be followed on device for testing of modules
1. reset
2. Connnect to the COM port of device and enter following commands
	- mq
	- audio_service -f /ifs/etc/lpass_cfg
	- mount -uw /mnt
	
3. Push acdb files at /mnt/etc/acdb/adp_8295 
4. Push .so files of control and data module at /mnt/etc/images/dsp
5. Reset device using "reset" command
6. Start Playback - audio_chime -g 21 <wav file path>
7. Connect to device using QACT 8.0.33 
8. Enable data module through QACT tool
9. Change other parameters of data and control module to check that data and control module are working properly
10. If the the QXDM(v5.2.510) dumps are taken then use QCAT v7.01.192 
