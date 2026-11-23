#!/usr/bin/env python3
"""Genere les fichiers Cell.bin de test, valides et corrompus."""

import struct, os, random

DOSSIER = "tests"
os.makedirs(DOSSIER, exist_ok=True)

def gen(nom, W, H, nb_pixels, swap=False, motif=None):
    """Ecrit un fichier : entete (W,H) puis nb_pixels octets."""
    fmt = '>I' if swap else '<I'
    chemin = os.path.join(DOSSIER, nom)
    with open(chemin, 'wb') as f:
        f.write(struct.pack(fmt, W))
        f.write(struct.pack(fmt, H))
        if motif is None:
            f.write(bytes(nb_pixels))
        else:
            f.write(bytes(motif(i) for i in range(nb_pixels)))
    print(f"  {nom:<18} {os.path.getsize(chemin):>6} octets")

def cadre(W, H):
    """Pixels noirs sur le bord, blancs ailleurs (imite un bord de case)."""
    def f(i):
        l, c = divmod(i, W)
        return 1 if (l < 2 or l >= H-2 or c < 2 or c >= W-2) else 0
    return f

# bitmaps de FontRasterized_0_1.h (moodle), pour dessiner de vrais chiffres
CHIFFRES = {
    0: [0x0, 0x0, 0xFF0000, 0x3FFC000, 0x7FFE000, 0xFFFE000, 0xFFFF000, 0x1FC3F000,
        0x1F83F000, 0x1F81F800, 0x3F01F800, 0x3F01F800, 0x3F01F800, 0x3F01F800, 0x3F01F800, 0x3F01F800,
        0x3F01F800, 0x3F01F800, 0x3F01F800, 0x3F01F800, 0x3F01F800, 0x3F01F800, 0x1F81F800, 0x1F81F800,
        0x1FC3F000, 0xFE7F000, 0xFFFF000, 0x7FFE000, 0x7FFC000, 0x1FF8000, 0x3C0000, 0x0],
    1: [0x0, 0x0, 0x1F0000, 0x1F0000, 0x3F0000, 0x7F0000, 0x1FF0000, 0x1FFF0000,
        0x1FFF0000, 0x1FFF0000, 0xFFF0000, 0x3F0000, 0x3F0000, 0x3F0000, 0x3F0000, 0x3F0000,
        0x3F0000, 0x3F0000, 0x3F0000, 0x3F0000, 0x3F0000, 0x3F0000, 0x3F0000, 0x3F0000,
        0x3F0000, 0x3F0000, 0x3F0000, 0x3F0000, 0x3F0000, 0x3F0000, 0x0, 0x0],
}

def chiffre(d, W, x, y):
    """Case blanche de largeur W avec le chiffre d, coin haut-gauche en (x, y)."""
    def f(i):
        l, c = divmod(i, W)
        r, k = l - y, c - x
        return 1 if 0 <= r < 32 and 0 <= k < 32 and (CHIFFRES[d][r] >> (31 - k)) & 1 else 0
    return f

print("Generation des fichiers de test :")

# --- cas valides -------------------------------------------------
gen('t_ok.bin',        75, 75, 75*75, motif=cadre(75, 75))
gen('t_nonsquare.bin', 75, 71, 75*71)
gen('t_min32.bin',     32, 32, 32*32)
gen('t_max100.bin',   100,100, 100*100)

# --- reconnaissance des chiffres (etape 3) -----------------------
gen('t_chiffre0.bin',    75, 75, 75*75, motif=chiffre(0, 75, 21, 21))
gen('t_chiffre1.bin',    75, 75, 75*75, motif=chiffre(1, 75, 21, 21))
gen('t_chiffre0_32.bin', 32, 32, 32*32, motif=chiffre(0, 32, 0, 0))    # une seule position
gen('t_chiffre1_44.bin', 44, 44, 44*44, motif=chiffre(1, 44, 12, 12))  # derniere position
gen('t_chiffre0_bas.bin', 75, 71, 75*71, motif=chiffre(0, 75, 43, 39)) # derniere position, non carree
gen('t_noir.bin',        75, 75, 75*75, motif=lambda i: 1)

# --- dimensions hors bornes -------------------------------------
gen('t_zero.bin',       0, 75, 0)
gen('t_sous10.bin',     9, 75, 9*75)
gen('t_sur100.bin',   101, 75, 101*75)
gen('t_neg100.bin', 4294967196, 75, 100)

# --- dimensions trop petites pour le bitmap 32x32 ---------------
gen('t_31.bin',        31, 50, 31*50)
gen('t_20.bin',        20, 20, 20*20)

# --- incoherence taille / entete --------------------------------
gen('t_manque.bin',    75, 75, 5000)
gen('t_trop.bin',      75, 75, 6000)
gen('t_entete.bin',    75, 75, 0)

# --- valeur de pixel invalide (un pixel = 2) ---------------------
gen('t_pixel2.bin',    75, 75, 75*75, motif=lambda i: 2 if i == 100 else 0)

# --- endianness --------------------------------------------------
gen('t_swap.bin',      75, 75, 75*75, swap=True)

# --- entetes cassees ---------------------------------------------
open(os.path.join(DOSSIER, 't_vide.bin'), 'wb').close()
print(f"  {'t_vide.bin':<18} {0:>6} octets")

with open(os.path.join(DOSSIER, 't_tronq.bin'), 'wb') as f:
    f.write(b'\x4b\x00\x00\x00\x4b\x00')      # W complet, H tronque
print(f"  {'t_tronq.bin':<18} {6:>6} octets")

print("\nTaille attendue pour 75x75 :", 8 + 75*75, "octets")