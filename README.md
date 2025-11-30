# Simulador de Gestor de Procesos para Sistemas Operativos

## Información del Proyecto

**Materia:** Sistemas Operativos  
**Duración:** Semestral  
**Institución:** Universidad Autónoma de Tamaulipas

## Descripción

Simulador básico de un gestor de procesos que implementa los conceptos fundamentales de sistemas operativos, incluyendo:

- Operaciones sobre procesos
- Asignación de recursos
- Algoritmos de planificación
- Comunicación y sincronización de procesos
- Terminación de procesos

## Características Implementadas

### 1. Operaciones sobre Procesos

- **Crear, suspender, reanudar y terminar procesos**
- Cada proceso se representa mediante una estructura PCB (Process Control Block) con:
  - PID único
  - Estado (Listo, Ejecutando, Esperando, Terminado)
  - Prioridad
  - Lista de recursos asignados (CPU, memoria)
  - Tiempos de ejecución y espera

### 2. Asignación de Recursos

- **Recursos del sistema:**
  - 1 CPU
  - 4 GB de memoria (4 bloques de 1GB = 1024MB cada uno)
  
- **Funcionalidades:**
  - Solicitud y liberación de recursos por parte de los procesos
  - Detección y prevención de conflictos (recursos no disponibles)
  - Política de prevención de deadlock: no permitir solicitudes que excedan recursos disponibles

### 3. Algoritmos de Planificación

Se pueden seleccionar al inicio del simulador:

- **FCFS (First Come First Served):** Primero en llegar, primero en ser atendido
- **Round Robin:** Con quantum configurable

### 4. Comunicación y Sincronización

#### Comunicación por Mensajes
- Envío y recepción de mensajes entre procesos
- Mensajes con contenido de texto
- Sistema de entrega confirmada

#### Sincronización con Semáforos
- Creación de semáforos con valor inicial configurable
- Operaciones `wait` y `signal`
- Bloqueo y desbloqueo de procesos según el valor del semáforo
- Demostración del problema productor-consumidor

### 5. Terminación de Procesos

- **Tipos de terminación:**
  - Normal (proceso completa su ejecución)
  - Error
  - Deadlock
  - Terminación por usuario
  
- Registro de causas de terminación
- Liberación automática de recursos al terminar

### 6. Interfaz de Usuario

CLI (Línea de comandos) con menú interactivo que permite:

- Crear procesos manualmente
- Listar procesos activos con su información
- Mostrar recursos disponibles y en uso
- Ejecutar la simulación paso a paso
- Forzar terminación, suspender/reanudar procesos
- Mostrar logs de eventos (cambios de estado, asignación de recursos)
- Ver estadísticas del sistema
- Operaciones de comunicación (mensajes)
- Operaciones de sincronización (semáforos)
- Demostración productor-consumidor

### 7. Sistema de Logs

Registro detallado de eventos:
- Cambios de estado de procesos
- Asignación y liberación de recursos
- Envío y recepción de mensajes
- Operaciones sobre semáforos
- Creación y terminación de procesos

### 8. Estadísticas

El simulador calcula y muestra:

- **Tiempo promedio de espera:** Promedio del tiempo que los procesos pasan esperando
- **Tiempo promedio de retorno (turnaround):** Tiempo total desde llegada hasta finalización
- **Utilización de CPU:** Porcentaje del tiempo que la CPU estuvo ocupada
- **Throughput:** Procesos completados por unidad de tiempo
- Algoritmo de planificación utilizado
- Total de procesos completados

## Compilación y Ejecución

### Compilar

```bash
gcc simulador_procesos.c -o simulador_procesos
```

### Ejecutar

```bash
./simulador_procesos
```

## Guía de Uso

### 1. Inicio del Simulador

Al iniciar, se solicita seleccionar el algoritmo de planificación:

```
1. FCFS (First Come First Served)
2. Round Robin
```

Si selecciona Round Robin, deberá especificar el quantum.

### 2. Crear Procesos

Opción 1 del menú principal. Se solicita:

- **Tiempo de ejecución (burst time):** Unidades de tiempo que necesita el proceso
- **Prioridad:** Número entero (mayor = más prioritario)
- **Bloques de memoria:** Cantidad de bloques de 1GB (máximo 4)

Ejemplo:
```
Tiempo de ejecución: 10
Prioridad: 5
Bloques de memoria: 2
```

### 3. Ejecutar Simulación

Opción 4 del menú: "Ejecutar siguiente paso"

Cada paso avanza el tiempo en 1 unidad y:
- Ejecuta el proceso actual (si hay alguno)
- Actualiza tiempos de espera
- Selecciona el siguiente proceso según el algoritmo
- Muestra el estado actual de la CPU

### 4. Demostración Productor-Consumidor

Opción 15 del menú.

Crea automáticamente:
- 2 procesos productores
- 2 procesos consumidores
- 3 semáforos (mutex, empty, full)
- Buffer compartido de tamaño 5

Use "Ejecutar siguiente paso" para ver la interacción.

### 5. Ver Estadísticas

Opción 9 del menú. Muestra métricas de rendimiento del sistema.

## Ejemplo de Flujo de Trabajo

```
1. Iniciar simulador
2. Seleccionar algoritmo (ej: Round Robin con quantum 3)
3. Crear proceso 1 (burst: 8, prioridad: 5, memoria: 1)
4. Crear proceso 2 (burst: 4, prioridad: 3, memoria: 2)
5. Crear proceso 3 (burst: 6, prioridad: 7, memoria: 1)
6. Listar procesos (opción 2)
7. Mostrar recursos (opción 3)
8. Ejecutar varios pasos (opción 4 repetidamente)
9. Ver logs (opción 8)
10. Ver estadísticas (opción 9)
```

## Estructura del Código

### Estructuras de Datos Principales

- **PCB:** Process Control Block - información de cada proceso
- **Resources:** CPU y memoria
- **Message:** Estructura de mensajes entre procesos
- **Semaphore:** Semáforo con cola de espera
- **SharedBuffer:** Buffer circular para productor-consumidor
- **LogEntry:** Entrada de log con timestamp
- **System:** Estado global del sistema

### Funciones Principales

- `init_system()`: Inicializa el sistema
- `create_process()`: Crea un nuevo proceso
- `execute_step()`: Ejecuta un paso de simulación
- `select_next_process()`: Selecciona siguiente proceso según algoritmo
- `request_resources()` / `release_resources()`: Gestión de recursos
- `send_message()` / `receive_message()`: Comunicación por mensajes
- `create_semaphore()` / `wait_semaphore()` / `signal_semaphore()`: Sincronización
- `show_statistics()`: Muestra estadísticas de rendimiento

## Prevención de Deadlocks

El simulador implementa una política simple de prevención:

- No permite que un proceso solicite recursos si no hay suficientes disponibles
- Esto evita la espera circular y el deadlock
- Los procesos que no pueden obtener recursos inmediatamente pasan a estado WAITING

## Limitaciones

- Máximo 50 procesos simultáneos
- Máximo 100 mensajes
- Máximo 10 semáforos
- Máximo 1000 entradas de log
- Simulación paso a paso (no en tiempo real)

## Integridad Académica

El código es original. Se permiten referencias externas con la debida citación.

## Autor

**Nombres de los estudiantes:**
- Solbes Davalos Rodrigo
- Izaguirre Cortes Emanuel
- Rosales Pereles Denisse Ariadna
- Morales Urrutia Javier Antonio
- Reyes Alejo Emiliano

**Materia:** Sistemas Operativos  
**Profesor:** Dante Adolfo Muñoz Quintero  
**Semestre:** 7o  
**Institución:** Universidad Autónoma de Tamaulipas

---

## Notas Técnicas

### Estados de Proceso

- **READY:** Proceso listo para ejecutar, esperando CPU
- **RUNNING:** Proceso en ejecución
- **WAITING:** Proceso esperando recursos o eventos
- **TERMINATED:** Proceso finalizado

### Algoritmo Round Robin

- Cada proceso recibe un quantum (tiempo de ejecución)
- Si no termina en su quantum, vuelve al final de la cola READY
- El quantum es configurable al inicio

### Sistema de Prioridades

- Números más altos = mayor prioridad
- Sin envejecimiento (starvation posible)
- Útil para procesos críticos