# zynqmp_cam_isp_demo_linux
zynqmp_cam_isp_demo(https://github.com/bxinquan/zynqmp_cam_isp_demo) linux软件项目
待更新


![image](https://github.com/bxinquan/zynqmp_cam_isp_demo_linux/blob/main/Doc/Linux_SW_Diagram.png)

```
#u-boot 配置nfsroot, tftp下载kernel和dtb, 启动kernel
setenv bootargs 'earlycon console=ttyPS1,115200 clk_ignore_unused root=/dev/nfs rw nfsroot=192.168.1.10:/tftpboot/zynqmp/rootfs,nolock,nfsvers=3 ip=dhcp cma=1000M '
setenv ipaddr 192.168.137.123; setenv netmask 255.255.255.0; setenv gatewayip 192.168.137.1; setenv serverip 192.168.1.10
tftpboot 0x00200000 zynqmp/Image; tftpboot 0x00100000 zynqmp/system.dtb
booti 0x00200000 - 0x00100000
```

```
#加载PL及其dtbo
mkdir -p /configfs
mount -t configfs configfs /configfs
echo 0 > /sys/class/fpga_manager/fpga0/flags
mkdir /configfs/device-tree/overlays/full
echo -n "pl.dtbo" > /configfs/device-tree/overlays/full/path
```

```
#配置AR1335,MIPI-RX格式
media-ctl --set-v4l2 '"ar1335 3-0036":0[fmt:SGRBG10_1X10/2048x1536 field:none]'
media-ctl --set-v4l2 '"a0030000.mipi_rx_to_video":0[fmt:SGRBG10_1X10/2048x1536 field:none]'
media-ctl --set-v4l2 '"a0030000.mipi_rx_to_video":1[fmt:SGRBG10_1X10/2048x1536 field:none]'
media-ctl --set-v4l2 '"axi:camif_ias1_axis_subsetconv":0[fmt:SGRBG10_1X10/2048x1536 field:none]'
media-ctl --set-v4l2 '"axi:camif_ias1_axis_subsetconv":1[fmt:Y10_1X10/2048x1536]'
```

```
#配置ISP,VIP格式
media-ctl -d /dev/media1 --set-v4l2 '"a0070000.xil_isp_lite":0[fmt:Y10_1X10/2048x1536 field:none]'
media-ctl -d /dev/media1 --set-v4l2 '"a00a0000.xil_vip":1[fmt:UYVY8_1X16/1920x1080 field:none]'
v4l2-ctl -d /dev/video2 -c low_latency_controls=2
```

```
#配置libcamera运行时环境
export LIBCAMERA_IPA_MODULE_PATH=/usr/lib/libcamera
export LIBCAMERA_IPA_PROXY_PATH=/usr/libexec/libcamera
export LIBCAMERA_IPA_CONFIG_PATH=/usr/share/libcamera/ipa
```

```
#libcamera-cam测试
cam -c 1 -s width=1920,height=1080,pixelformat=YUYV,role=video,colorspace=rec709 --capture=10 --file
```

```
#gst-lanuch显示测试
gst-launch-1.0 libcamerasrc camera-name="/base/axi/i2c@a0010000/camera@36" ! video/x-raw,format=YUY2,width=1920,height=1080,framerate=30/1 ! kmssink bus-id=fd4a0000.display fullscreen-overlay=1 async=false
```

![image](https://github.com/bxinquan/zynqmp_cam_isp_demo_linux/blob/main/Doc/cmd.png)
