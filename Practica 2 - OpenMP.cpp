
#include <iostream>  
#include <cmath> 
#include <iomanip>  
#include <omp.h>   
#include <cstdlib>   
#include <string>
#include <vector>

//Funcion para la Parte 1, estimacion de PI
double funcion_x(double x) {
	return 4.0 / (1.0 + x * x);
}

//Función pesada, para la Parte 2.
double funcion_x_heavy(double x) {
	// La función que te da el enunciado
	return exp(-x) * pow(sin(10.0 * x), 2.0);
}

void Parte1(long long N);
void Parte2(long long N);


int main(int argc, char** argv) {
	long long N = 10000000;

	if (argc > 1) {
		N = std::stoll(std::string(argv[1]));
	}

	std::cout << "---PRACTICA 2 OPENMP---\n" << std::endl;

	Parte1(N);
	Parte2(N);
	//Parte3(N);

return 0;
}



void Parte1(long long N) {

	std::cout << "(Usando maximo: " << omp_get_max_threads() << " hilos.)" << std::endl;
	//Nos piden calcular el area debajo de la curva de f(x) desde x=0 hasta x=1.
	//Sin embargo, f(x) no es ni un rectangulo ni un cuadrado, es una curva, por lo que para saber el area exacta, utilizamos el metodo del trapecio con subdivisiones
			//Consiste en cortar en rebanadas la curva en N subdivisiones.
			//Una rebanada tendrá la forma de un trapecio. Como cada rebanada es muy fina, asumimos que la parte de arriba es una recta (aunque sea una curva)
	//El proceso será el siguiente:
			//1. Dividimos en N rebanadas
			//2. Calculamos areas de cada uno de los trapecios
			//3. Sumamos estas mini areas para tener el area total aproximada.
	//Cuanto mayor sea N, mayor precisión tendremos a la curva real, ya que casda rebanada tendra una parte de arriba mas pequeña y mas parecida a una recta real.

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

}



void Parte2(long long N) {
	//La función pesada es casi identica a la de la parte 1, pero añadiendo oscilaciones pequeñas, sin ser una funcion suave o predecible
	//Esta parte trata de averiguar con que tipo de schedule conseguiremos mayor rendimiento:
		//Static es para cuando las cargas son iguales en todos los hilos.
		//Dynamic es para cuando los hilos no tienen cargas iguales.
		//Guided es para cargas no uniformes igual que dynamic, pero de una manera mas eficiente.
	//Para conocer la respuesta, podemos acudir a lo que hay dentro del for. 
		//exp(), sin(), pow(), y multiplicaciones para cada i.
		//Lo logico es pensar que como cada i tiene que realizar las mismas operaciones, lo más optimos erá static, dado que se supone una carga "balanceada".
		//Si hubiese, por ejemplo, zonas de x en las que no existiese la gráfica, en ese x en concreto no habría que calcular nada, por lo que el reparto de carga no sería el mismo para todo X.
		//Como no es el caso, podriamos caer en la trampa de pensar que static seria lo mejor
		// 
		//Sin embargo, si nos metemos a nivel matemático y gráfico.
		//Lo primero es darse cuenta que los picos altos de la mitad izquierda son mucho más altos que los de la derecha.
			//Por ello, los hilos que trabajen con la mitad izquierda operarán con numeros más grandes, y viceversa
		//Lo segundo es darse cuenta de las minas [cuando e^-x * sen^2(10x) es casi 0]. Estos puntos se llaman denormales, los que estan muy cerca de 0, pero no son 0.
		//Para identificar los puntos donde encontraremos numeros denormales, basta con igualar f(x) = 0, y ver que hay 4 puntos:
			//x = 0, x = pi/10, x = 2pi/10 y x = 3pi/10. Las llamaremos minas
		//En terminos de CPU, este tipo de numeros le cuesta infinitamente más que los demas numeros normales.
		//Static en este caso divide 
	std::cout << "\n\n-- PARTE 2.1 -- Funcion pesada y Squeduling\n" << std::endl;
	
	double h = 1.0 / static_cast<double>(N);
	double t_secuencial;

	std::cout << "Calculando Secuencial (1 Hilo) para f_heavy(x)..." << std::endl;
	//Mismo proceso que en la Parte 1 sustituyendolo por la f(x) heavy.
	double t0_sec = omp_get_wtime();
	double t1_sec;
	double alturas_secuencial = funcion_x_heavy(0.0) + funcion_x_heavy(1.0);

	for (long long i = 1; i < N; i++) {
		double x = h * static_cast<double>(i);
		alturas_secuencial += 2.0 * funcion_x_heavy(x);
	}
	double integral_secuencial = (h / 2.0) * alturas_secuencial;
	t1_sec = omp_get_wtime();
	t_secuencial = (t1_sec - t0_sec) * 1000.0; //En ms
	std::cout << "  (Resultado Secuencial = " << std::setprecision(10) << integral_secuencial << ")" << std::endl;

	std::cout << "\nAnalisis de Scheduling (Tiempos en ms, Eficiencia = Speedup/hilos)" << std::endl;
	
	//Queremos cortar el trabajo en paquetes, cada uno con tamaño dinamico chunk. Cada hilo coge un paquete, y cuando acaba, se pone a la cola de los hilos listos para coger otro chunk
		//El chunk no puede ser ni demasiado pequeño ni demasiado grande:
			//Si chunk es muy pequeño, tendras a los hios todo el rato eejcutando rapidisimo su chunk y preguntando por mas trabajo, sumando el tiempo de comunicacion muchisimo.
			//Si es demasiado grande, perderás la gracia de dynamic, y te pareceras mas a static.
		//Por ello, hemos pensado nuestro chunk como  un equilibrio, cogiendo lo que le deberia tocar a cada hilo en statico, y dividiendolo en 10 trozos. Por ello, si tenemos como max 16 hilos, a cada hilo le tocarán 10 paquetes de trabajo, en vez de 1, como en static.
		//Sin embargo, nos aprovechamos de dynamic, ya que si un hilo acaba antes que otro, puede coger el siguiente paquete de trabajo que quede, sin tener que esperar al que de verdad le tocaba ese paquete.
		//Claro, esto funcionaria genial si la carga estuviese desproporcionada, cosa que creemos que no ocurre en este caso.
	int chunk = N / (omp_get_max_threads() * 10);
	//Un chunk no puede ser de 0, asique nos guardamos las espaldas poniendolos a 1 si se da el caso.
	if (chunk == 0) {
		chunk = 1;
	}

	std::cout << "Hilos\tT_Static(ms)\tT_Dynamic(ms)\tT_Guided(ms)\tE_Static\tE_Dynamic\tE_Guided" << std::endl;	std::cout << "----------------------------------------------------------------------------------" << std::endl;

	std::vector<int> hilos_a_probar = {1, 2, 4, 8, 16 }; //Con 1 hilo lo hice antes, en la version secuencial arriba.

	for (int p : hilos_a_probar) {
		if (p == 1) {
			std::cout << "1\t" << std::fixed << std::setprecision(4) << t_secuencial << " (Eficiencia: 1.00)" << std::endl;
			continue;
		}

		omp_set_num_threads(p);

		double t0_static = omp_get_wtime();
		double t1_static;
		double alturas_static = funcion_x_heavy(0.0) + funcion_x_heavy(1.0);
#pragma omp parallel for reduction(+:alturas_static) schedule(static)
		for (long long i = 1; i < N; ++i) {
			double x = h * static_cast<double>(i);
			alturas_static += 2.0 * funcion_x_heavy(x);
		}
		double integral_static = (h / 2.0) * alturas_static;
		t1_static = omp_get_wtime();
		double t_static = (t1_static - t0_static) * 1000.0;
		double speedup_static = t_secuencial / t_static;
		double eficiencia_static = speedup_static / p;


		double t0_dynamic = omp_get_wtime();
		double t1_dynamic;
		double alturas_dynamic = funcion_x_heavy(0.0) + funcion_x_heavy(1.0);
#pragma omp parallel for reduction(+:alturas_dynamic) schedule(dynamic, chunk)
		for (long long i = 1; i < N; ++i) {
			double x = h * static_cast<double>(i);
			alturas_dynamic += 2.0 * funcion_x_heavy(x);
		}
		double integral_dynamic = (h / 2.0) * alturas_dynamic; 
		t1_dynamic = omp_get_wtime();
		double t_dynamic = (t1_dynamic - t0_dynamic) * 1000.0;
		double speedup_dynamic = t_secuencial / t_dynamic;
		double eficiencia_dynamic = speedup_dynamic / p;

		double t0_guided = omp_get_wtime();
		double t1_guided;
		double alturas_guided = funcion_x_heavy(0.0) + funcion_x_heavy(1.0);
#pragma omp parallel for reduction(+:alturas_guided) schedule(guided)
		for (long long i = 1; i < N; ++i) {
			double x = h * static_cast<double>(i);
			alturas_guided += 2.0 * funcion_x_heavy(x);
		}
		double integral_guided = (h / 2.0) * alturas_guided;
		t1_guided = omp_get_wtime();
		double t_guided = (t1_guided - t0_guided) * 1000.0;
		double speedup_guided = t_secuencial / t_guided;
		double eficiencia_guided = speedup_guided / p;

		if (integral_static + integral_dynamic + integral_guided == -2) {
			std::cout << "Esto nunca se imprime, solo es para que el compilador no tire por tierra las variables sin hacer el bucle" << std::endl;
		}

		std::cout << p << "\t"
			<< std::fixed << std::setprecision(4) << t_static << "\t"
			<< std::fixed << std::setprecision(4) << t_dynamic << "\t"
			<< std::fixed << std::setprecision(4) << t_guided << "\t"
			<< std::fixed << std::setprecision(2) << eficiencia_static << "\t\t"
			<< std::fixed << std::setprecision(2) << eficiencia_dynamic << "\t\t"
			<< std::fixed << std::setprecision(2) << eficiencia_guided
			<< std::endl;
	}
}
