# zynqmp_cam_isp_demo_linux
[zynqmp_cam_isp_demo](https://github.com/bxinquan/zynqmp_cam_isp_demo) linux软件项目
安装[xilinx-zynqmp-common-v2022.1](https://www.xilinx.com/member/forms/download/xef.html?filename=xilinx-zynqmp-common-v2022.1_04191534.tar.gz)/sdk.sh
待更新


![image](https://github.com/bxinquan/zynqmp_cam_isp_demo_linux/blob/main/Doc/Linux_SW_Diagram.png)

### u-boot 配置nfsroot, tftp下载kernel和dtb, 启动kernel
```
setenv bootargs 'earlycon console=ttyPS1,115200 clk_ignore_unused root=/dev/nfs rw nfsroot=192.168.1.10:/tftpboot/zynqmp/rootfs,nolock,nfsvers=3 ip=dhcp cma=1000M '
setenv ipaddr 192.168.137.123; setenv netmask 255.255.255.0; setenv gatewayip 192.168.137.1; setenv serverip 192.168.1.10
tftpboot 0x00200000 zynqmp/Image; tftpboot 0x00100000 zynqmp/system.dtb
booti 0x00200000 - 0x00100000
```

### 加载PL及其dtbo
```
mkdir -p /configfs
mount -t configfs configfs /configfs
echo 0 > /sys/class/fpga_manager/fpga0/flags
mkdir /configfs/device-tree/overlays/full
echo -n "pl.dtbo" > /configfs/device-tree/overlays/full/path
```

### 配置AR1335,MIPI,ISP,VIP
```
media-ctl -d /dev/media0 --set-v4l2 '"ar1335 3-0036":0[fmt:SGRBG10_1X10/2048x1536 field:none]'
media-ctl -d /dev/media0 --set-v4l2 '"a0020000.mipi_rx_to_video":0[fmt:SGRBG10_1X10/2048x1536 field:none]'
media-ctl -d /dev/media0 --set-v4l2 '"a0020000.mipi_rx_to_video":1[fmt:SGRBG10_1X10/2048x1536 field:none]'
media-ctl -d /dev/media0 --set-v4l2 '"axi:camif_ias1_axis_subsetconv":0[fmt:SGRBG10_1X10/2048x1536 field:none]'
media-ctl -d /dev/media0 --set-v4l2 '"axi:camif_ias1_axis_subsetconv":1[fmt:Y10_1X10/2048x1536]'
media-ctl -d /dev/media1 --set-v4l2 '"a0050000.xil_isp_lite":0[fmt:Y10_1X10/2048x1536 field:none]'
media-ctl -d /dev/media1 --set-v4l2 '"a0060000.xil_vip":1[fmt:UYVY8_1X16/1920x1080 field:none]'
media-ctl -d /dev/media1 --set-v4l2 '"a0070000.xil_vip":1[fmt:UYVY8_1X16/1920x1080 field:none]'
v4l2-ctl -d /dev/video1 -c low_latency_controls=2
v4l2-ctl -d /dev/video2 -c low_latency_controls=2
v4l2-ctl -d /dev/video3 -c low_latency_controls=2
```

### MIPI-RX V4L2测试
```
v4l2-ctl -d /dev/video0 -c exposure=3000,analogue_gain=0x2020
v4l2-ctl -d /dev/video0 -C exposure,analogue_gain
v4l2-ctl -d /dev/video0 --set-fmt-video=width=2048,height=1536,pixelformat=XY10,bytesperline=3840 --stream-mmap=3 --stream-skip=30 --stream-count=60 --stream-poll --stream-to=camera.raw10
```

### ISP-PIPE V4L2测试
```
v4l2-ctl -d /dev/video4 --set-fmt-meta XISP --stream-mmap=3 --stream-poll --stream-loop --stream-to=camera.stat &
v4l2-ctl -d /dev/video1 --set-fmt-video=width=1920,height=1080,pixelformat=YUYV --stream-mmap=3 --stream-poll --stream-to=camera1.yuyv &
v4l2-ctl -d /dev/video2 --set-fmt-video=width=1920,height=1080,pixelformat=YUYV --stream-mmap=3 --stream-poll --stream-to=camera2.yuyv &
v4l2-ctl -d /dev/video3 --set-fmt-video-out=width=2048,height=1536,pixelformat=XY10,bytesperline=3840 --stream-out-mmap=3 --stream-poll --stream-from=camera.raw10
```

### libcamera-cam测试
```
cam -c 1 -s width=1920,height=1080,pixelformat=YUYV,role=video,colorspace=rec709 --capture=10 --file
```

### libcamera-apps测试
```
modetest -D fd4a0000.display -w 40:alpha:0 #Primary Plane全透明,让Overlay Plane显示
libcamera-hello -t 0
libcamera-hello -t 0 --ev -1.0
libcamera-hello -t 0 --shutter 33333.0 --analoggain 4.0 --awbgains 1.0,1.0 --denoise off --sharpness 0
libcamera-still --mode 2048:1536:10:P --width 1920 --height 1080 -r -o image.jpg
libcamera-still --mode 2048:1536:10:P --width 1920 --height 1080 -r -e yuv420 -o image.yuv --shutter 33333.0 --analoggain 4.0
```

### gst-lanuch预览
```
media-ctl -d /dev/media1 --set-v4l2 '"a0060000.xil_vip":1[fmt:UYVY8_1X16/1920x1080 field:none]'
media-ctl -d /dev/media1 --set-v4l2 '"a0070000.xil_vip":1[fmt:UYVY8_1X16/1920x1080 field:none]'
gst-launch-1.0 libcamerasrc camera-name="/base/axi/i2c@a0000000/camera@36" ! video/x-raw,format=YUY2,width=1920,height=1080,framerate=30/1 ! kmssink bus-id=fd4a0000.display fullscreen-overlay=1 async=false
```
### gst-lanuch录制
```
media-ctl -d /dev/media1 --set-v4l2 '"a0060000.xil_vip":1[fmt:VYYUYY8_1X24/2048x1536 field:none]'
media-ctl -d /dev/media1 --set-v4l2 '"a0070000.xil_vip":1[fmt:VYYUYY8_1X24/2048x1536 field:none]'
gst-launch-1.0 libcamerasrc camera-name="/base/axi/i2c@a0000000/camera@36" ! video/x-raw,format=NV12,width=2048,height=1536,framerate=30/1 ! omxh264enc target-bitrate=4000 ! h264parse config-interval=-1 ! mpegtsmux ! filesink location=test.mp4
```
### gst-lanuch预览+录制
```
media-ctl -d /dev/media1 --set-v4l2 '"a0060000.xil_vip":1[fmt:UYVY8_1X16/1920x1080 field:none]'
media-ctl -d /dev/media1 --set-v4l2 '"a0070000.xil_vip":1[fmt:VYYUYY8_1X24/1920x1080 field:none]'
gst-launch-1.0 libcamerasrc camera-name="/base/axi/i2c@a0000000/camera@36" name=camera camera.src ! video/x-raw,format=YUY2,width=1920,height=1080,framerate=30/1 ! kmssink bus-id=fd4a0000.display fullscreen-overlay=1 async=false camera.src_0 ! video/x-raw,format=NV12,width=1920,height=1080,framerate=30/1 ! omxh264enc target-bitrate=4000 ! h264parse config-interval=-1 ! mpegtsmux ! filesink location=test.mp4
```
```
media-ctl -d /dev/media1 --set-v4l2 '"a0060000.xil_vip":1[fmt:UYVY8_1X16/1024x768 field:none]'
media-ctl -d /dev/media1 --set-v4l2 '"a0070000.xil_vip":1[fmt:VYYUYY8_1X24/1920x1080 field:none]'
gst-launch-1.0 libcamerasrc camera-name="/base/axi/i2c@a0000000/camera@36" name=camera camera.src ! video/x-raw,format=YUY2,width=1024,height=768,framerate=30/1 ! kmssink bus-id=fd4a0000.display fullscreen-overlay=1 async=false camera.src_0 ! video/x-raw,format=NV12,width=1920,height=1080,framerate=30/1 ! omxh264enc target-bitrate=4000 ! h264parse config-interval=-1 ! mpegtsmux ! filesink location=test.mp4
```

![image](https://github.com/bxinquan/zynqmp_cam_isp_demo_linux/blob/main/Doc/cmd.png)

### 调试信息
```
media-ctl --device /dev/media0 --print-topology
media-ctl --device /dev/media1 --print-topology
```
```
echo 0xf > /sys/class/video4linux/video0/dev_debug
echo 0xf > /sys/class/video4linux/video1/dev_debug
echo 0xf > /sys/class/video4linux/video2/dev_debug
echo 0xf > /sys/class/video4linux/video3/dev_debug
echo 0xf > /sys/class/video4linux/video4/dev_debug
echo 0xf > /sys/class/video4linux/v4l-subdev0/dev_debug
echo 0xf > /sys/class/video4linux/v4l-subdev1/dev_debug
echo 0xf > /sys/class/video4linux/v4l-subdev2/dev_debug
echo 0xf > /sys/class/video4linux/v4l-subdev3/dev_debug
echo 0xf > /sys/class/video4linux/v4l-subdev4/dev_debug
echo 0xf > /sys/class/video4linux/v4l-subdev5/dev_debug
echo 1 > /sys/module/videobuf2_v4l2/parameters/debug
```
```
export LIBCAMERA_IPA_MODULE_PATH=/usr/lib/libcamera
export LIBCAMERA_IPA_PROXY_PATH=/usr/libexec/libcamera
export LIBCAMERA_IPA_CONFIG_PATH=/usr/share/libcamera/ipa
export LIBCAMERA_LOG_LEVELS=*:DEBUG
```
```
gst-launch-1.0 -v videotestsrc ! video/x-raw,format=YUY2,width=1920,height=1080,framerate=30/1 ! kmssink bus-id=fd4a0000.display fullscreen-overlay=1 sync=false async=false
gst-launch-1.0 -v videotestsrc ! video/x-raw,format=NV12,width=1920,height=1080,framerate=30/1 ! kmssink bus-id=fd4a0000.display fullscreen-overlay=1 sync=false async=false
gst-launch-1.0 -v videotestsrc ! video/x-raw,format=RGB,width=1920,height=1080,framerate=30/1 ! kmssink bus-id=fd4a0000.display fullscreen-overlay=1 sync=false async=false
```
