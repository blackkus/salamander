# -*- coding: utf-8 -*-
# One-shot rename: French identifiers -> English, across code, scripts, config, docs.
# Identifiers only; French prose in *.md docs is kept where it is not an identifier.
import io, os

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# (old, new) applied in order; case variants handled by listing them explicitly
PAIRS = [
    ('NICK', 'NICK'),
    ('COLOR', 'COLOR'),
    ('Nick', 'Nick'),
    ('nick', 'nick'),
    ('PLACEHOLDER', 'PLACEHOLDER'),
    ('World_Test', 'World_Test'),
    ('M_GroundGreen', 'M_GroundGreen'),
    # gameplay tag dev comments (config)
    ('DevComment="Damage SetByCaller (GE_Damage)"', 'DevComment="Damage SetByCaller (GE_Damage)"'),
    ('DevComment="Player dead (T8)"', 'DevComment="Player dead (T8)"'),
    ('DevComment="Melee attack (T9)"', 'DevComment="Melee attack (T9)"'),
]

EXTS = ('.cpp', '.h', '.cs', '.py', '.bat', '.ini', '.md')
SKIP_DIRS = {'Binaries', 'Intermediate', 'Saved', '.git', 'DerivedDataCache', 'obj', 'bin'}

changed = []
for dirpath, dirnames, filenames in os.walk(ROOT):
    dirnames[:] = [d for d in dirnames if d not in SKIP_DIRS]
    for fn in filenames:
        if not fn.endswith(EXTS):
            continue
        p = os.path.join(dirpath, fn)
        rel = os.path.relpath(p, ROOT)
        if rel.startswith('Content'):
            continue  # asset names handled by the editor commandlet
        try:
            s = io.open(p, encoding='utf-8').read()
        except Exception:
            continue
        orig = s
        for old, new in PAIRS:
            s = s.replace(old, new)
        if s != orig:
            io.open(p, 'w', encoding='utf-8', newline='').write(s)
            changed.append(rel)

print('RENAMED_FILES=%d' % len(changed))
for c in sorted(changed):
    print('  ' + c)
