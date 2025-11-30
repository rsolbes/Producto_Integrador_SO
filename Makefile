CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
TARGET = simulador_procesos
SOURCE = simulador_procesos.c

# Detectar sistema operativo
ifeq ($(OS),Windows_NT)
    TARGET_EXT = .exe
    RM = del /Q
    RUN = $(TARGET)$(TARGET_EXT)
else
    TARGET_EXT = 
    RM = rm -f
    RUN = ./$(TARGET)
endif

TARGET_FULL = $(TARGET)$(TARGET_EXT)

all: $(TARGET_FULL)

# Compilar el simulador
$(TARGET_FULL): $(SOURCE)
	$(CC) $(CFLAGS) $(SOURCE) -o $(TARGET_FULL)
	@echo "✓ Compilación exitosa"
	@echo "Ejecuta con: $(RUN)"

# Compilar con optimización
release: $(SOURCE)
	$(CC) -O2 -std=c99 $(SOURCE) -o $(TARGET_FULL)
	@echo "✓ Compilación con optimización exitosa"

# Limpiar archivos generados
clean:
ifeq ($(OS),Windows_NT)
	if exist $(TARGET_FULL) $(RM) $(TARGET_FULL)
else
	$(RM) $(TARGET_FULL)
endif
	@echo "✓ Archivos de compilación eliminados"

# Ejecutar el programa
run: $(TARGET_FULL)
	$(RUN)

help:
	@echo "Makefile para Simulador de Gestor de Procesos"
	@echo ""
	@echo "Uso:"
	@echo "  make         - Compilar el simulador"
	@echo "  make release - Compilar con optimización"
	@echo "  make clean   - Limpiar archivos generados"
	@echo "  make run     - Compilar y ejecutar"
	@echo "  make help    - Mostrar esta ayuda"

.PHONY: all clean run help release