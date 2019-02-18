CC = gcc
C = lcp-create
S = lcp-search
R = read_text
LIBS = cmdline.o  ${SOURCEDIR}/sais-lite-2.4.1/sais.o
SAIS = ${SOURCEDIR}/sais-lite-2.4.1
LDFLAGS = -Wall -O3 -lz
SOURCEDIR := .

ifneq "$(MAKECMDGOALS)" "clean"
-include ${SOURCES:.c=.d}
-include ${T_SOURCES:.c=.d}
endif

bin: $(C) $(S)
$(C): $(C).o $(SAIS)/sais.o $(R).o
	$(CC) $(LDFLAGS) -o $@ $^ 

$(S): $(LIBS) $(S).o $(R).o
	$(CC) $(LDFLAGS) -o $@ $^

cleanall:
	@echo "Cleaning all..."
	rm -rf *.o cmdline.* *.bin $(C) $(S)
clean:
	@echo "Cleaning .o"
	rm -rf *.o
.PHONY: clean
.PHONY: cleanll

$(SAIS)/sais.o: $(SAIS)/sais.c $(SAIS)/sais.h
	cd $(SAIS) && make sais.o
cmdline.c cmdline.h: $(S).ggo
	gengetopt -i $< --output-dir=./ 


