import re, sys
src = open('assets/obseg/setup/UsetuparkZ.c', encoding='utf-8', errors='replace').read()
hdr = open('src/bondaicommands.h', encoding='utf-8', errors='replace').read()
lens = dict(re.findall(r'#define (\w+)_LENGTH (0x[0-9a-f]+)', hdr))
def dis(name):
    m = re.search(r'u8 %s\[\] = \{(.*?)\n\};' % name, src, re.S)
    body = m.group(1)
    off = 0; out=[]
    for line in body.splitlines():
        line=line.strip().rstrip(',')
        if not line: continue
        nm = line.split('(')[0].split()[0]
        L = lens.get(nm)
        if L is None:
            out.append(f"{off:5d}  ?? {line[:60]}"); continue
        L=int(L,16)
        out.append(f"{off:5d} +{L:<2d} {line[:70]}")
        off+=L
    return out, off
for name in sys.argv[1:]:
    lines,total = dis(name)
    print(f"== {name} (total {total} bytes) ==")
    for l in lines: print(l)
