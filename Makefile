# Tiko editor - Linux build
FBC      ?= fbc
DEPS      = deps
SCI_LIB   = $(DEPS)/scintilla/bin/scintilla.a
LEX_LIB   = $(DEPS)/lexilla/bin/liblexilla.a
FBFLAGS  ?= -w all -gen gcc -O 2

all: tiko

SCI_VER   = 553
LEX_VER   = 541

$(DEPS)/scintilla/gtk/makefile:
	mkdir -p $(DEPS)
	cd $(DEPS) && curl -sSLO https://www.scintilla.org/scintilla$(SCI_VER).tgz && tar xzf scintilla$(SCI_VER).tgz
$(DEPS)/lexilla/src/makefile:
	mkdir -p $(DEPS)
	cd $(DEPS) && curl -sSLO https://www.scintilla.org/lexilla$(LEX_VER).tgz && tar xzf lexilla$(LEX_VER).tgz

$(SCI_LIB): $(DEPS)/scintilla/gtk/makefile
	$(MAKE) -C $(DEPS)/scintilla/gtk GTK3=1
$(LEX_LIB): $(DEPS)/lexilla/src/makefile
	$(MAKE) -C $(DEPS)/lexilla/src

tiko: $(SCI_LIB) $(LEX_LIB) src/*.bas src/*.bi src/*.inc src/lnx/*
	$(FBC) $(FBFLAGS) src/tiko.bas -x tiko -a $(SCI_LIB) -a $(LEX_LIB) -l gmodule-2.0 -l pangoft2-1.0 -l fontconfig

clean:
	rm -f tiko

.PHONY: all clean
