# 
# 负责对内核文件进行签名与总签名。
# @date 2025-12-16
# 
# Copyright (c) 2025 Tony Chen Smith
# 
# SPDX-License-Identifier: MIT
# 
from cryptography.exceptions import InvalidSignature
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives import hashes,serialization
from cryptography.hazmat.primitives.asymmetric import padding,rsa
import argparse
import hashlib
import json
import os

def sign_kernel(debug):
    '''
    对期望内核文件进行签名，签名后进行校验，检查签名是否合格。
    '''
    kprivate_key_path='sig/kernel.rsa.release.private.pem'
    kpublic_key_path='sig/kernel.rsa.release.public.pem'
    sprivate_key_path='sig/signature.rsa.release.private.pem'
    spublic_key_path='sig/signature.rsa.release.public.pem'
    manifest_path='release.manifest.json'
    sig_path='sig/Release/aos.kernel.sig'
    sig_db_path='sig/Release/aos.kernel.sig.db'
    sig_current_path='sig/Release/current.sig.txt'
    sig_txt_path='sig/Release/aos.kernel.sig.txt'
    if debug:
        kprivate_key_path='sig/kernel.rsa.debug.private.pem'
        kpublic_key_path='sig/kernel.rsa.debug.public.pem'
        sprivate_key_path='sig/signature.rsa.debug.private.pem'
        spublic_key_path='sig/signature.rsa.debug.public.pem'
        manifest_path='debug.manifest.json'
        sig_path='sig/Debug/aos.kernel.sig'
        sig_db_path='sig/Debug/aos.kernel.sig.db'
        sig_current_path='sig/Debug/current.sig.txt'
        sig_txt_path='sig/Debug/aos.kernel.sig.txt'
    
    with open(kprivate_key_path,'rb') as pf:
        pem_data=pf.read()
    kprivate_key=serialization.load_pem_private_key(pem_data,password=None,backend=default_backend())
    with open(kpublic_key_path,'rb') as pf:
        pem_data=pf.read()
    kpublic_key=serialization.load_pem_public_key(pem_data,backend=default_backend())

    with open(sprivate_key_path,'rb') as pf:
        pem_data=pf.read()
    sprivate_key=serialization.load_pem_private_key(pem_data,password=None,backend=default_backend())
    with open(spublic_key_path,'rb') as pf:
        pem_data=pf.read()
    spublic_key=serialization.load_pem_public_key(pem_data,backend=default_backend())

    project_base=os.path.normpath(os.path.abspath('../'))
    kernel_path=None
    with open(manifest_path,'r') as f:
        manifest=json.load(f)
    for info in manifest['manifest']:
        if info['file']=='aos.kernel':
            kernel_path=os.path.join(project_base,info['project'],info['file'])
            break
    if kernel_path==None:
        print('未发现清单内的内核文件路径。')
    else:
        ksha512=hashlib.sha512()
        ksha256=hashlib.sha256()

        with open(kernel_path,'rb') as f:
            while chunk:=f.read(4096):
                ksha256.update(chunk)
                ksha512.update(chunk)
        ksha512hash=ksha512.digest()
        ksha256hash=ksha256.digest()

        ksignature=kprivate_key.sign(ksha512hash,padding=padding.PSS(mgf=padding.MGF1(hashes.SHA512()),salt_length=hashes.SHA512.digest_size),algorithm=hashes.SHA512())
        print('已对当前版本内核进行签名。')
        try:
            kpublic_key.verify(ksignature,data=ksha512hash,padding=padding.PSS(mgf=padding.MGF1(hashes.SHA512()),salt_length=hashes.SHA512.digest_size),algorithm=hashes.SHA512())
            print('对当前版本内核签名验证成功。')
        except InvalidSignature as e:
            print('对当前版本内核签名验证失败。')
            return
        except Exception as e:
            print(f'发生错误：{e}')
            return
        
        sig_data=ksha256hash+ksignature
        with open(sig_current_path,'wb') as f:
            f.write(sig_data)

        sig_db=bytes()
        if os.path.exists(sig_db_path):
            with open(sig_db_path,'rb') as f:
                sig_db=f.read()
        sig_data=sig_data+sig_db

        ssha512=hashlib.sha512()
        ssha256=hashlib.sha256()

        ssha512.update(sig_data)
        ssha256.update(sig_data)
        
        ssha512hash=ssha512.digest()
        ssha256hash=ssha256.digest()
        ssignature=sprivate_key.sign(ssha512hash,padding=padding.PSS(mgf=padding.MGF1(hashes.SHA512()),salt_length=hashes.SHA512.digest_size),algorithm=hashes.SHA512())
        print('已对当前版本签名文件进行签名。')
        try:
            spublic_key.verify(ssignature,data=ssha512hash,padding=padding.PSS(mgf=padding.MGF1(hashes.SHA512()),salt_length=hashes.SHA512.digest_size),algorithm=hashes.SHA512())
            print('对当前版本签名文件签名验证成功。')
        except InvalidSignature as e:
            print('对当前版本签名文件签名验证失败。')
            return
        except Exception as e:
            print(f'发生错误：{e}')
            return

        with open(sig_txt_path,'w') as f:
            f.write(f'[K]SHA-256[{len(ksha256hash):04d}]:{ksha256hash.hex().upper()}\n')
            f.write(f'[K]SHA-512[{len(ksha512hash):04d}]:{ksha512hash.hex().upper()}\n')
            f.write(f'[K]RSA-PSS[{len(ksignature):04d}]:{ksignature.hex().upper()}\n')
            f.write(f'[S]SHA-256[{len(ssha256hash):04d}]:{ssha256hash.hex().upper()}\n')
            f.write(f'[S]SHA-512[{len(ssha512hash):04d}]:{ssha512hash.hex().upper()}\n')
            f.write(f'[S]RSA-PSS[{len(ssignature):04d}]:{ssignature.hex().upper()}\n')

        with open(sig_path,'wb') as f:
            f.write(ssha256hash)
            f.write(ssignature)
            f.write(sig_data)

def generate_key():
    '''
    生成新密钥。
    '''
    private_key=rsa.generate_private_key(public_exponent=65537,key_size=4096,backend=default_backend())
    private_key_content=private_key.private_bytes(encoding=serialization.Encoding.PEM,format=serialization.PrivateFormat.PKCS8,encryption_algorithm=serialization.NoEncryption())
    with open('sig/kernel.rsa.debug.private.pem','wb') as pf:
        pf.write(private_key_content)
    
    public_key=private_key.public_key()
    public_key_content=public_key.public_bytes(encoding=serialization.Encoding.PEM,format=serialization.PublicFormat.PKCS1)
    with open('sig/kernel.rsa.debug.public.pem','wb') as pf:
        pf.write(public_key_content)

    private_key=rsa.generate_private_key(public_exponent=65537,key_size=4096,backend=default_backend())
    private_key_content=private_key.private_bytes(encoding=serialization.Encoding.PEM,format=serialization.PrivateFormat.PKCS8,encryption_algorithm=serialization.NoEncryption())
    with open('sig/kernel.rsa.release.private.pem','wb') as pf:
        pf.write(private_key_content)
    
    public_key=private_key.public_key()
    public_key_content=public_key.public_bytes(encoding=serialization.Encoding.PEM,format=serialization.PublicFormat.PKCS1)
    with open('sig/kernel.rsa.release.public.pem','wb') as pf:
        pf.write(public_key_content)
    
    private_key=rsa.generate_private_key(public_exponent=65537,key_size=4096,backend=default_backend())
    private_key_content=private_key.private_bytes(encoding=serialization.Encoding.PEM,format=serialization.PrivateFormat.PKCS8,encryption_algorithm=serialization.NoEncryption())
    with open('sig/signature.rsa.debug.private.pem','wb') as pf:
        pf.write(private_key_content)
    
    public_key=private_key.public_key()
    public_key_content=public_key.public_bytes(encoding=serialization.Encoding.PEM,format=serialization.PublicFormat.PKCS1)
    with open('sig/signature.rsa.debug.public.pem','wb') as pf:
        pf.write(public_key_content)

    private_key=rsa.generate_private_key(public_exponent=65537,key_size=4096,backend=default_backend())
    private_key_content=private_key.private_bytes(encoding=serialization.Encoding.PEM,format=serialization.PrivateFormat.PKCS8,encryption_algorithm=serialization.NoEncryption())
    with open('sig/signature.rsa.release.private.pem','wb') as pf:
        pf.write(private_key_content)
    
    public_key=private_key.public_key()
    public_key_content=public_key.public_bytes(encoding=serialization.Encoding.PEM,format=serialization.PublicFormat.PKCS1)
    with open('sig/signature.rsa.release.public.pem','wb') as pf:
        pf.write(public_key_content)

def export_public():
    '''
    导出公钥成数组元素。
    '''
    with open('sig/kernel.rsa.debug.public.pem','rb') as pf:
        pem_data=pf.read()
    public_key=serialization.load_pem_public_key(pem_data,backend=default_backend())
    public_numbers=public_key.public_numbers()
    n_bytes=public_numbers.n.to_bytes(512,'big')
    e_bytes=public_numbers.e.to_bytes(3,'big')

    with open(f'sig/Debug/kernel_keyn.txt','w') as nf:
        for i in range(0,len(n_bytes),16):
            line=n_bytes[i:i+16]
            hex_str=','.join([f'0x{b:02X}'for b in line])
            nf.write(f'{hex_str}')
            if i+16<len(n_bytes):
                nf.write(',\n')
            else:
                nf.write('\n')
    
    with open(f'sig/Debug/kernel_keye.txt','w') as ef:
        hex_str=','.join([f'0x{b:02X}'for b in e_bytes])
        ef.write(f'{hex_str}\n')
    
    with open('sig/kernel.rsa.release.public.pem','rb') as pf:
        pem_data=pf.read()
    
    public_key=serialization.load_pem_public_key(pem_data,backend=default_backend())
    public_numbers=public_key.public_numbers()
    n_bytes=public_numbers.n.to_bytes(512,'big')
    e_bytes=public_numbers.e.to_bytes(3,'big')

    with open(f'sig/Release/kernel_keyn.txt','w') as nf:
        for i in range(0,len(n_bytes),16):
            line=n_bytes[i:i+16]
            hex_str=','.join([f'0x{b:02X}'for b in line])
            nf.write(f'{hex_str}')
            if i+16<len(n_bytes):
                nf.write(',\n')
            else:
                nf.write('\n')
    
    with open(f'sig/Release/kernel_keye.txt','w') as ef:
        hex_str=','.join([f'0x{b:02X}'for b in e_bytes])
        ef.write(f'{hex_str}\n')
    
    with open('sig/signature.rsa.debug.public.pem','rb') as pf:
        pem_data=pf.read()
    public_key=serialization.load_pem_public_key(pem_data,backend=default_backend())
    public_numbers=public_key.public_numbers()
    n_bytes=public_numbers.n.to_bytes(512,'big')
    e_bytes=public_numbers.e.to_bytes(3,'big')

    with open(f'sig/Debug/signature_keyn.txt','w') as nf:
        for i in range(0,len(n_bytes),16):
            line=n_bytes[i:i+16]
            hex_str=','.join([f'0x{b:02X}'for b in line])
            nf.write(f'{hex_str}')
            if i+16<len(n_bytes):
                nf.write(',\n')
            else:
                nf.write('\n')
    
    with open(f'sig/Debug/signature_keye.txt','w') as ef:
        hex_str=','.join([f'0x{b:02X}'for b in e_bytes])
        ef.write(f'{hex_str}\n')
    
    with open('sig/signature.rsa.release.public.pem','rb') as pf:
        pem_data=pf.read()
    
    public_key=serialization.load_pem_public_key(pem_data,backend=default_backend())
    public_numbers=public_key.public_numbers()
    n_bytes=public_numbers.n.to_bytes(512,'big')
    e_bytes=public_numbers.e.to_bytes(3,'big')

    with open(f'sig/Release/signature_keyn.txt','w') as nf:
        for i in range(0,len(n_bytes),16):
            line=n_bytes[i:i+16]
            hex_str=','.join([f'0x{b:02X}'for b in line])
            nf.write(f'{hex_str}')
            if i+16<len(n_bytes):
                nf.write(',\n')
            else:
                nf.write('\n')
    
    with open(f'sig/Release/signature_keye.txt','w') as ef:
        hex_str=','.join([f'0x{b:02X}'for b in e_bytes])
        ef.write(f'{hex_str}\n')

if __name__=="__main__":
    '''
    默认将会对文件进行签名。带了参数后才会改变行为。
    '''
    parser=argparse.ArgumentParser()
    groups=parser.add_mutually_exclusive_group()
    groups.add_argument('-d','-D','-debug','-Debug','-DEBUG',action='store_const',dest='mode',default='Debug',const='Debug')
    groups.add_argument('-r','-R','-release','-Release','-RELEASE',action='store_const',dest='mode',const='Release')
    groups.add_argument('-k','-K','-key','-Key','-KEY',action='store_const',dest='mode',const='Key')
    groups.add_argument('-e', '-E', '-export', '-Export', '-EXPORT',action='store_const', dest='mode', const='Export')
    args=parser.parse_args()

    if args.mode=='Release':
        sign_kernel(False)
        print('已经完成对发行版内核签名。')
    elif args.mode=='Key':
        generate_key()
        print('已经生成了新的密钥，请注意备份。')
    elif args.mode=='Export':
        export_public()
        print('已经将公钥转换成数组元素。')
    else:
        sign_kernel(True)
        print('已经完成对调试版内核签名。')
