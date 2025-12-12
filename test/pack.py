# 
# 将所有文件打包到虚拟机硬盘。要求硬盘文件存在且格式化为FAT32，盘符唯一。
# @date 2025-12-03
# 
# Copyright (c) 2025 Tony Chen Smith
# 
# SPDX-License-Identifier: MIT
# 
import ctypes
import os
import json
import shutil
import subprocess
import sys

def run_as_admin():
    """
    请求管理员权限并重启窗口。
    """
    if ctypes.windll.shell32.IsUserAnAdmin():
        return True
    
    # 重新以管理员身份运行
    ctypes.windll.shell32.ShellExecuteW(None,"runas", sys.executable," ".join(sys.argv),None,1)
    sys.exit(0)

def run_ps_command(script):
    """
    运行PowerShell命令。
    """
    try:
        result=subprocess.run(["powershell.exe","-Command",script],capture_output=True,text=True,shell=True)
        return result
    except Exception as e:
        print(f"错误: {e}")
        return None

def mount_vhd(vhd_path):
    """
    挂载VHD文件并返回第一个盘符。
    """

    script=f'Mount-VHD -Path "{vhd_path}" -PassThru | Get-Disk | Get-Partition | Get-Volume | ForEach-Object {{ $_.DriveLetter }} | Where-Object {{ $_ -ne $null }}'
    result=run_ps_command(script)
    
    if result and result.returncode==0:
        if result.stdout!=None:
            disk=sorted(result.stdout.strip().split("\n"))
            if len(disk)>0:
                return disk[0]
    return None

def unmount_vhd(vhd_path):
    """
    卸载VHD文件。
    """
    
    script=f'Dismount-VHD -Path "{vhd_path}" -Confirm:$false'
    run_ps_command(script)

if __name__=="__main__":
    run_as_admin()

    project_base=os.path.normpath(os.path.abspath('../'))
    disk_base=None
    manifest=None
    if os.path.exists('manifest.json'):
        with open('manifest.json','r') as jsonfile:
            manifest=json.load(jsonfile)

    vhd=os.path.normpath(os.path.join(project_base,manifest['disk']))
    if os.path.exists(vhd):
        unmount_vhd(vhd)
        disk_base=mount_vhd(vhd)
    
    if disk_base!=None:
        disk_base=disk_base+':/'
        disk_base=os.path.normpath(disk_base)

        for info in manifest['manifest']:
            dpath=os.path.normpath(os.path.join(disk_base,info['disk']))
            os.makedirs(dpath,exist_ok=True)
            dpath=os.path.join(dpath,info['file'])
            ppath=os.path.normpath(os.path.join(project_base,info['project'],info['file']))
            if os.path.exists(ppath):
                shutil.copy2(ppath,dpath)

        unmount_vhd(vhd)