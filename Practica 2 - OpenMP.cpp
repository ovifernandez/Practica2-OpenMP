
#include <iostream>  
#include <cmath> 
#include <iomanip>  
#include <omp.h>   
#include <cstdlib>   
#include <string>

double funcion_x(double x) {
	return 4.0 / (1.0 + x * x);
}


	//Nos piden calcular el area debajo de la curva de f(x) desde x=0 hasta x=1.
	//Sin embargo, f(x) no es ni un rectangulo ni un cuadrado, es una curva, por lo que para saber el area exacta, utilizamos el metodo del trapecio con subdivisiones
			//Consiste en cortar en rebanadas la curva en N subdivisiones.
			//Una rebanada tendrá la forma de un trapecio. Como cada rebanada es muy fina, asumimos que la parte de arriba es una recta (aunque sea una curva)
	//El proceso será el siguiente:
			//1. Dividimos en N rebanadas
			//2. Calculamos areas de cada uno de los trapecios
			//3. Sumamos estas mini areas para tener el area total aproximada.
	//Cuanto mayor sea N, mayor precisión tendremos a la curva real, ya que casda rebanada tendra una parte de arriba mas pequeña y mas parecida a una recta real.
int main(int argc, char** argv) {
	long long N = 10000000;

	if (argc > 1) {
		N = std::stoll(std::string(argv[1]));
	}

	std::cout << "---PRACTICA 2 OPENMP---\n" << std::endl;
	std::cout << "(Usando maximo: " << omp_get_max_threads() << " hilos.)" << std::endl;

	std::cout << "-- PARTE 1.1 -- Estimacion de PI --  Version Secuencial\n" << std::endl;
	
	double h = 1.0 / static_cast<double>(N);
	double alturas_secuencial = 0.0;
	double t0_sec, t1_sec, t_secuencial;

	std::cout << "Calculando Secuencial con un hilo...\n" << std::endl;

	t0_sec = omp_get_wtime();

	//Pasa algo muy curioso. El area de un trapecio es Altura por (Base Mayor mas Base Menor / 2). 
		//Nuestro trapecio esta de "pie", lo que significa que:
			//Altura: Ancho, llamado h. Caluclado facilmente, al dividir 1 (ancho total) entre el nº de trapecios (N)
			//Base Menor: nos lo da f(x izquierdo), lo que seria la primera altura que nos encontramos si vamos de izq->der
			//Base mayor: nos lo da f(x derecho).
	//Si nos damos cuenta, los f(x der) de cada trapecio van a ser los f(x izq) del siguiente trapecio contiguo, por lo que nos podemos ahorrar bastante calculo.
	//Sin embargo, el primer y ultimo trapecio son los unicos que tienen que calcular ambas de sus alturas, ya que su f(x izq) [del 1ero] y su f(x der) [del ultimo] no se comparten con ninguno otro
	alturas_secuencial = funcion_x(0.0) + funcion_x(1.0);

	//Por otro lado, la suma de las areas es = (Ancho trapecio * (B + b / 2) + (Ancho trap * (B + b/2) + ...)
	//Si sacamos Ancho (h) y /2 como factor comun,, nos quedaria = h/2 * ((B + b) + (B + b) + ...)
	for (long long i = 1; i < N; i++) {
		double x = h * static_cast<double>(i);
		//Aprovechando lo que hemos mencionado de que se comparten alturas izq y der, se calcula una altura f(x) y se multiplica por dos, para pasar a la siguiente y  no hacer doble calculo.
		alturas_secuencial += 2.0 * funcion_x(x);
	}

	double pi_secuencial = (h / 2.0) * alturas_secuencial;

	t1_sec = omp_get_wtime();
	t_secuencial = (t1_sec - t0_sec) * 1000; //En ms.

	std::cout << "Secuencialmente: \n\tPI = " << std::setprecision(10) << pi_secuencial
		<< "\n\tTiempo secuencial = " << t_secuencial << "ms\n\n" << std::endl;


	std::cout << "-- PARTE 1.2 -- Estimacion de PI --  Version Paralela\n" << std::endl;

	
	double alturas_paralelo = 0.0;
	double t0_par, t1_par, t_paralelo;
	
	std::cout << "Calculando Paralelamente...\n" << std::endl;

	t0_par = omp_get_wtime();

	//Parte 100% secuencial, lo hace el hilo maestro.
	alturas_paralelo = funcion_x(0.0) + funcion_x(1.0);

	//Ahora paralelizamos. Cada hilo sumará su parte en acumuladores privados, tal y como se indica en (reduction+:alturas_paralelo)
	//La magia de OpenMP es que al final coge y suma todos los resultados de los acumuladores privados de cada hilo.
	#pragma omp parallel for reduction(+:alturas_paralelo)
	for (long long i = 1; i < N; i++) {
		double x = h * static_cast<double>(i);
		alturas_paralelo += 2.0 * funcion_x(x);
	}

	//Parte secuencial obligada, hecha por el master, ya que no hace falta paralelizar.
	double pi_paralelo = (h / 2.0) * alturas_paralelo;

	t1_par = omp_get_wtime();

	t_paralelo = (t1_par - t0_par) * 1000; //En ms
	
	std::cout << "\nParalelo: PI = " << std::setprecision(10) << pi_paralelo
		<< ", Tiempo = " << t_paralelo << " ms\n" << std::endl;
	

	// 5. Cálculo del Speedup
	// ---
	double speedup = t_secuencial / t_paralelo;
	std::cout << "\n\n------------------------------------" << std::endl;
	std::cout << "Speedup (T_seq / T_par): " << speedup << "x" << std::endl;

	//En este caso, tras ejecutar, vemos como la funcion a ejecutar solo consta de una multiplicacion, una suma y una division, 
	// Por lo que el tiempo secuencial es más rapido.
	//El paralelo pierde eficiencia al tener que crear 16 hilos, distribuirles a cada hilo su curro, y hacer que se comuniquen (overhead).

	//Por ello, en tareas con una funcion a ejecutar tan sencilla, no veremos mejoria, de hecho, incluso perderemos eficiencia.
	return 0;
}
