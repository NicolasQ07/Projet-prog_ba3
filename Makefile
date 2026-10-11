CC      = cc
CFLAGS  = -std=c11 -Wall -Wextra -Wpedantic -Wconversion -g -O0
LDFLAGS =

OBJ     = dim.o
BINS    = test_dim test_read

.PHONY: all test clean fichiers memcheck

all: $(BINS)

# edition de liens 
test_dim:  test_dim.o  $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

test_read: test_read.o $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

# compilation 
%.o: %.c dim.h
	$(CC) $(CFLAGS) -c $< -o $@

# fichiers de test 
fichiers:
	python3 gen_tests.py

# campagne de tests 
test: all fichiers
	@echo ""
	@./test_dim
	@echo ""
	@echo "=== Tests d'integration ReadCell ==="
	@echo ""
	@./test_read tests/t_ok.bin              0
	@./test_read tests/t_nonsquare.bin       0
	@./test_read tests/t_min32.bin           0
	@./test_read tests/t_max100.bin          0
	@./test_read tests/t_zero.bin         -120
	@./test_read tests/t_sous10.bin       -120
	@./test_read tests/t_sur100.bin       -120
	@./test_read tests/t_neg100.bin       -120
	@./test_read tests/t_swap.bin         -120
	@./test_read tests/t_31.bin           -125
	@./test_read tests/t_20.bin           -125
	@./test_read tests/t_manque.bin       -140
	@./test_read tests/t_trop.bin         -145
	@./test_read tests/t_vide.bin         -115
	@./test_read tests/t_tronq.bin        -115
	@./test_read tests/inexistant.bin     -110
	@echo ""
	@echo "=== Campagne terminee ==="

# controle des fuites memoire 
memcheck: all fichiers
	@for f in tests/*.bin; do \
	    valgrind --error-exitcode=9 --leak-check=full \
	             --errors-for-leak-kinds=all -q ./test_read $$f \
	    || echo "  >>> PROBLEME sur $$f"; \
	done

clean:
	rm -f *.o $(BINS)
	rm -rf tests