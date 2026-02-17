# 
# 将所有文件打包到虚拟机硬盘。要求硬盘文件存在且格式化为FAT32，盘符有序。
# @date 2025-12-03
# 
# Copyright (c) 2025-2026 Tony Chen Smith
# 
# SPDX-License-Identifier: MIT
# 
import argparse
import ctypes
import os
import json
import sign
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

    script=f'Mount-VHD -Path "{vhd_path}" -PassThru|Get-Disk|Get-Partition|Where-Object {{$_.DriveLetter}}|Select-Object PartitionNumber,Guid,DriveLetter|Sort-Object PartitionNumber|ConvertTo-Json -Compress'
    result=run_ps_command(script)
    
    if result and result.returncode==0:
        output=json.loads(result.stdout)
        result=[]
        index=1
        for partition in output:
            result.append({'partition':f'hd{index}/','drive':f'{partition['DriveLetter']}:/','guid':partition['Guid'][1:-1].upper()})
            index=index+1
        if len(result)>0:
            return result

    return None

def unmount_vhd(vhd_path):
    """
    卸载VHD文件。
    """
    
    script=f'Dismount-VHD -Path "{vhd_path}" -Confirm:$false'
    run_ps_command(script)

if __name__=="__main__":
    parser=argparse.ArgumentParser()
    groups=parser.add_mutually_exclusive_group()
    groups.add_argument('-d','-D','-debug','-Debug','-DEBUG',action='store_const',dest='mode',default='Debug',const='Debug')
    groups.add_argument('-r','-R','-release','-Release','-RELEASE',action='store_const',dest='mode',const='Release')
    args=parser.parse_args()

    run_as_admin()
    
    manifest_path='debug.manifest.json'
    if args.mode=='Release':
        sign.sign_kernel(False)
        manifest_path='release.manifest.json'
    else:
        sign.sign_kernel(True)

    project_base=os.path.normpath(os.path.abspath('../'))
    partitions=None
    manifest=None
    if os.path.exists(manifest_path):
        with open(manifest_path,'r') as jsonfile:
            manifest=json.load(jsonfile)
    else:
        print(f'文件{manifest_path}未找到。')
        input()
        sys.exit(1)

    vhd=os.path.normpath(os.path.join(project_base,manifest['disk']))
    if os.path.exists(vhd):
        unmount_vhd(vhd)
        partitions=mount_vhd(vhd)
    
    if partitions!=None:
        system_partiton=None
        for partition in partitions:
            if manifest['system']==partition['partition']:
                system_partiton=partition['guid']
                break
        if system_partiton!=None:
            data='HD,GPT,'+system_partiton
            location_path=os.path.normpath(os.path.join(project_base,'test/','aos.system.location'))
            with open(location_path,'w') as f:
                f.write(data)

        for info in manifest['manifest']:
            for partition in partitions:
                if info['disk'].startswith(partition['partition']):
                    dpath=os.path.normpath(partition['drive']+info['disk'][len(partition['partition']):])
                    os.makedirs(dpath,exist_ok=True)
                    dpath=os.path.join(dpath,info['file'])
                    ppath=os.path.normpath(os.path.join(project_base,info['project'],info['file']))
                    if os.path.exists(ppath):
                        shutil.copy2(ppath,dpath)
                    break
                else:
                    continue
        
        unmount_vhd(vhd)
    else:
        unmount_vhd(vhd)
    
    if args.mode=='Release':
        print('已复制发行版清单内的所有文件。')
    else:
        print('已复制调试版清单内的所有文件。')