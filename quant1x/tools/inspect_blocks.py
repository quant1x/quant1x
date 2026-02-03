from quant1x import config
from quant1x.market.blocks import parse_raw_block_file, get_sector_filename
import os, json, datetime
meta = config.meta_path
print('meta_path ->', meta)
files = sorted(os.listdir(meta))
print('FILES:')
for f in files:
    print(' ', f)

for name in ('tdxzs.cfg','tdxzs3.cfg','tdxhy.cfg'):
    p = os.path.join(meta, name)
    print('\n==', name, 'EXISTS' if os.path.exists(p) else 'MISSING')
    if os.path.exists(p):
        try:
            with open(p, 'r', encoding='utf-8', errors='replace') as fh:
                for i, line in enumerate(fh):
                    print(line.rstrip()[:200])
                    if i >= 9:
                        break
        except Exception as e:
            print('  cannot read', e)

# parse block files
for b in ('block.dat', 'block_gn.dat', 'block_fg.dat', 'block_zs.dat'):
    p = os.path.join(meta, b)
    print('\n== parse', b, 'exists' if os.path.exists(p) else 'MISSING')
    if os.path.exists(p):
        recs = parse_raw_block_file(b)
        print('  records:', len(recs))
        if recs:
            print('  sample:', json.dumps(recs[0], ensure_ascii=False)[:1000])

# show generated blocks file head
bf = get_sector_filename()
print('\n== Generated sector file:', bf, 'EXISTS' if os.path.exists(bf) else 'MISSING')
if os.path.exists(bf):
    try:
        with open(bf, 'r', encoding='utf-8', errors='replace') as fh:
            for i, line in enumerate(fh):
                print(line.rstrip())
                if i >= 20:
                    break
    except Exception as e:
        print(' cannot read generated file:', e)

print('\nDone')

# Extra diagnostics: attempt to run generator and show mappings
from quant1x.market.blocks import parse_and_generate_block_file
print('\n== run parse_and_generate_block_file()')
res = parse_and_generate_block_file()
print('  result ->', res)

# inspect cfg index and raw name mapping
def load_cfg_index(name):
    fn = os.path.join(meta, name)
    out = []
    if not os.path.exists(fn):
        return out
    with open(fn, 'r', encoding='gbk', errors='ignore') as fh:
        for line in fh:
            line = line.strip()
            if not line:
                continue
            arr = line.split('|')
            out.append({'name': arr[0] if len(arr)>0 else '', 'code': arr[1] if len(arr)>1 else '', 'type': int(arr[2]) if len(arr)>2 and arr[2].isdigit() else 0, 'block': arr[5] if len(arr)>5 else ''})
    return out

cfg_index = []
for cfg in ('tdxzs.cfg','tdxzs3.cfg'):
    cfg_index.extend(load_cfg_index(cfg))
print('cfg entries:', len(cfg_index))

name2block = {}
for f in ('block.dat','block_gn.dat','block_fg.dat','block_zs.dat'):
    if os.path.exists(os.path.join(meta, f)):
        recs = parse_raw_block_file(f)
        for r in recs:
            name2block[r['block_name']] = r

matches = 0
for v in cfg_index:
    if v['name'] in name2block:
        matches += 1
print('cfg names matched in raw files:', matches, '/', len(cfg_index))

if matches < 5:
    print('Sample cfg names not matched:')
    for v in cfg_index[:50]:
        if v['name'] not in name2block:
            print(' ', v['name'])
            break
