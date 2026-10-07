# 
# 负责对内核文件进行签名与总签名。
# @date 2025-12-16
# 
# Copyright (c) 2025-2026 Tony Chen Smith
# 
# SPDX-License-Identifier: MIT
# 
import argparse
import datetime
import hashlib
import json
import os
import re
import shutil
from cryptography import x509
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.asymmetric import ec,utils
from cryptography.x509.oid import NameOID

# 版本宏提取。
# 宏定义里<MODULE>代表模块名，<MAJOR>代表主版本号，<MINOR>代表次版本号，<PATCH>代表修订版本号。
# Y、M、D均是BCD数，Y代表年份，M代表月份，D代表日期。
# 版本头文件内版本宏格式固定如下：
#   #define AOS_<MODULE>_VERSION_<MAJOR>_<MINOR>_<PATCH> (0xYYYYMMDD|0xY{4,12}MMDDULL)
#   #define AOS_<MODULE>_VERSION AOS_<MODULE>_VERSION_<MAJOR>_<MINOR>_<PATCH>
_VERSION_DEFINE_RE=re.compile(r"^#define\s+(\w+)\s+(\S+)",re.M)
_VERSION_VALUE_RE=re.compile(r"^(?:0x[0-9]{8}|0x[0-9]{8,16}ULL)$")

def read_version_macros(path):
    """
    读取版本头文件，返回{宏名:原始值}。
    """
    with open(path,"r",encoding="utf-8") as f:
        return dict(_VERSION_DEFINE_RE.findall(f.read()))

def parse_version_value(raw):
    """
    版本字面量转换成BCD数值。格式不符即报错。
    """
    raw=raw.strip()
    if not _VERSION_VALUE_RE.match(raw):
        raise ValueError(f"版本值格式不符: {raw}")
    return int(raw[:-3] if raw.endswith("ULL") else raw,16)

def bcd_to_int(byte):
    """
    BCD字节转十进制。
    """
    return (byte>>4)*10+(byte&0xF)

def check_version_value(value):
    """
    版本数值正常性检查。
    """
    digits=f"{value:X}"
    if any(c>"9"for c in digits):
        raise ValueError(f"版本值含非BCD数字: 0x{digits}")

    year_bcd=value>>16
    year=int(f"{year_bcd:X}") if year_bcd else 0
    month=bcd_to_int((value>>8)&0xFF)
    day=bcd_to_int(value&0xFF)

    if not (datetime.MINYEAR<=year<= datetime.MAXYEAR):
        raise ValueError(f"版本值年份超出支持范围({datetime.MINYEAR}~{datetime.MAXYEAR}): {year}")
    try:
        datetime.date(year,month,day)
    except ValueError as e:
        raise ValueError(f"版本值日期非法: {year}-{month:02d}-{day:02d}") from e

def read_version_value(macros,name):
    """
    按宏名读取版本数值。
    解析并检查后返回BCD数值。
    """
    if name not in macros:
        raise KeyError(f"未找到版本宏: {name}")
    raw=macros[name]
    for _ in range(8):
        if raw.startswith("0x"):
            value=parse_version_value(raw)
            check_version_value(value)
            return value
        if raw not in macros:
            raise KeyError(f"版本宏引用未定义: {name}->{raw}")
        raw=macros[raw]
    raise ValueError(f"版本宏引用链异常: {name}")

def read_version_field(macros,name):
    """
    按宏名读取对应版本值。
    """
    return read_version_value(macros,name).to_bytes(8,"little")

def find_manifest_file(debug,file_name):
    """
    按调试状态读取清单，返回清单内指定文件的绝对路径。
    """
    manifest_name="debug.manifest.json" if debug else "release.manifest.json"
    with open(manifest_name,"r",encoding="utf-8") as f:
        manifest=json.load(f)
    project_base=os.path.normpath(os.path.abspath("../"))
    for info in manifest["manifest"]:
        if info["file"]==file_name:
            return os.path.join(project_base,info["project"],info["file"])
    raise KeyError(f"未在清单 {manifest_name} 中找到文件 {file_name}。")

def generate_keys(debug=True):
    """
    根据调试状态生成新的密钥。正本存入当日备份目录，并复制一份到sign/keys/下。
    """
    project_base=os.path.normpath(os.path.abspath("../"))
    macros=read_version_macros(os.path.join(project_base,"include","version.h"))
    uefi_version=read_version_value(macros,"AOS_UEFI_VERSION")
    kernel_version=read_version_value(macros,"AOS_KERNEL_VERSION")
    time_now=datetime.datetime.now().astimezone()
    record_time=time_now.strftime("%Y%m%d-%H%M%S")

    if debug:
        uefi_record_dir="sign/keys/Debug/uefi/"
        kernel_record_dir="sign/keys/Debug/kernel/"
        uefi_root_public_key="uefi.debug.public.der"
        uefi_root_private_key="uefi.debug.private.pem"
        kernel_root_public_key="kernel.debug.public.der"
        kernel_root_private_key="kernel.debug.private.pem"

        uefi_subject_name=x509.Name([x509.NameAttribute(NameOID.COMMON_NAME,"Debug Module aos.uefi")])
        kernel_subject_name=x509.Name([x509.NameAttribute(NameOID.COMMON_NAME,"Debug Module aos.kernel")])
    else:
        uefi_record_dir="sign/keys/Release/uefi/"
        kernel_record_dir="sign/keys/Release/kernel/"
        uefi_root_public_key="uefi.release.public.der"
        uefi_root_private_key="uefi.release.private.pem"
        kernel_root_public_key="kernel.release.public.der"
        kernel_root_private_key="kernel.release.private.pem"

        uefi_subject_name=x509.Name([x509.NameAttribute(NameOID.COMMON_NAME,"Release Module aos.uefi")])
        kernel_subject_name=x509.Name([x509.NameAttribute(NameOID.COMMON_NAME,"Release Module aos.kernel")])
    
    os.makedirs(uefi_record_dir,exist_ok=True)
    os.makedirs(kernel_record_dir,exist_ok=True)

    curve=ec.SECP384R1()
    uefi_private=ec.generate_private_key(curve)
    kernel_private=ec.generate_private_key(curve)

    uefi_private_data=uefi_private.private_bytes(encoding=serialization.Encoding.PEM,format=serialization.PrivateFormat.PKCS8,encryption_algorithm=serialization.NoEncryption())
    kernel_private_data=kernel_private.private_bytes(encoding=serialization.Encoding.PEM,format=serialization.PrivateFormat.PKCS8,encryption_algorithm=serialization.NoEncryption())

    uefi_public_data=(x509.CertificateBuilder()
        .subject_name(uefi_subject_name)
        .issuer_name(uefi_subject_name)
        .public_key(uefi_private.public_key())
        .serial_number(x509.random_serial_number())
        .not_valid_before(time_now)
        .not_valid_after(time_now+datetime.timedelta(days=3650))
        .sign(uefi_private,hashes.SHA512())
        .public_bytes(serialization.Encoding.DER))
    
    kernel_public_data=(x509.CertificateBuilder()
        .subject_name(kernel_subject_name)
        .issuer_name(kernel_subject_name)
        .public_key(kernel_private.public_key())
        .serial_number(x509.random_serial_number())
        .not_valid_before(time_now)
        .not_valid_after(time_now+datetime.timedelta(days=3650))
        .sign(kernel_private,hashes.SHA512())
        .public_bytes(serialization.Encoding.DER))

    uefi_public_key=f"uefi.public.ecdsa.V{uefi_version:08X}.{record_time}.der"
    uefi_private_key=f"uefi.private.ecdsa.V{uefi_version:08X}.{record_time}.pem"
    kernel_public_key=f"kernel.public.ecdsa.V{kernel_version:08X}.{record_time}.der"
    kernel_private_key=f"kernel.private.ecdsa.V{kernel_version:08X}.{record_time}.pem"
    with open(os.path.join(uefi_record_dir,uefi_public_key),"wb") as f:
        f.write(uefi_public_data)
    with open(os.path.join(uefi_record_dir,uefi_private_key),"wb") as f:
        f.write(uefi_private_data)
    with open(os.path.join(kernel_record_dir,kernel_public_key),"wb") as f:
        f.write(kernel_public_data)
    with open(os.path.join(kernel_record_dir,kernel_private_key),"wb") as f:
        f.write(kernel_private_data)
    
    shutil.copy2(os.path.join(uefi_record_dir,uefi_public_key),os.path.join("sign/keys/",uefi_root_public_key))
    shutil.copy2(os.path.join(uefi_record_dir,uefi_private_key),os.path.join("sign/keys/",uefi_root_private_key))
    shutil.copy2(os.path.join(kernel_record_dir,kernel_public_key),os.path.join("sign/keys/",kernel_root_public_key))
    shutil.copy2(os.path.join(kernel_record_dir,kernel_private_key),os.path.join("sign/keys/",kernel_root_private_key))

def sign_aos(debug=True):
    """
    准备签名文件。
    """
    project_base=os.path.normpath(os.path.abspath("../"))
    macros=read_version_macros(os.path.join(project_base,"include","version.h"))
    uefi_version=read_version_field(macros,"AOS_UEFI_VERSION")
    kernel_version=read_version_field(macros,"AOS_KERNEL_VERSION")

    kernel_path=find_manifest_file(debug,"aos.kernel")
    if not os.path.exists(kernel_path):
        raise KeyError(f"文件 {kernel_path} 不存在。")
    
    sha256=hashlib.sha256()
    sha384=hashlib.sha384()
    sha512=hashlib.sha512()
    with open(kernel_path,"rb") as f:
        while chunk:=f.read(4096):
            sha256.update(chunk)
            sha384.update(chunk)
            sha512.update(chunk)
    kernel_sha256=sha256.digest()
    kernel_sha384=sha384.digest()
    kernel_sha512=sha512.digest()

    if debug:
        signature_dir="out/Debug"
        with open("sign/keys/kernel.debug.private.pem","rb") as f:
            kernel_key_pem=f.read()
        with open("sign/keys/uefi.debug.private.pem","rb") as f:
            uefi_key_pem=f.read()
    else:
        signature_dir="out/Release"
        with open("sign/keys/kernel.release.private.pem","rb") as f:
            kernel_key_pem=f.read()
        with open("sign/keys/uefi.release.private.pem","rb") as f:
            uefi_key_pem=f.read()
    kernel_key=serialization.load_pem_private_key(kernel_key_pem,password=None,backend=default_backend())
    uefi_key=serialization.load_pem_private_key(uefi_key_pem,password=None,backend=default_backend())
    os.makedirs(signature_dir,exist_ok=True)

    sha512=hashlib.sha512()
    sha512.update(kernel_version)
    sha512.update(kernel_sha256)
    sha512.update(kernel_sha384)
    sha512.update(kernel_sha512)
    with open(kernel_path,"rb") as f:
        while chunk:=f.read(4096):
            sha512.update(chunk)
    kernel_group_sha512=sha512.digest()

    kernel_ecdsa_der=kernel_key.sign(kernel_group_sha512,ec.ECDSA(utils.Prehashed(hashes.SHA512())))
    kernel_r,kernel_s=utils.decode_dss_signature(kernel_ecdsa_der)
    kernel_ecdsa=kernel_r.to_bytes(48,"big")+kernel_s.to_bytes(48,"big")
    kernel_signature=kernel_version+kernel_sha256+kernel_sha384+kernel_sha512+kernel_ecdsa

    sha256=hashlib.sha256()
    sha384=hashlib.sha384()
    sha512=hashlib.sha512()
    sha256.update(kernel_signature)
    sha384.update(kernel_signature)
    sha512.update(kernel_signature)
    uefi_sha256=sha256.digest()
    uefi_sha384=sha384.digest()
    uefi_sha512=sha512.digest()

    sha512=hashlib.sha512()
    sha512.update(uefi_version)
    sha512.update(uefi_sha256)
    sha512.update(uefi_sha384)
    sha512.update(uefi_sha512)
    sha512.update(kernel_signature)
    uefi_group_sha512=sha512.digest()

    uefi_ecdsa_der=uefi_key.sign(uefi_group_sha512,ec.ECDSA(utils.Prehashed(hashes.SHA512())))
    uefi_r,uefi_s=utils.decode_dss_signature(uefi_ecdsa_der)
    uefi_ecdsa=uefi_r.to_bytes(48,"big")+uefi_s.to_bytes(48,"big")
    uefi_signature=uefi_version+uefi_sha256+uefi_sha384+uefi_sha512+uefi_ecdsa

    with open(os.path.join(signature_dir,"aos.signature"),"wb") as f:
        f.write(uefi_signature+kernel_signature)

if __name__=="__main__":
    """
    默认将会对文件进行签名。带了参数后才会改变行为。
    """
    parser=argparse.ArgumentParser()
    groups1=parser.add_mutually_exclusive_group()
    groups1.add_argument("-d","-D","-debug","-Debug","-DEBUG",action="store_const",dest="mode",default="Debug",const="Debug")
    groups1.add_argument("-r","-R","-release","-Release","-RELEASE",action="store_const",dest="mode",const="Release")
    groups2=parser.add_mutually_exclusive_group()
    groups2.add_argument("-k","-K","-key","-Key","-KEY",action="store_const",dest="target",const="Key")
    groups2.add_argument("-s","-S","-sign","-Sign","-SIGN",action="store_const",dest="target",default="Sign",const="Sign")
    args=parser.parse_args()

    if args.target=="Key":
        if args.mode=="Release":
            generate_keys(False)
            print("已经生成了新的发行版密钥。")
        else:
            generate_keys(True)
            print("已经生成了新的调试版密钥。")
    else:
        if args.mode=="Release":
            sign_aos(False)
            print("已经生成了新的发行版签名。")
        else:
            sign_aos(True)
            print("已经生成了新的调试版签名。")