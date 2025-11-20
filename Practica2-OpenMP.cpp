
#include <iostream>  
#include <cmath> 
#include <iomanip>  
#include <omp.h>   
#include <cstdlib>   
#include <string>
#include <vector>
#include <random>

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
void Parte3(long long N);
void TestSincronizacion(long long N);
void Parte3_SinFalseSharing(long long N);

int main(int argc, char** argv) {
	std::cout << "=========================================\n";
	std::cout << "------" << "PRACTICA 2 OPENMP" << "------\n";
	std::cout << "=========================================\n\n";
	long long N = 10000000;

	if (argc > 1) {
		N = std::stoll(std::string(argv[1]));
	}
	Parte1(N);
	Parte2(N);
	Parte3(N);
	TestSincronizacion(N);
	Parte3_SinFalseSharing(N);

return 0;
}



void Parte1(long long N) {
	std::cout << " >>> PARTE 1.1: Calculo de PI (Secuencial)\n";
	std::cout << " " << std::string(40, '-') << "\n";

	//Nos piden calcular el area debajo de la curva de f(x) desde x=0 hasta x=1.
	//Sin embargo, f(x) no es ni un rectangulo ni un cuadrado, es una curva, por lo que para saber el area exacta, utilizamos el metodo del trapecio con subdivisiones
			//Consiste en cortar en rebanadas la curva en N subdivisiones.
			//Una rebanada tendrá la forma de un trapecio. Como cada rebanada es muy fina, asumimos que la parte de arriba es una recta (aunque sea una curva)
	//El proceso será el siguiente:
			//1. Dividimos en N rebanadas
			//2. Calculamos areas de cada uno de los trapecios
			//3. Sumamos estas mini areas para tener el area total aproximada.
	//Cuanto mayor sea N, mayor precisión tendremos a la curva real, ya que casda rebanada tendra una parte de arriba mas pequeña y mas parecida a una recta real.


	double h = 1.0 / static_cast<double>(N);
	double alturas_secuencial = 0.0;
	double t0_sec, t1_sec, t_secuencial;


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

	std::cout << "    [Secuencial] PI     : " << std::fixed << std::setprecision(10) << pi_secuencial << "\n"
		<< "    [Secuencial] Tiempo : " << std::setprecision(4) << t_secuencial << " ms\n\n";


	std::cout << " >>> PARTE 1.2: Calculo de PI (Paralelo)\n";
	std::cout << " " << std::string(40, '-') << "\n";
	std::cout << "    Calculando en paralelo...\n";

	double alturas_paralelo = 0.0;
	double t0_par, t1_par, t_paralelo;


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

	std::cout << "    [Paralelo]   PI     : " << std::fixed << std::setprecision(10) << pi_paralelo << "\n"
		<< "    [Paralelo]   Tiempo : " << std::setprecision(4) << t_paralelo << " ms\n";


	// 5. Cálculo del Speedup
	// ---
	double speedup = t_secuencial / t_paralelo;
	std::cout << "\n" << std::string(50, '.') << "\n";
	std::cout << "    RESULTADO SPEEDUP (T_seq / T_par): "
		<< "\033[1;32m" << speedup << "x" << "\033[0m" 
		<< "\n" << std::string(50, '.') << "\n\n";

	//En este caso, tras ejecutar, vemos como la funcion a ejecutar solo consta de una multiplicacion, una suma y una division, 
	// Por lo que el tiempo secuencial es más rapido.
	//El paralelo pierde eficiencia al tener que crear 16 hilos, distribuirles a cada hilo su curro, y hacer que se comuniquen (overhead).

	//Por ello, en tareas con una funcion a ejecutar tan sencilla, no veremos mejoria, de hecho, incluso perderemos eficiencia.

}



void Parte2(long long N) {
	std::cout << "\n >>> PARTE 2: Analisis de Scheduling (Carga Pesada)\n";
	std::cout << " " << std::string(40, '-') << "\n";
	//La función pesada es casi identica a la de la parte 1, pero añadiendo oscilaciones pequeñas, sin ser una funcion suave o predecible
	//Esta parte trata de averiguar con que tipo de schedule conseguiremos mayor rendimiento:
		//Static es para cuando las cargas son iguales en todos los hilos.
		//Dynamic es para cuando los hilos no tienen cargas iguales.
		//Guided es para cargas no uniformes igual que dynamic, pero de una manera mas eficiente.
	//Para conocer la respuesta, podemos acudir a lo que hay dentro del for. 
		//exp(), sin(), pow(), y multiplicaciones para cada i.
		//Lo logico es pensar que como cada i tiene que realizar las mismas operaciones, lo más optimos erá static, dado que se supone una carga "balanceada".
		//Si hubiese, por ejemplo, zonas de x en las que no existiese la gráfica, en ese x en concreto no habría que calcular nada, por lo que el reparto de carga no sería el mismo para todo X.
		//Como no es el caso, podriamos caer en la trampa de pensar que static seria lo mejor.
		// 
		//Sin embargo, si nos metemos a nivel matemático y gráfico, vemos este aspecto:
		//Lo primero es darse cuenta de las minas [cuando e^-x * sen^2(10x) es casi 0]. Estos puntos se llaman denormales, los que estan muy cerca de 0, pero no son 0.
		//Para identificar los puntos donde encontraremos numeros denormales, basta con igualar f(x) = 0, y ver que hay 4 puntos:
			//x = 0, x = pi/10, x = 2pi/10 y x = 3pi/10. Las llamaremos minas
		//En terminos de CPU, este tipo de numeros le cuesta infinitamente más que los demas numeros normales.
		
		//Entonces, tenemos que aprox 99% de los valores de x son numeros normales, y el 1% estan en esa zona de minas.
		//Ahora, tendriamos que ver que compensa más, si aplicar static con ese tiempo de desbalanceo de carga ocasionado por las minas, o si nos conviene mejor una tecnica dinámica con su tiempo extra de comunicacion de gestion de la pila.
			//Static: genera desbalanceo de carga debido a que en las zonas de minas, el procesador que le toque tardará más tiempo que los demás, y dejara a los otros en standby
			//Dynamic y guided: Corrigen el problema de static, pero le añaden a su tiempo el tiempo de comunicacion de la pila.
	//Por lo tanto, concluimos que la respuesta la tendrán los resultados que veremos más abajo, ya que la cosa esta bastante empatada.
	std::cout << "\n\n-- PARTE 2.1 -- Funcion pesada y Squeduling\n" << std::endl;
	
	double h = 1.0 / static_cast<double>(N);
	double t_secuencial;

	std::cout << "    Calculando Base Secuencial (1 Hilo)...\n";
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
	std::cout << "    -> Tiempo Base: " << std::fixed << std::setprecision(4) << t_secuencial << " ms\n";
	std::cout << "    -> Resultado  : " << std::setprecision(10) << integral_secuencial << "\n\n";

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

	std::cout << "\nTABLA DE RENDIMIENTO (Tiempos en ms | E = Eficiencia)\n";
	std::cout << std::string(95, '-') << "\n";
	std::cout << std::left << std::setw(8) << "Hilos"
		<< std::setw(12) << "T.Static" << std::setw(12) << "T.Dynamic" << std::setw(12) << "T.Guided"
		<< " | "
		<< std::setw(10) << "E.Static" << std::setw(10) << "E.Dynamic" << std::setw(10) << "E.Guided\n";
	std::cout << std::string(95, '-') << "\n";

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

		std::cout << std::left << std::setw(8) << p
			<< std::fixed << std::setprecision(2)
			<< std::setw(12) << t_static
			<< std::setw(12) << t_dynamic
			<< std::setw(12) << t_guided
			<< " | "
			<< std::setw(10) << eficiencia_static
			<< std::setw(10) << eficiencia_dynamic
			<< std::setw(10) << eficiencia_guided
			<< "\n";
		//Hablando con los resultados en mano, podemos concluir que hay un empate tecnico entre static y guided, saliendo static como la ligera ganadora, debido a que ese desbalanceo no es tan grande como el tiempo que consume guided en gestionar la pila. Dynamic se queja un poco atrás, ya que no cuenta con la inteligencia que aplica guided, aunque la verdad es que estan los tres bastante empatados.

	}
}

void Parte3(long long N) {
	std::cout << "\n >>> PARTE 3.1: Montecarlo\n";
	std::cout << " " << std::string(40, '-') << "\n";
	//La logica es la siguiente: 
		//Imagina un cuadrado de lados 2, Area 4 u2, centrado en el (0,0) 
		// y dentro un circulo unitario de lado dos, que toque los bordes. Area circulo pi * r^2 = pi * 1 = pi.
		//Lanzamos N dardos al azar, cayendo todos dentro del cuadrado, que llamaermos tablero.
		//La proporcion de dardos que caen dentro del circulo seguirá la siguiente formula:
		//Dardos dentro / Total lanzados = Area circulo / Area Cuadrado = pi/4
		//Si despejamos pi = 4 x Dardos Dentro / Total lanzados. Esta será la formula que modelizaremos

	//En primer lugar, generamos un bucle for que genera una X y una Y en cada iteracion (dardo), ambos entre (-1,1).
	//Después, comrpobamos si es un Dardo Dentro, usando pitágoras. x^2 + y^2 <= 1
	// Si la distancia al centro (0,0) es menor o igual que 1, quiere decirnos que estamos dentro del circulo.
	
	//Si el dardo cae dentro, hay que sumar el contador +1. Sin embargo, al estar todo en paralelo, esta variable hay que protegerla, por lo que podriamos aplicar reduction, con sus acumuladores privados por hilo, para evitar peleas entre hilos al querer modificar el acumulador.

	long long aciertos = 0;
	double t0 = omp_get_wtime();

#pragma omp parallel reduction(+: aciertos)
	{

		//La semilla se basa en el numero de hilo, su id
	unsigned int seed = omp_get_thread_num();
	//Cada hilo crea su propio generador, para eliminar carreras posibles, mediante la semilla más el tiempo de creacion del hilo. 
	//La distribucion es uniforme, de -1 a 1.
	std::mt19937 generador(seed + (unsigned int)time(NULL));
	std::uniform_real_distribution<double> distribucion(-1.0, 1.0);

	#pragma omp for schedule(static)
		for (long long i = 0; i < N; i++) {
			double x = distribucion(generador);
			double y = distribucion(generador);
			if ((x*x + y*y) <= 1) {
				aciertos++;
			}
		}
	}
	double t1 = omp_get_wtime();
	double tiempo_montecarlo = (t1 - t0) * 1000.0; // Pasamos a milisegundos

	double pi;
	pi = 4.0 * static_cast<double>(aciertos) / static_cast<double>(N);

	std::cout << "    [MonteCarlo] PI Estimado : " << std::fixed << std::setprecision(10) << pi << "\n";
	std::cout << "    [MonteCarlo] Tiempo      : " << std::setprecision(4) << tiempo_montecarlo << " ms\n";
	//En esta paralelizacion, la granularidad es muy gruesa.
	// Esto quiere decir que, si medimos el tiempo que se tarda en generar un numero aleatorio de calidad es el siguiente:
	//Sin embargo, en la Parte 1 se tarda infinitamente menos en cada iteracion del bucle de reduccion. 
	//Hablando de la sincronizacion, que preferimos? Una granularidad fina, que se vea eclipsada por el tiempo de comunicacion (overhead), o una gruesa, para que el tiempo de overhead sea insignificante?.
	//Por ello, el tiempo de sincronizacion se nota muchisimo más cuando estamos tratando con operaciones muy simples, que cuando tratamos con operaciones grandes.
}

struct alignas(64) Contador {
	long long valor;
};

void TestSincronizacion(long long N) {
	std::cout << "\n >>> PARTE 3.2.1: Sincronizacion MC vs Trapecio\n";
	std::cout << " " << std::string(40, '-') << "\n";

	// Test Trapecio (1 hilo)
	double h = 1.0 / static_cast<double>(N);
	double alturas_secuencial = 0.0;
	double t0_trap, t1_trap, t_trap;

	t0_trap = omp_get_wtime();
	alturas_secuencial = funcion_x(0.0) + funcion_x(1.0);

	for (long long i = 1; i < N; i++) {
		double x = h * static_cast<double>(i);
		alturas_secuencial += 2.0 * funcion_x(x);
	}

	double pi_secuencial = (h / 2.0) * alturas_secuencial;

	t1_trap = omp_get_wtime();
	t_trap = (t1_trap - t0_trap) * 1000; //En ms.
	double tiempo_trap_iteracion = (t_trap * 1000000) / static_cast<double>(N); // ns

	// ARREGLO 1: Engañar al compilador usando la variable
	if (pi_secuencial == -1.0) std::cout << "Ignorar";

	// Test Monte Carlo (1 hilo)
	long long aciertos = 0;
	unsigned int seed = omp_get_thread_num();
	std::mt19937 generador(seed + (unsigned int)time(NULL));
	std::uniform_real_distribution<double> distribucion(-1.0, 1.0);


	double t0_mc = omp_get_wtime();

	for (long long i = 0; i < N; i++) {
		double x = distribucion(generador);
		double y = distribucion(generador);
		if ((x * x + y * y) <= 1) {
			aciertos++;
		}
	}
	double t1_mc = omp_get_wtime();
	double t_mc = (t1_mc - t0_mc) * 1000.0; // Pasamos a milisegundos
	double tiempo_mc_iteracion = (t_mc * 1000000) / static_cast<double>(N); // ns

	if (aciertos == -1) std::cout << "Ignorar";

	std::cout << std::fixed << std::setprecision(5);
	std::cout << "  Trapecio (1 hilo)   : " << t_trap << " ms total"
		<< " -> " << tiempo_trap_iteracion << " ns/iteracion\n";
	std::cout << "  Monte Carlo (1 hilo): " << t_mc << " ms total"
		<< " -> " << tiempo_mc_iteracion << " ns/iteracion\n";

	double ratio_trabajo = tiempo_mc_iteracion / tiempo_trap_iteracion;
	std::cout << "\n---Monte Carlo es " << std::setprecision(5) << ratio_trabajo
		<< "x mas costoso por iteracion\n";

	std::cout << "\n >>> PARTE 3.2.2: Analisis Overhead de Paralelizacion\n";
	std::cout << " " << std::string(40, '-') << "\n";

	double overhead_us = 50.0; // Estimación conservadora
	double iteraciones_para_amortizar_trap = (overhead_us * 1000.0) / tiempo_trap_iteracion;
	double iteraciones_para_amortizar_mc = (overhead_us * 1000.0) / tiempo_mc_iteracion;

	std::cout << "  Iteraciones necesarias para amortizar overhead:\n";
	std::cout << "    - Trapecio: ~" << std::setprecision(1) << iteraciones_para_amortizar_trap
		<< " iteraciones\n";
	std::cout << "    - Monte Carlo: ~" << iteraciones_para_amortizar_mc << " iteraciones\n\n";

	std::cout << "  \033[1;33m CONCLUSION:\033[0m\n";
	std::cout << "    El Trapecio necesita "
		<< std::setprecision(1) << (iteraciones_para_amortizar_trap / iteraciones_para_amortizar_mc)
		<< "x MAS iteraciones que Monte Carlo\n";
	std::cout << "    para que la paralelizacion compense el overhead.\n";
}


void Parte3_SinFalseSharing(long long N) {
	//Para el apartado de optimizar este ejercicio, hemos decidido explorar el de False Sharing.
	//A veces se produce este false sharing del que hablamos.
	//Si dos hilos (a y b) tienen sus propios contadores privados (contA, contB) y da la casualidad que ambos contadores se almacenen contiguos en memoria RAM,
	//, ambos contadores viajaran en el mismo bloque, lo que obliga a B esperar a que A termine o viceversa, ya que no pueden usar el mismo bloque a la vez.
	//Porque usan el mismo bloque? Pues pq la RAM y la CPU se comunican por lineas de cache de 64B, porque la CPU no lee variables (long long ocupa 8B), lee bloques.
	//Por ello, si necesito tocar contadorA, me traigo el bloque en donde vive ese contador en RAM, y inevitablemente tb me traigo el contadorB que vive al lado suyo en el mismo bloque.
	//Si B quisiese escribir en su contador, se le pararian los pies, ya que ese bloque estaria ocupado temporalmente.


	//Como solucionamos esto? Como evitamos que dos contadores se hallen en el mismo bloque?
	//Hacemos que cada contador tenga una estructura que ocupe 64B, aunque de esos 64B solo use los 8B del valor de long long contador.
	//De esta manera, te aseguras al 100% que no se van a producir más condiciones de false sharing.

	std::cout << "\n >>> PARTE 3.3: Monte Carlo (Optimizado - Padding)\n";
	std::cout << " " << std::string(40, '-') << "\n";
	//Esto otorga a cada hilo su propio contador privado, que ocupa 64B.
	
	int numHilos = omp_get_max_threads();
	std::vector<Contador> contadores(numHilos);
	for (auto& c : contadores) c.valor = 0;

	double t0 = omp_get_wtime();
#pragma omp parallel
	{
		int id = omp_get_thread_num();

		unsigned int seed = (unsigned int)time(NULL) + id;
		std::mt19937 generador(seed);
		std::uniform_real_distribution<double> distrib(-1.0, 1.0);

#pragma omp for schedule(static)
		for (long long i = 0; i < N; i++) {
			double x = distrib(generador);
			double y = distrib(generador);

			if (x * x + y * y <= 1.0) {
				contadores[id].valor++;  // Sin false sharing
			}
		}
	}

	long long totalAciertos = 0;
	for (auto& c : contadores) totalAciertos += c.valor;

	double t1 = omp_get_wtime();

	double pi = 4.0 * (double)totalAciertos / (double)N;

	std::cout << "    [Optimizado] PI Estimado : " << std::fixed << std::setprecision(10) << pi << "\n";
	std::cout << "    [Optimizado] Tiempo      : " << std::setprecision(4) << (t1 - t0) * 1000.0 << " ms\n\n";

}
