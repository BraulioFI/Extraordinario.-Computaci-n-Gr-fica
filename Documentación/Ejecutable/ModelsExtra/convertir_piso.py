import os
import shutil

# ---------------- Configuracion ----------------
TEXTURAS = {
    "Pasto":  "pastotextura.jpg",
    "Tierra": "tierra.jpg",
    "Arena":  "arena.jpg",
    "Agua":   "agua.jpg",
}

# El piso es una rejilla de 16x16 celdas de 6.25 unidades.
# Columna i = (X + 50) // 6.25 ; fila j = (Z + 50) // 6.25
# El rio es una lista de celdas de agua por fila (j: [columnas i]).
# Serpentea de norte (j=2) a sur (j=9) sobre la zona plana del terreno.
RIO = {
    2: [12],
    3: [12],
    4: [11, 12],
    5: [11],
    6: [11],
    7: [11, 12],
    8: [12],
    9: [12],
}

AGUA = {(i, j) for j, cols in RIO.items() for i in cols}

def material(cx, cz):
    """Decide el material de una cara segun su centro (X, Z)."""
    i = int((cx + 50) // 6.25)
    j = int((cz + 50) // 6.25)
    if (i, j) in AGUA:
        return "Agua"
    for (a, b) in AGUA:
        if abs(i - a) + abs(j - b) == 1:   # orilla: solo celdas vecinas directas
            return "Arena"
    if cx <= -31.25:
        return "Tierra"
    return "Pasto"
# -----------------------------------------------

if not os.path.exists("Floor_original.obj"):
    shutil.copy("Floor.obj", "Floor_original.obj")
    if os.path.exists("Floor.mtl"):
        shutil.copy("Floor.mtl", "Floor_original.mtl")

lineas = open("Floor_original.obj", encoding="utf-8").read().splitlines()

verts, cabecera, caras = [], [], []
for ln in lineas:
    if ln.startswith("v "):
        p = ln.split()
        verts.append((float(p[1]), float(p[2]), float(p[3])))
        cabecera.append(ln)
    elif ln.startswith("f "):
        caras.append(ln)
    elif ln.startswith("usemtl"):
        continue
    else:
        cabecera.append(ln)

def centro(cara):
    idx = [int(t.split("/")[0]) for t in cara.split()[1:]]
    xs = [verts[i - 1][0] for i in idx]
    zs = [verts[i - 1][2] for i in idx]
    return sum(xs) / len(xs), sum(zs) / len(zs)

grupos = {m: [] for m in TEXTURAS}
for c in caras:
    cx, cz = centro(c)
    grupos[material(cx, cz)].append(c)

salida = cabecera[:]
for m in TEXTURAS:
    if grupos[m]:
        salida.append("usemtl " + m)
        salida.extend(grupos[m])

with open("Floor.obj", "w", encoding="utf-8") as f:
    f.write("\n".join(salida) + "\n")

with open("Floor.mtl", "w", encoding="utf-8") as f:
    f.write("# Materiales del piso\n")
    for m, tex in TEXTURAS.items():
        f.write("newmtl %s\nNs 250.000000\nKa 1.000000 1.000000 1.000000\n"
                "Ks 0.500000 0.500000 0.500000\nKe 0.000000 0.000000 0.000000\n"
                "Ni 1.500000\nd 1.000000\nillum 2\nmap_Kd %s\n\n" % (m, tex))

print("Listo. Caras por material:")
for m in TEXTURAS:
    print("  %-7s %d" % (m, len(grupos[m])))