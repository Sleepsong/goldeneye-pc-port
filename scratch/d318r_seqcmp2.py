import re, sys

def seq(p):
    out = []
    for line in open(p, encoding="utf8", errors="replace"):
        m = re.search(r"n=(\d+) t=(\d+) ra=([0-9a-f]+)", line)
        if m:
            out.append((int(m.group(2)), m.group(3)))
    return out

a = seq(sys.argv[1])
b = seq(sys.argv[2])
print("draws:", len(a), len(b))
n = min(len(a), len(b))
first = None
for i in range(n):
    if a[i] != b[i]:
        first = i
        break
if first is None:
    print("FULL SEQUENCE IDENTICAL over", n, "draws (tick, caller)")
else:
    print("first divergence at draw index", first)
    for j in range(max(0, first - 2), min(n, first + 3)):
        tag = "same" if a[j] == b[j] else "DIFF <=="
        print("  [%d] A t=%4d ra=%s   B t=%4d ra=%s  %s"
              % (j, a[j][0], a[j][1], b[j][0], b[j][1], tag))
