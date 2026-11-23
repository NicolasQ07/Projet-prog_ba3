CC      = cc
CFLAGS  = -std=c11 -Wall -Wextra -Wpedantic -Wconversion -g -O0
LDFLAGS =

.PHONY: all test test_ocr clean fichiers memcheck

all: OCR

# executable du projet : un seul fichier source
OCR: OCR.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

# fichiers de test
fichiers:
	python3 gen_tests.py

# campagne de tests de OCR (codes d'erreur + CellValue.txt)
test: OCR fichiers
	@echo ""
	@echo "=== Tests de l'executable OCR ==="
	@echo ""
	@bash test_ocr.sh ./OCR tests
	@echo ""
	@echo "=== Campagne terminee ==="

test_ocr: test

# controle des fuites memoire (valgrind, Linux) : seules les erreurs de
# valgrind (code 9) sont signalees, pas les codes d'erreur normaux de OCR
memcheck: OCR fichiers
	@for f in tests/*.bin; do \
	    valgrind --error-exitcode=9 --leak-check=full \
	             --errors-for-leak-kinds=all -q ./OCR $$f 90 90 90 > /dev/null 2>&1; \
	    if [ $$? -eq 9 ]; then echo "  >>> PROBLEME sur $$f"; fi; \
	done

clean:
	rm -f OCR
	rm -rf OCR.dSYM tests
