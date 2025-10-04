# OLD, credits to PeterSeggi

# tp-arqui

## How-to

## Docker:
Docker se usa para **compilar y linkeditar** nomas.
Docker basicamente es una imagen con un cierto gcc, un cierto nasm y un par de ciertos mas que no nos importan (hasta uqe edite este readme lol)

### Instalar la imagen de docker:
```sudo docker pull agodio/itba‐so:1.0``` -> esto descarga la imagen 

*Note: si esto tira un error, hace ```sudo service docker restart```

```sudo docker run -d -v ${PWD}:/root --security-opt seccomp:unconfined -ti --name NOMBRE agodio/itba-so:1.0``` -> esto crea la imagen y la guarda en su compu local con nombre NOMBRE (yo le puse BOX a la mia)

### Si quieren que docker no necesite sudo 
Hay una parte del pdf (guia uso de Docker) que no hice todaiva para hacer que sea ejecutable sin sudo

### Correr Docker
Esto es lo que mas van a hacer
```sudo docker start NOMBRE``` -> esto inicializa la instancia
```sudo docker exec -ti NOMBRE bash``` -> les devuelve una terminal ya en la instancia
```cd /root``` -> esto los lleva a la directoria donde se labura

*Note: lo de cd Toolkit y make all y eso ya esta hecho*

## Qemu
Quemu es lo que se usa para correr el script en si que ejecuta el sistema operativo. El comando ```run.sh``` se encarga de todo lo que es preaparar el kernel y ejecutarlo

Para correr el kernel solo hace falta hacer ```sudo ./run.sh``` **afuera de Docker**
