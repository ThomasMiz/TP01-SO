# TP01-SO
Trabajo Práctico 1 de Sistemas Operativos.

Realizado por los alumnos:
- Thomas Mizrahi
- Matias Gonzales

## Instrucciones de Compilación
Instrucciones de compilación
Los programas deben ser compilados utilizando el contenedor docker provisto por la cátedra, usando el volumen agodio-itba:1.0:

```bash
docker pull agodio/itba-so:1.0
cd TP01-SO
docker run -v "${PWD}:/root" --security-opt seccomp:unconfined -ti agodio/itba-so:1.0
```

Dentro del repositorio, se encuentra la carpeta src y un archivo Makefile. Con una terminal abierta en el root del repositorio, se pueden generar los ejecutables “solve” y “worker” ejecutando el comando `make app` y se puede generar el ejecutable “vista” con el comando `make vista`. Puede hacerse ambos al mismo tiempo para generar los ejecutables utilizando `make all`

Todos los ejecutables generados son ubicados en el root del repositorio. Los flags de compilación que se pasan al gcc se encuentran en el archivo src/Makefile.inc.
