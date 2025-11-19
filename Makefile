# Compilador a usar (g++ es el estándar en Linux/HPC)
CXX = g++

# Flags (Opciones) de compilación:
# -std=c++17 : Usar estándar C++ moderno
# -O3        : Nivel máximo de optimización (Vital para HPC)
# -fopenmp   : ¡CLAVE! Activa el soporte de OpenMP
# -Wall      : Mostrar todas las advertencias (buena práctica)
CXXFLAGS = -std=c++17 -O3 -fopenmp -Wall

# Nombre del ejecutable final
TARGET = Practica2-OpenMP

# Archivo fuente
SRC = main.cpp

# Regla por defecto (lo que pasa si escribes 'make')
all: $(TARGET)

# Cómo crear el ejecutable
$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

# Regla para limpiar archivos generados (comando 'make clean')
clean:
	rm -f $(TARGET)