#!/bin/bash

#添加本地root用户到xhost权限
xhost +local:root

#添加SI:localuser:root权限
xhost +SI:localuser:root

#设置QT_QPA_PLATFORM环境变量
QT_QPA_PLATFORM=xcb

echo "set complete."