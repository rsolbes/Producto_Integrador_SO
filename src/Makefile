# Makefile para Simulador de Gestor de Memoria
# Sistemas Operativos - Universidad Autónoma de Tamaulipas

# Compilador y flags base
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
LDFLAGS =

# Detección del sistema operativo
ifeq ($(OS),Windows_NT)
    DETECTED_OS := Windows
    TARGET = simulador_memoria.exe
    RM = del /Q
    RMDIR = rmdir /S /Q
    MKDIR = mkdir
    SEP = \\
    CLEAN_FILES = $(TARGET) *.o *.log *.txt
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        DETECTED_OS := Linux
        TARGET = simulador_memoria
        RM = rm -f
        RMDIR = rm -rf
        MKDIR = mkdir -p
        SEP = /
        CLEAN_FILES = $(TARGET) *.o *.log *.txt
    endif
    ifeq ($(UNAME_S),Darwin)
        DETECTED_OS := macOS
        TARGET = simulador_memoria
        RM = rm -f
        RMDIR = rm -rf
        MKDIR = mkdir -p
        SEP = /
        CLEAN_FILES = $(TARGET) *.o *.log *.txt
    endif
endif

# Archivos
SRC = simulador_memoria.c
OBJ = $(SRC:.c=.o)

# Directorios
SRCDIR = src
DOCSDIR = docs
TESTSDIR = tests

# Colores para output (solo en Unix-like)
ifneq ($(DETECTED_OS),Windows)
    RED = \033[0;31m
    GREEN = \033[0;32m
    YELLOW = \033[1;33m
    BLUE = \033[0;34m
    NC = \033[0m
else
    RED =
    GREEN =
    YELLOW =
    BLUE =
    NC =
endif

# Regla principal
all: detect $(TARGET)
	@echo "$(GREEN)✓ Compilación exitosa$(NC)"
	@echo "$(GREEN)✓ Sistema: $(DETECTED_OS)$(NC)"
	@echo "$(GREEN)✓ Ejecutable: $(TARGET)$(NC)"

# Detectar sistema operativo
detect:
	@echo "$(BLUE)═══════════════════════════════════════════$(NC)"
	@echo "$(BLUE)  Simulador de Gestor de Memoria RAM/Swap$(NC)"
	@echo "$(BLUE)  Universidad Autónoma de Tamaulipas$(NC)"
	@echo "$(BLUE)═══════════════════════════════════════════$(NC)"
	@echo "$(YELLOW)Sistema detectado: $(DETECTED_OS)$(NC)"
	@echo ""

# Compilar el ejecutable
$(TARGET): $(SRC)
	@echo "$(YELLOW)Compilando $(TARGET) para $(DETECTED_OS)...$(NC)"
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

# Compilar objeto (si se necesita)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Limpiar archivos generados
clean:
	@echo "$(YELLOW)Limpiando archivos en $(DETECTED_OS)...$(NC)"
ifeq ($(DETECTED_OS),Windows)
	-$(RM) $(TARGET) 2>nul
	-$(RM) *.o 2>nul
	-$(RM) *.log 2>nul
	-$(RM) *.txt 2>nul
else
	$(RM) $(CLEAN_FILES)
endif
	@echo "$(GREEN)✓ Limpieza completa$(NC)"

# Ejecutar el simulador
run: $(TARGET)
	@echo "$(GREEN)Ejecutando simulador en $(DETECTED_OS)...$(NC)"
	@echo ""
ifeq ($(DETECTED_OS),Windows)
	$(TARGET)
else
	./$(TARGET)
endif

# Crear estructura de directorios
dirs:
	@echo "$(YELLOW)Creando estructura de directorios...$(NC)"
ifeq ($(DETECTED_OS),Windows)
	-$(MKDIR) $(SRCDIR) 2>nul
	-$(MKDIR) $(DOCSDIR) 2>nul
	-$(MKDIR) $(TESTSDIR) 2>nul
else
	$(MKDIR) $(SRCDIR) $(DOCSDIR) $(TESTSDIR)
endif
	@echo "$(GREEN)✓ Directorios creados$(NC)"

# Mostrar información del sistema
info:
	@echo "$(BLUE)═══════════════════════════════════════════$(NC)"
	@echo "$(BLUE)  Información de Compilación$(NC)"
	@echo "$(BLUE)═══════════════════════════════════════════$(NC)"
	@echo "$(YELLOW)Sistema Operativo:$(NC) $(DETECTED_OS)"
	@echo "$(YELLOW)Compilador:$(NC)        $(CC)"
	@echo "$(YELLOW)Flags:$(NC)             $(CFLAGS)"
	@echo "$(YELLOW)Target:$(NC)            $(TARGET)"
	@echo "$(YELLOW)Fuente:$(NC)            $(SRC)"
	@echo "$(BLUE)═══════════════════════════════════════════$(NC)"

# Mostrar ayuda
help:
	@echo "$(BLUE)═══════════════════════════════════════════$(NC)"
	@echo "$(BLUE)  Comandos Disponibles$(NC)"
	@echo "$(BLUE)═══════════════════════════════════════════$(NC)"
	@echo "$(YELLOW)  make$(NC)          - Compilar el simulador"
	@echo "$(YELLOW)  make run$(NC)      - Compilar y ejecutar"
	@echo "$(YELLOW)  make clean$(NC)    - Eliminar archivos generados"
	@echo "$(YELLOW)  make dirs$(NC)     - Crear estructura de directorios"
	@echo "$(YELLOW)  make info$(NC)     - Mostrar información del sistema"
	@echo "$(YELLOW)  make help$(NC)     - Mostrar esta ayuda"
ifeq ($(DETECTED_OS),Linux)
	@echo "$(YELLOW)  make install$(NC)  - Instalar en /usr/local/bin"
	@echo "$(YELLOW)  make uninstall$(NC) - Desinstalar"
endif
	@echo "$(BLUE)═══════════════════════════════════════════$(NC)"
	@echo ""
	@echo "$(GREEN)Sistema actual: $(DETECTED_OS)$(NC)"

# Instalar (solo Linux/macOS)
ifneq ($(DETECTED_OS),Windows)
install: $(TARGET)
	@echo "$(YELLOW)Instalando $(TARGET) en /usr/local/bin...$(NC)"
	install -m 755 $(TARGET) /usr/local/bin/
	@echo "$(GREEN)✓ Instalación completa$(NC)"

# Desinstalar (solo Linux/macOS)
uninstall:
	@echo "$(YELLOW)Desinstalando $(TARGET)...$(NC)"
	$(RM) /usr/local/bin/$(TARGET)
	@echo "$(GREEN)✓ Desinstalación completa$(NC)"
endif

# Compilación con debug
debug: CFLAGS += -g -DDEBUG
debug: clean $(TARGET)
	@echo "$(GREEN)✓ Compilado con símbolos de debug$(NC)"

# Compilación optimizada
release: CFLAGS += -O3 -DNDEBUG
release: clean $(TARGET)
	@echo "$(GREEN)✓ Compilado en modo release$(NC)"

.PHONY: all clean run dirs help info detect debug release
ifneq ($(DETECTED_OS),Windows)
.PHONY: install uninstall
endif