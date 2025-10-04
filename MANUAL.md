# OLD, credits to PeterSeggi

# Manual de uso 
## Instalar el sistema
* Asegurarse de tener la version de qemu `6.2.0` instalada con el servicio `pulseAudio` funcional
* Descargar la imagen de docker con `docker pull agodio/itba-so:1.0`
* Crear la imagen local con  `sudo docker run -d -v ${PWD}:/root --security-opt seccomp:unconfined -ti --name NOMBRE agodio/itba-so:1.0`

## Compilar y correr
* Inicializar la imagen con `docker start NOMBRE`
* Ejecutar los siguientes comandos: `docker exec -ti NOMBRE bash`
.. `cd /root/Toolchain && make clean all`
.. `cd .. && make clean all`
.. `exit`
* Ejecutar el sistema operativo con `./run.sh` 
---
## Dentro del sistema
### Menu Principal
Se pueden usar las teclas `A/D` o `J/L` para seleccionar una de las opciones y la tecla `Enter` para seleccionarla.

### Shell
El teclado predeterminado es el de formato qwerty estandar de estados unidos. Los comandos se escriben en la terminal y se ejecutan con enter. Una funcion `help` lista los comandos disponibles.
*Nota: el guion medio se encuentra a la derecha del 0*

### Snake
Se pueden usar las teclas `W/S` o `I/K` para seleccionar entre 1 o 2 jugadores y seleccionar con `Enter`. Una vez jugando los controles son:
* Jugador 1: `WASD`
* Jugador 2: `IJKL` \

Una vez finalizado el programa este vuelve a la pantalla principal
