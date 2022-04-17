# TP01-SO
Trabajo Práctico 1 de Sistemas Operativos.

Realizado por los alumnos:
- Thomas Mizrahi
- Matias Gonzales

## Instrucciones de Compilación
Los programas deben ser compilados utilizando el contenedor docker provisto por la cátedra, usando el volumen _agodio-itba:1.0_:

```sh
docker pull agodio/itba-so:1.0
cd TP01-SO
docker run -v "${PWD}:/root" --security-opt seccomp:unconfined -ti agodio/itba-so:1.0
```

Dentro del repositorio, se encuentra la carpeta src y un archivo Makefile. Con una terminal abierta en el root del repositorio, se pueden generar los ejecutables “solve” y “worker” ejecutando el comando `make app` y se puede generar el ejecutable “vista” con el comando `make vista`. Puede hacerse ambos al mismo tiempo para generar los ejecutables utilizando `make all`

Todos los ejecutables generados son ubicados en el root del repositorio. Los flags de compilación que se pasan al gcc se encuentran en el archivo _src/Makefile.inc_.

## Instrucciones de Ejecución
Un requerimiento que precisa el programa para correr es tener instalado _minisat_. Esto se puede instalar fácilmente en el contenedor docker con `apt-get`, corriendo el comando:

```sh
apt-get install minisat
```

Una vez generados los ejecutables, archivos con fórmulas en el formato _DIMACS CNF_ pueden ser pasados por parámetro al solve, por ejemplo ejecutando:

```sh
./solve formulas/*.cnf
```

Durante la ejecución, el solve generará un archivo csv conteniendo llamado result.csv. Si el archivo ya existía previamente, será reemplazado. Si el archivo no puede ser abierto, el solve seguirá corriendo pero no generará la salida csv (todavía puede ser conectado el proceso vista para ver la salida).

**Importante:** El archivo ejecutable “worker” debe estar en la misma carpeta que el ejecutable “solve” para que este pueda encontrarlo.

Si se desea conectar un proceso vista, esto se puede realizar de dos maneras. La primera opción es mediante un pipe:

```sh
./solve formulas/*.cnf | ./vista
```

La otra forma es ejecutar el solve sin el pipe hacia vista, en cuyo caso se va a imprimir a consola un string que contiene los datos necesarios para indicarle al proceso vista como conectarse:

```sh
./solve formulas/*.cnf
/MIKALU:4096
```

Este debe luego ser pasado como parámetro al proceso vista antes de que expire un timeout de 10 segundos:

```sh
./vista /MIKALU:4096
```
