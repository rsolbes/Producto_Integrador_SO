# Simulador de Gestor de Memoria RAM y Swap

## Información del Proyecto

**Materia:** Sistemas Operativos  
**Institución:** Universidad Autónoma de Tamaulipas  
**Profesor:** Dante Adolfo Muñoz Quintero  
**Semestre:** 7°

## Autores

- Solbes Davalos Rodrigo
- Izaguirre Cortes Emanuel
- Rosales Pereles Denisse Ariadna
- Morales Urrutia Javier Antonio
- Reyes Alejo Emiliano

---

## Descripción General

Este proyecto implementa un **simulador funcional del gestor de memoria RAM y el área de intercambio (Swap)** de un sistema operativo. El simulador permite visualizar cómo un SO asigna recursos, traduce direcciones y maneja situaciones de escasez de memoria en un entorno multiprogramado utilizando un esquema de **Paginación**.

El sistema simula:
- Asignación y liberación de memoria mediante paginación
- Tabla de páginas por proceso
- Translation Lookaside Buffer (TLB)
- Algoritmo de reemplazo de páginas FIFO
- Operaciones de Swapping (intercambio entre RAM y Swap)
- Manejo de fallos de página (page faults)

---

## Características Principales

### 1. Sistema de Paginación

- **Memoria física dividida en marcos** de tamaño fijo (configurable)
- **Memoria lógica dividida en páginas** del mismo tamaño
- **Tabla de páginas por proceso** que mapea páginas lógicas a marcos físicos
- **Bit de validez** para indicar si una página está presente en RAM

### 2. Translation Lookaside Buffer (TLB)

- Cache de alta velocidad para traducciones frecuentes
- Almacena las traducciones página→marco más recientes
- Reduce el tiempo de acceso a memoria
- Implementación con algoritmo de reemplazo LRU para la TLB

### 3. Algoritmo de Reemplazo FIFO

- Selecciona la página más antigua en RAM como víctima
- Utiliza una cola circular para rastrear el orden de llegada
- Simple y predecible

### 4. Swapping (Memoria Virtual)

- **Swap Out:** Mover páginas de RAM a Swap cuando la RAM está llena
- **Swap In:** Traer páginas de Swap a RAM cuando se necesitan
- Manejo automático de fallos de página
- Registro detallado de todas las operaciones de swap

### 5. Gestión de Procesos

- Creación dinámica de procesos con tamaño variable
- Cálculo automático del número de páginas necesarias
- Asignación inteligente en RAM y Swap
- Liberación completa de recursos al terminar

### 6. Visualización Completa

- **Mapa de memoria RAM:** Estado de cada marco (ocupado/libre)
- **Mapa de Swap:** Estado del área de intercambio
- **Tabla de páginas:** Estado de cada página de un proceso
- **Estado del TLB:** Entradas válidas y estadísticas
- **Lista de procesos activos:** Con su información completa

### 7. Métricas de Rendimiento

- **Número total de operaciones de swap**
- **Tiempo promedio de acceso a memoria** (considerando TLB y page faults)
- **Fragmentación interna** (espacio desperdiciado en páginas)
- **Utilización de RAM y Swap** (porcentaje ocupado)
- **Tasa de aciertos/fallos en TLB**
- **Total de fallos de página**

### 8. Sistema de Logs

- Registro automático de todos los eventos del sistema
- Timestamps precisos para cada operación
- Exportación a archivos de texto
- Visualización configurable (últimos N eventos)

---

## Configuración del Sistema

El simulador se configura mediante el archivo `config.ini`:

```ini
[MEMORIA]
RAM_SIZE = 2048      # Tamaño de RAM en KB
SWAP_SIZE = 4096     # Tamaño de Swap en KB
PAGE_SIZE = 256      # Tamaño de página/marco en KB

[TLB]
TLB_SIZE = 4         # Número de entradas en TLB

[SISTEMA]
MAX_PROCESSES = 50   # Máximo de procesos simultáneos
REPLACEMENT_ALGORITHM = FIFO
VERBOSE_LOGS = 1     # Logs detallados
```

### Valores por Defecto

- **RAM:** 2048 KB (8 marcos de 256 KB)
- **Swap:** 4096 KB (16 marcos de 256 KB)
- **Página/Marco:** 256 KB
- **TLB:** 4 entradas

---

## Compilación y Ejecución

### Requisitos

- Compilador GCC
- Sistema operativo Unix/Linux o Windows con MinGW
- Make (opcional, pero recomendado)

### Compilar con Make

```bash
make
```

### Compilar manualmente

```bash
gcc -Wall -Wextra -std=c99 -O2 -o memory_simulator memory_simulator.c
```

### Ejecutar

```bash
./memory_simulator
```

### Ejecutar directamente con Make

```bash
make run
```

### Limpiar archivos generados

```bash
make clean
```

---

## Guía de Uso

### Menú Principal

```
╔════════════════════════════════════════════════════════════╗
║        SIMULADOR DE GESTOR DE MEMORIA RAM Y SWAP          ║
╠════════════════════════════════════════════════════════════╣
║  GESTIÓN DE PROCESOS:                                      ║
║   1. Crear nuevo proceso                                   ║
║   2. Terminar proceso                                      ║
║   3. Listar procesos activos                               ║
║                                                            ║
║  VISUALIZACIÓN DE MEMORIA:                                 ║
║   4. Mostrar mapa de memoria (RAM y Swap)                  ║
║   5. Mostrar tabla de páginas de un proceso                ║
║   6. Mostrar estado de la TLB                              ║
║                                                            ║
║  OPERACIONES DE MEMORIA:                                   ║
║   7. Simular acceso a página (swap in si es necesario)     ║
║                                                            ║
║  INFORMACIÓN Y ESTADÍSTICAS:                               ║
║   8. Ver estadísticas del sistema                          ║
║   9. Ver registro de eventos (logs)                        ║
║  10. Guardar logs en archivo                               ║
║                                                            ║
║  11. Salir                                                 ║
╚════════════════════════════════════════════════════════════╝
```

### 1. Crear Proceso

Al seleccionar la opción 1, se solicita:
- **Nombre del proceso:** Identificador alfanumérico
- **Tamaño en KB:** Cantidad de memoria que necesita

El sistema automáticamente:
1. Calcula el número de páginas necesarias
2. Asigna páginas en RAM (si hay espacio)
3. Asigna páginas restantes en Swap
4. Crea la tabla de páginas del proceso
5. Actualiza el TLB

**Ejemplo:**
```
Ingrese el nombre del proceso: Editor
Ingrese el tamaño del proceso (KB): 600

✓ Proceso creado exitosamente:
  PID: 1
  Nombre: Editor
  Tamaño: 600 KB
  Páginas: 3 (Tamaño de página: 256 KB)
  Páginas en RAM: 3
  Páginas en Swap: 0
```

### 2. Terminar Proceso

Libera todos los recursos de un proceso:
1. Libera marcos en RAM
2. Libera marcos en Swap
3. Invalida entradas de TLB
4. Elimina el PCB del proceso

### 3. Listar Procesos Activos

Muestra una tabla con todos los procesos activos:
- PID
- Nombre
- Tamaño
- Número de páginas
- Estado (ACTIVO, SUSPENDIDO, INTERCAMBIADO)

### 4. Mapa de Memoria

Visualiza el estado completo de la memoria:

**RAM:**
```
[Marco  0] Proceso 1, Página 0
[Marco  1] Proceso 1, Página 1
[Marco  2] [LIBRE]
[Marco  3] Proceso 2, Página 0
...
Total marcos RAM: 8 | Libres: 2 | Ocupados: 6
Utilización: 75.00%
```

**Swap:**
```
[Swap  0] Proceso 3, Página 2
[Swap  1] [LIBRE]
...
Total marcos Swap: 16 | Libres: 14 | Ocupados: 2
Utilización: 12.50%
```

### 5. Tabla de Páginas

Muestra la tabla de páginas de un proceso específico:

```
Página    Estado        Marco RAM    Válido    Swap Pos
----------------------------------------------------------------
0         EN RAM        2            Sí        -
1         EN RAM        5            Sí        -
2         EN SWAP       -            No        3
```

### 6. Estado del TLB

Visualiza el contenido del Translation Lookaside Buffer:

```
Entrada    PID      Página       Marco RAM    Válido
------------------------------------------------------------
0          1        0            2            Sí
1          1        1            5            Sí
2          2        0            3            Sí
3          -        -            -            No

Estadísticas TLB:
Aciertos (hits): 45
Fallos (misses): 12
Tasa de aciertos: 78.95%
```

### 7. Simular Acceso a Página

Simula el acceso a una página específica de un proceso:

**Escenario 1: TLB Hit**
```
✓ TLB HIT: Página encontrada en TLB (Marco 2)
  Acceso directo a memoria física.
```

**Escenario 2: TLB Miss + Página en RAM**
```
✗ TLB MISS: Página no encontrada en TLB
  Consultando tabla de páginas...

✓ Página encontrada en RAM (Marco 5)
  Actualizando TLB...
```

**Escenario 3: Page Fault (Página en Swap)**
```
✗ TLB MISS: Página no encontrada en TLB
  Consultando tabla de páginas...

✗ PAGE FAULT: Página en Swap (posición 3)
  Iniciando swap in...

SWAP OUT: Proceso 2, Página 1 movida de RAM[4] a Swap[7]
✓ Swap in completado exitosamente.
  Página ahora en RAM (Marco 4)
  TLB actualizada.
```

### 8. Estadísticas del Sistema

Muestra métricas completas de rendimiento:

```
MÉTRICAS DE RENDIMIENTO:

Total de fallos de página:                15
Total de operaciones de swap:             8
Tiempo promedio de acceso:                125.50 ns
Fragmentación interna:                    192 KB

UTILIZACIÓN DE MEMORIA:

Utilización de RAM:                       87.50%
Utilización de Swap:                      25.00%

TLB:

Aciertos en TLB:                         45
Fallos en TLB:                           12
Tasa de aciertos en TLB:                  78.95%

PROCESOS:

Procesos activos:                         3
Tiempo de ejecución:                      125 segundos
```

### 9. Ver Registro de Eventos

Muestra los últimos eventos del sistema:

```
[14:30:25] Sistema de memoria inicializado correctamente
[14:30:32] Proceso creado: PID=1, Nombre='Editor', Tamaño=600 KB, Páginas=3
[14:30:45] Proceso creado: PID=2, Nombre='Navegador', Tamaño=1200 KB, Páginas=5
[14:31:02] SWAP OUT: Proceso 1, Página 2 movida de RAM[7] a Swap[0]
[14:31:15] Acceso a memoria: Proceso 2, Página 1 - TLB HIT (Marco 5)
```

### 10. Guardar Logs

Exporta automáticamente todos los logs a un archivo con timestamp:
```
memory_simulator_log_20241202_143525.txt
```

---

## Ejemplo de Flujo de Trabajo Completo

```bash
# 1. Compilar
make

# 2. Ejecutar
./memory_simulator

# 3. Crear procesos
Opción: 1
Nombre: Editor
Tamaño: 600 KB

Opción: 1
Nombre: Navegador
Tamaño: 1500 KB

Opción: 1
Nombre: Terminal
Tamaño: 400 KB

# 4. Ver estado del sistema
Opción: 3  # Listar procesos
Opción: 4  # Ver mapa de memoria

# 5. Ver tabla de páginas de un proceso
Opción: 5
PID: 1

# 6. Simular accesos a memoria
Opción: 7
PID: 2
Página: 3

# 7. Ver estadísticas
Opción: 8

# 8. Ver TLB
Opción: 6

# 9. Ver logs
Opción: 9
Eventos: 20

# 10. Terminar un proceso
Opción: 2
PID: 1

# 11. Guardar logs y salir
Opción: 10
Opción: 11
```

---

## Estructura del Proyecto

```
memory-simulator/
├── memory_simulator.c     # Código fuente principal
├── config.ini             # Configuración del sistema
├── Makefile              # Script de compilación
├── README.md             # Este archivo
├── docs/                 # Documentación
│   ├── manual_tecnico.pdf
│   ├── manual_usuario.pdf
│   └── documento_pruebas.pdf
└── tests/                # Pruebas y logs
    └── capturas/         # Capturas de pantalla
```

---

## Algoritmos Implementados

### Algoritmo de Paginación

```
1. Al crear proceso:
   - Calcular num_pages = ⌈size / PAGE_SIZE⌉
   - Para cada página:
       Si hay marco libre en RAM:
           Asignar en RAM
           Actualizar tabla de páginas
           Agregar a cola FIFO
       Si no:
           Asignar en Swap
   
2. Al acceder a página:
   - Buscar en TLB
   - Si TLB hit: acceso directo
   - Si TLB miss:
       Buscar en tabla de páginas
       Si página en RAM: actualizar TLB
       Si página en Swap: PAGE FAULT
           Llamar swap_in()
```

### Algoritmo FIFO para Reemplazo

```
1. Mantener cola circular de marcos
2. Al asignar marco en RAM:
   - Agregar al final de la cola
3. Al necesitar marco (RAM llena):
   - Seleccionar marco al frente de la cola
   - swap_out(marco_víctima)
   - Reusar el marco liberado
   - Agregar nuevo marco al final
```

### Algoritmo de Swap Out

```
swap_out(frame_index):
1. Obtener proceso y página del marco
2. Buscar marco libre en Swap
3. Si no hay espacio en Swap: ERROR
4. Copiar página de RAM a Swap
5. Actualizar tabla de páginas:
   - estado = PAGE_IN_SWAP
   - valid = false
   - swap_position = marco_swap
6. Liberar marco en RAM
7. Invalidar entrada en TLB
8. Registrar operación en logs
```

### Algoritmo de Swap In

```
swap_in(pid, page_number):
1. Verificar que página esté en Swap
2. Buscar marco libre en RAM
3. Si RAM llena:
   - Seleccionar víctima con FIFO
   - swap_out(víctima)
4. Copiar página de Swap a RAM
5. Actualizar tabla de páginas:
   - estado = PAGE_IN_RAM
   - valid = true
   - frame_number = marco_ram
6. Liberar marco en Swap
7. Actualizar TLB
8. Incrementar page_faults
9. Registrar operación en logs
```

---

## Conceptos Teóricos Implementados

### Paginación

División de la memoria en bloques de tamaño fijo (marcos en memoria física, páginas en memoria lógica) que permite:
- Eliminación de fragmentación externa
- Asignación no contigua de memoria
- Compartición de memoria
- Protección de memoria

### Tabla de Páginas

Estructura de datos que mantiene la correspondencia entre páginas lógicas y marcos físicos:
- Una tabla por proceso
- Entrada por cada página
- Contiene: número de marco, bit de validez, bit de modificación, etc.

### TLB (Translation Lookaside Buffer)

Cache asociativa de alta velocidad que almacena traducciones recientes:
- Reduce el tiempo de acceso a memoria
- Hit: traducción encontrada (rápido)
- Miss: consultar tabla de páginas (más lento)

### Memoria Virtual y Swapping

Técnica que permite ejecutar procesos más grandes que la memoria física:
- Swap Out: mover páginas a disco
- Swap In: traer páginas de disco
- Page Fault: interrupción cuando se accede a página no presente

### Algoritmos de Reemplazo

Estrategias para seleccionar la página víctima cuando la memoria está llena:
- **FIFO:** La página más antigua
- LRU: La menos recientemente usada
- Óptimo: La que se usará más tarde

---

## Limitaciones y Consideraciones

1. **Máximo de procesos:** 50 simultáneos
2. **Máximo de logs:** 1000 entradas
3. **Simulación:** No es tiempo real, es paso a paso manual
4. **Sin procesos concurrentes:** Un proceso a la vez
5. **Sin protección de memoria:** Simplificado para fines educativos
6. **TLB simplificado:** Implementación básica con LRU

## Referencias y Recursos

- Silberschatz, Galvin, Gagne. *Operating System Concepts*. 10th Edition.
- Tanenbaum, Bos. *Modern Operating Systems*. 4th Edition.
- Stallings, William. *Operating Systems: Internals and Design Principles*. 9th Edition.

---

## Licencia

Este proyecto es de código abierto con fines educativos.