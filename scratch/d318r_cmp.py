import bisect, sys

syms = []
for line in open("scratch/syms.txt"):
    p = line.split()
    if len(p) >= 2:
        try:
            syms.append((int(p[0], 16), p[1]))
        except ValueError:
            pass
syms.sort()
keys = [s[0] for s in syms]

def resolve(ra):
    a = int(ra, 16)
    i = bisect.bisect_right(keys, a) - 1
    if i < 0:
        return "??"
    b, n = syms[i]
    return "%s+%x" % (n, a - b)

def counts(p):
    d = {}
    for line in open(p):
        q = line.split()
        if len(q) >= 2:
            d[resolve(q[1].replace("ra=", ""))] = int(q[0])
    return d

a = counts("scratch/d318r_addrs1.txt")
b = counts("scratch/d318r_addrs2.txt")
allk = sorted(set(a) | set(b), key=lambda k: -(a.get(k, 0) + b.get(k, 0)))
print("%-45s %5s %5s" % ("caller", "run1", "run2"))
for k in allk[:18]:
    mark = "   <== DIFFERS" if a.get(k, 0) != b.get(k, 0) else ""
    print("%-45s %5d %5d%s" % (k, a.get(k, 0), b.get(k, 0), mark))
print("total draws:", sum(a.values()), sum(b.values()))
