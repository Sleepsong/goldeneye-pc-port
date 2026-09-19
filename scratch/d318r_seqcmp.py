import re, sys

def seq(p):
    out = []
    for line in open(p, encoding="utf8", errors="replace"):
        m = re.search(r"n=(\d+) t=(\d+) ra=([0-9a-f]+)", line)
        if m:
            out.append((int(m.group(1)), int(m.group(2)), m.group(3)))
    return out

a = seq("scratch/detA.log")
b = seq("scratch/detB.log")
print("draws A:", len(a), " B:", len(b))

# find first divergence in (t, ra) sequence
n = min(len(a), len(b))
first = None
for i in range(n):
    if a[i][1:] != b[i][1:]:
        first = i
        break
if first is None:
    print("sequences identical over", n, "draws; length diff only")
else:
    print("first divergence at draw index", first)
    for j in range(max(0, first - 3), min(n, first + 4)):
        mark = " <==" if j == first else ""
        sa = a[j]; sb = b[j]
        same = "same" if sa[1:] == sb[1:] else "DIFF"
        print(f"  [{j}] A: t={sa[1]:4d} ra={sa[2]}   B: t={sb[1]:4d} ra={sb[2]}  {same}{mark}")
