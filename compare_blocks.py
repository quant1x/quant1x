import os,sys,codecs
home=os.path.expanduser('~/.q2x-rust')
meta=os.path.join(home,'meta')
print('meta dir=',meta)
idx_files=['tdxzs.cfg','tdxzs3.cfg']
index_map={}
for f in idx_files:
    p=os.path.join(meta,f)
    if not os.path.exists(p):
        print('missing',p)
        continue
    b=open(p,'rb').read()
    for line in b.split(b'\n'):
        if not line: continue
        s=line.rstrip(b'\r')
        sep = s.find(b'|')
        name_bytes = s if sep==-1 else s[:sep]
        # decode GBK per C++/Rust
        try:
            name = name_bytes.decode('gbk').strip()
        except:
            name = name_bytes.decode('utf-8','ignore').strip()
        index_map[name]=name_bytes.hex().upper()
print('index count=',len(index_map))
# parse block.dat
bd=os.path.join(meta,'block.dat')
parsed=[]
if os.path.exists(bd):
    data=open(bd,'rb').read()
    if len(data)>400:
        # skip 384
        pos=384
        import struct
        if pos+2<=len(data):
            cnt=struct.unpack_from('<H',data,pos)[0]
            pos+=2
            for i in range(cnt):
                if pos+2813>len(data): break
                tmp1=data[pos:pos+2813]
                pos+=2813
                raw9=tmp1[:9]
                if b'\x00' in raw9:
                    raw9=raw9[:raw9.find(b'\x00')]
                try:
                    name=raw9.decode('gbk').strip()
                except:
                    name=raw9.decode('utf-8','ignore').strip()
                parsed.append((name,raw9.hex().upper()))
print('parsed count=',len(parsed))
# compare
mismatch=[]
for name,hexv in parsed:
    idx_hex=index_map.get(name)
    if idx_hex is None:
        mismatch.append((name,hexv,index_map.get(name,None)))
print('mismatch count=',len(mismatch))
for i in mismatch[:50]:
    print(i)
