ifeq ($(OS),Windows_NT)
    EXE = main.exe
    RM = del /Q
    RUN = .\$(EXE)
else
    EXE = main
    RM = rm -f
    RUN = ./$(EXE)
endif

.PHONY: all run clean

all: $(EXE)

run: $(EXE)
	@$(RUN)

$(EXE): main.c
	gcc -o $(EXE) main.c

clean:
	$(RM) $(EXE)