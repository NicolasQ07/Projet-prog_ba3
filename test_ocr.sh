#!/bin/bash
# ============================================================
#  test_ocr.sh -- campagne de tests de l'executable OCR
#  Usage : bash test_ocr.sh <executable OCR> <dossier des .bin>
#
#  Chaque cas lance OCR et compare son code de sortie au code
#  attendu. Sur macOS/Linux le code de sortie est tronque a
#  8 bits : le code attendu est donc compare modulo 256
#  (-110 -> 146, -135 -> 121).
#  En plus :
#   - succes : CellValue.txt doit exister et respecter le
#              format  d:'<0|1| >', <pourcentage>%
#              (avec reconnu : contenir le chiffre attendu)
#   - erreur : aucun CellValue.txt ne doit etre ecrit
#  Les cas sont executes dans un dossier temporaire, le
#  dossier des .bin n'est pas modifie.
# ============================================================

if [ $# -ne 2 ]; then
    echo "Usage : bash $0 <executable OCR> <dossier des .bin>" >&2
    exit 2
fi

OCR="$(cd "$(dirname -- "$1")" && pwd)/$(basename -- "$1")"
SRC="$2"
[ -x "$OCR" ]          || { echo "$1 introuvable ou non executable" >&2; exit 2; }
[ -f "$SRC/t_ok.bin" ] || { echo "$SRC/t_ok.bin absent (lancer gen_tests.py)" >&2; exit 2; }

TMP="$(mktemp -d "${TMPDIR:-/tmp}/test_ocr.XXXXXX")" || exit 2
trap 'chmod -R u+rwx "$TMP"; rm -rf "$TMP"' EXIT
cp "$SRC"/*.bin "$TMP"/
cd "$TMP" || exit 2

# dossiers et fichiers speciaux
mkdir dossier sous lecture_seule cible_dossier cible_dossier/CellValue.txt
cp t_chiffre0.bin sous/Cell.bin
cp t_chiffre0.bin lecture_seule/Cell.bin
cp t_chiffre0.bin cible_dossier/Cell.bin
cp t_ok.bin illisible.bin
chmod 555 lecture_seule
chmod 000 illisible.bin

ok=0
echecs=0

# check <code attendu> <description> [arguments de OCR...]
check() {
    local attendu=$1 nom=$2 sortie=CellValue.txt obtenu probleme="" msg
    shift 2
    [ $# -ge 1 ] && sortie="$(dirname -- "$1")/CellValue.txt"
    [ -f "$sortie" ] && rm -f "$sortie"

    "$OCR" "$@" > stdout.txt 2> stderr.txt
    obtenu=$?

    [ "$obtenu" -ne $(( (attendu + 256) % 256 )) ] && probleme="code de sortie $obtenu"
    if [ "$attendu" -eq 0 ]; then
        grep -Eqx "d:'[01 ]', [0-9]{1,3}\.[0-9]{4}%" "$sortie" 2>/dev/null \
            || probleme="${probleme:+$probleme, }CellValue.txt absent ou mal forme"
        if [ -n "$chiffre_attendu" ] && ! grep -q "^d:'$chiffre_attendu'," "$sortie" 2>/dev/null; then
            probleme="${probleme:+$probleme, }chiffre attendu '$chiffre_attendu'"
        fi
        msg="$(tail -n 1 stdout.txt)"
    else
        [ -f "$sortie" ] && probleme="${probleme:+$probleme, }CellValue.txt ecrit malgre l'erreur"
        msg="$(tail -n 1 stderr.txt)"
    fi

    if [ -z "$probleme" ]; then
        ok=$((ok + 1))
        printf 'OK    | %-24s | %5d (%3d) | %s\n' "$nom" "$attendu" "$obtenu" "$msg"
    else
        echecs=$((echecs + 1))
        printf 'ECHEC | %-24s | %5d (%3d) | %s\n' "$nom" "$attendu" "$obtenu" "$msg"
        printf '        attendu %d (sortie %d), probleme : %s\n' \
               "$attendu" $(( (attendu + 256) % 256 )) "$probleme"
    fi
}

# reconnu <chiffre attendu, ' ' = vide> <description> [arguments de OCR...]
reconnu() {
    local chiffre_attendu=$1
    shift
    check 0 "$@"
}

echo "--- parametres"
check -100 "aucun parametre"
check -100 "2 seuils seulement"   t_ok.bin 90 90
check -100 "4 seuils"             t_ok.bin 90 90 90 90
check -105 "seuil 'abc'"          t_ok.bin abc 90 90
check -105 "seuil vide"           t_ok.bin "" 90 90
check -105 "seuil -1"             t_ok.bin -1 90 90
check -105 "seuil 101"            t_ok.bin 101 90 90
check -105 "seuil 50.5"           t_ok.bin 50.5 90 90
check -105 "seuil '9x'"           t_ok.bin 9x 90 90
check -105 "seuil 1e2"            t_ok.bin 1e2 90 90
check -105 "seuil trop grand"     t_ok.bin 99999999999999999999 90 90
check -105 "3e seuil invalide"    t_ok.bin 90 90 abc
check    0 "seuils 0, 100, +7"    t_ok.bin 0 100 +7

echo "--- fichier Cell.bin"
check -110 "fichier inexistant"   inexistant.bin 90 90 90
check -110 "chemin vide"          "" 90 90 90
check -110 "fichier illisible"    illisible.bin 90 90 90
check -135 "chemin = dossier"     dossier 90 90 90
check -115 "fichier vide"         t_vide.bin 90 90 90
check -115 "entete tronquee"      t_tronq.bin 90 90 90
check -120 "W = 0"                t_zero.bin 90 90 90
check -120 "W = 9"                t_sous10.bin 90 90 90
check -120 "W = 101"              t_sur100.bin 90 90 90
check -120 "W = -100 (unsigned)"  t_neg100.bin 90 90 90
check -120 "entete big endian"    t_swap.bin 90 90 90
check -125 "W = 31"               t_31.bin 90 90 90
check -125 "20 x 20"              t_20.bin 90 90 90
check -140 "entete sans pixel"    t_entete.bin 90 90 90
check -140 "5000 / 5625 pixels"   t_manque.bin 90 90 90
check -145 "6000 / 5625 pixels"   t_trop.bin 90 90 90
check -150 "un pixel = 2"         t_pixel2.bin 90 90 90

echo "--- fichier CellValue.txt"
check -160 "dossier lecture seule" lecture_seule/Cell.bin 90 90 90
check -160 "CellValue.txt dossier" cible_dossier/Cell.bin 90 90 90

echo "--- cases vides"
reconnu " " "vide 75 x 71"         t_nonsquare.bin 90 90 90
reconnu " " "vide 32 x 32"         t_min32.bin 90 90 90
reconnu " " "vide 100 x 100"       t_max100.bin 90 90 90

echo "--- reconnaissance (etape 3)"
reconnu 0 "'0' 75 x 75"           t_chiffre0.bin 99 90 90
reconnu 1 "'1' 75 x 75"           t_chiffre1.bin 99 90 90
reconnu 0 "'0' 32 x 32 exact"     t_chiffre0_32.bin 99 90 90
reconnu 1 "'1' 44 x 44 en bas"    t_chiffre1_44.bin 99 90 90
reconnu 0 "'0' 75 x 71 en bas"    t_chiffre0_bas.bin 99 90 90
reconnu 0 "'0', seuils bas 70"    t_chiffre0.bin 99 70 70
reconnu 1 "'1', seuils bas 70"    t_chiffre1.bin 99 70 70
reconnu 0 "chemin absolu"         "$TMP/sous/Cell.bin" 99 90 90
check -155 "case toute noire"     t_noir.bin 90 90 90
check -155 "cadre seul"           t_ok.bin 90 90 90

echo ""
echo "$((ok + echecs)) cas, $echecs echec(s)"
[ "$echecs" -eq 0 ]
