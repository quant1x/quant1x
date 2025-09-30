import os,sys,codecs,struct
home=os.path.expanduser('~/.q2x-rust')
meta=os.path.join(home,'meta')
print('meta dir=',meta)
idx_files=['tdxzs.cfg','tdxzs3.cfg']
index_map={}  # name -> raw hex
index_raws=[]
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
        try:
            name = name_bytes.decode('gbk').strip()
        except:
            name = name_bytes.decode('utf-8','ignore').strip()
        hx = name_bytes.hex().upper()
        index_map[name]=hx
        index_raws.append((name,hx,f))
print('index count=',len(index_map))
# parse all block files
bfs=['block.dat','block_gn.dat','block_fg.dat','block_zs.dat']
parsed=[]
for bf in bfs:
    bd=os.path.join(meta,bf)
    if not os.path.exists(bd):
        #print('no',bd)
        continue
    data=open(bd,'rb').read()
    if len(data)<=400:
        continue
    pos=384
    if pos+2>len(data): continue
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
        parsed.append((name,raw9.hex().upper(),bf))
print('parsed total entries=',len(parsed))
# unique parsed names
parsed_map={}
for name,hx,f in parsed:
    if name not in parsed_map:
        parsed_map[name]=hx
print('unique parsed names=',len(parsed_map))
# compare
index_names=set(index_map.keys())
parsed_names=set(parsed_map.keys())
missing = sorted(list(index_names - parsed_names))
extra = sorted(list(parsed_names - index_names))
print('index_names=',len(index_names),'parsed_names=',len(parsed_names),'missing_index_names=',len(missing),'extra_parsed_names=',len(extra))
print('\nSamples of missing (up to 40):')
for nm in missing[:40]:
    print(nm, index_map.get(nm))
print('\nSamples of extra (up to 40):')
for nm in extra[:40]:
    print(nm, parsed_map.get(nm))
# check names that exist in both but with different raw hex
hex_mismatch=[]
for name in index_names & parsed_names:
    idx_hex=index_map.get(name)
    parsed_hex=parsed_map.get(name)
    if idx_hex!=parsed_hex:
        hex_mismatch.append((name,idx_hex,parsed_hex))
print('\nNames present both but raw hex differ:',len(hex_mismatch))
for t in hex_mismatch[:40]:
    print(t)
# show first 50 parsed entries for inspection
print('\nFirst 50 parsed entries:')
for p in list(parsed)[:50]:
    print(p)
