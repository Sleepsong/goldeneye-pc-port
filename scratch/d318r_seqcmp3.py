import re, sys

def draws(p):
    out = []
    for line in open(p, encoding="utf8", errors="replace"):
        m = re.search(r"n=(\d+) t=(\d+) ra=([0-9a-f]+)", line)
        if m:
            out.append((int(m.group(2)), m.group(3)))
    return out

a = draws(sys.argv[1])
b = draws(sys.argv[2])
print("draws:", len(a), len(b))
n = min(len(a), len(b))

# compare caller sequence only (ignore tick stamps)
first_ra = None
for i in range(n):
    if a[i][1] != b[i][1]:
        first_ra = i
        break
if first_ra is None:
    print("CALLER SEQUENCE IDENTICAL over", n, "draws")
else:
    print("first caller divergence at draw", first_ra)
    for j in range(max(0, first_ra - 3), min(n, first_ra + 4)):
        tag = "same" if a[j][1] == b[j][1] else "DIFF <=="
        print("  [%d] A t=%4d ra=%s   B t=%4d ra=%s  %s"
              % (j, a[j][0], a[j][1], b[j][0], b[j][1], tag))

# tick-stamp divergence count (pacing jitter)
t_diff = sum(1 for i in range(n) if a[i][0] != b[i][0])
print("draws with differing tick stamps:", t_diff, "of", n)
if n:
    print("final tick: A t=%d  B t=%d" % (a[-1][0], b[-1][0]))
