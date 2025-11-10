# Trabajo Práctico Nº2
## Construcción del Núcleo de un Sistema Operativo y mecanismos de administración de recursos

### Autores

|Alumno|Legajo|
|-|-|
|Ignacio Gastón Frund|61465|
|Joaquín José Huerta|64252|
|Thomas Adrianus Ruijgt|62875|

## Instrucciones de compilación y ejecución

### Requerimientos
Para la ejecución del sistema es necesario contar con la imagen de Docker de la cátedra de Sistemas Operativos, QEMU, y una copia del presente repositorio.

#### Imagen

Para obtener la imagen de Docker de la cátedra utilizar el comando:

```docker pull agodio/itba-so-multi-platform:3.0```

### Inicialización del entorno de compilación

Para iniciar un contenedor temporal con la imagen descargada previamente:

```sudo docker run --rm -v ${PWD}:/root --security-opt seccomp:unconfined --add-host=host.docker.internal:host-gateway-it agodio/itba-so-multi-platform:3.0```

Alternativamente puede utilizarse un contenedor que cuente con dicha imagen.

### Compilación

El sistema cuenta con dos administradores de memoria: uno simple ideado por los autores y otro que utiliza la técnica de alocación **buddy**. Por lo tanto, se ofrecen dos opciones de compilación, una para cada uno de los administradores.

#### Compilación con administrador de memoria simple
```make all```

#### Compilación con administrador de memoria *buddy*
```make buddy```

#### Eliminación de archivos compilados
```make clean```

### Ejecución

La ejecución del sistema se realiza por fuera del entorno de compilación, sobre la terminal de un sistema operativo basado en Linux. **Es decir, no debe ejecutarse dentro del contenedor utilizado para la compilación.**

#### Ejecución del sistema
```./run.sh```

### Debug y análisis de código

El sistema está preparado para ser debugueado con `gdb` y `gdb-dashboard`, además del analizador PVS-studio.

#### Debug

1. Compilar el sistema con las instrucciones de compilación detalladas anteriormente
2. Fuera del contenedor, ejecutar el sistema con el comando `./run.sh gdb`
3. Otra vez dentro del contenedor, ejecutar el comando `gdb`. Se iniciará `gdb-dashboard` para comenzar con el debugueo.

#### PVS-studio

1. Compilar el sistema con el comando `make pvs`
2. Consultar el informe generado en el directorio `/informe_completo.html/index.html`

## Instrucciones de replicación

El teclado del sistema está en inglés, por lo que el mapeo de teclas coincide con este teclado y no con el de español. Se aclarará la combinación de teclas correcta en caso de ser necesario.

### Comandos

|Comando|Descripción|Parámetros
|-|-|-|
|`exit`|Finaliza la shell||
|`clear`|Limpia la pantalla||
|`sleep secs`|Congela el sistema por `secs` segundos|`secs`: `int`, segundos a congelar|
|`help`|Imprime una lista de comandos y sus parámetros||
|`registers`|Imprime el valor actual de los registros|Presionar `LALT` antes de ejecutar|
|`test-mm bytes`|Ejecuta el `test-mm` de la cátedra|`bytes`: `int`, bytes a pedir recurrentemente|
|`test-prio maxprio`|Ejecuta el `test-prio` de la cátedra|`maxprio`: `int`, máxima prioridad a aplicar|
|`test-pcs procs`|Ejecuta el `test-pcs` de la cátedra|`procs`: `int`, cantidad de procesos a crear|
|`test-sync ops sem?`|Ejecuta el `test-sync` de la cátedra|`ops`: `int`, cantidad de veces que se realizan las operaciones. `sem?`: `0\|1`, `0` para no utilizar semáforos, `1` para utilizarlos|
|`mem`|Imprime el estado de la memoria||
|`kill pid`|Mata al proceso `pid`|`pid`: `int`, identificador del proceso a matar|
|`ps`|Imprime el estado de todos los procesos vivos y zombie||
|`nice pid prio`|Cambia la prioridad del proceso `pid` a `prio`|`pid`: `int`, identificador del proceso al que modificar su prioridad\.`prio`: `int`, prioridad a aplicar|
|`block pid`|Bloquea al proceso `pid`|`pid`: `int`, identificador del proceso a bloquear|
|`unblock pid`|Desbloquea al proceso `pid`|`pid`: `int`, identificador del proceso a desbloquear|
|`loop secs`|Imprime un mensaje en pantalla cada `secs` segundos|`secs`: `int`, segundos de espera|
|`wc`|Cuenta la cantidad de líneas escritas en el modo de escritura||
|`cat`|Imprime en pantalla cada línea escrita en el modo de escritura||
|`filter`|Imprime cada vocal de cada línea escrita en el modo de escritura||
|`mvar writers readers`|Simula, mediante semáforos y un pipe, el comportamiento de una variable global sobre la que los escritores escriben una letra y los lectores la imprimen en pantalla con un color distintivo|`writers`: `int`, cantidad de escritores. `readers`: `int`, cantidad de lectores|
|`msg`|Imprime un mensaje en pantalla||

### Caracteres especiales
#### Pipes `|`
Para comunicar procesos creados por comandos, utilizar el caracter  `|`, mediante la combinación ``LSHIFT + ` ``.

Ejemplo: `cat | filter`

#### Background `&`
Para ejecutar procesos creados por comandos en background en lugar de foreground, utilizar el caracter  `&`, mediante la combinación ``LSHIFT + 7``.

Ejemplo: `loop 1 &`

### Atajos
#### Interrumpir ejecución `LCTRL + C`
Para interrumpir la ejecución de un proceso en foreground, utilizar `LCTRL + C`
#### Enviar EOF `LCTRL + D`
Para enviar EOF a un proceso en foreground, utilizar `LCTRL + D`

### Ejemplos para demostrar los requerimientos
Se detallan ejemplos para requerimientos que no son triviales; en términos generales, que un requerimiento sea trivial significa que es necesario para realizar los otros ejemplos. Por ejemplo, reservar memoria es un requerimiento trivial ya que sin él no puede utilizarse la shell (entre otros problemas).
#### Memoria
- `mem`: imprime en pantalla la memoria total, usada y disponible.
#### Procesos, CS y Scheduling
- `loop 10 &`: crea un proceso en background que imprime en pantalla y renuncia al CPU cada 10 segundos. Para el resto de ejemplos se supone que este proceso es el de pid `2`.
- `ps`: imprime todos los procesos vivos o zombie junto con detalles de los mismos, entre ellos su nombre, pid, prioridad y RSP. No se indica si el mismo es foreground: consultar el informe para su justificación.
- `kill 2`: mata al proceso de pid `2`.
- `nice 2 1`: modifica la prioridad del proceso de pid `2` a la más alta, `1`.
- `block 2`: bloquea el proceso de pid `2`.
- `unblock 2`: desbloquea el proceso de pid `2`.
- `cat`: crea un proceso en foreground, hijo de la shell, que para finalizar debe recibir EOF. Una vez que finaliza vuelve a la shell. (Es decir, la shell espera a que su hijo `cat` finalice).
#### Sincronización
- `mvar 2 2`: crea 2 escritores y 2 lectores (del problema de sincronización de escritores y lectores) que utilizan dos semáforos (se crean, abren, cierran, bloquean y liberan).
#### Inter Process Communication
- `cat | wc`: crea y abre un pipe que establece el extremo de escritura de `cat` como el de lectura de `wc`, provocando que se imprima la cantidad de líneas escritas en `cat`.
#### Aplicaciones de User space
Consultar la sección # Comandos, utilizarlas basta para comprobar su funcionamiento.

### Requerimientos faltantes o parciales
#### Cerrar semáforos
Aunque el presente trabajo cuenta con la implementación de la syscall para cerrar semáforos, la misma no se utiliza al matar procesos (incluida la `shell`).

Al matar un proceso que usa semáforos en su interacción con otros, si el mismo se encontraba en uso de la región crítica, no libera el uso del semáforo y los demás procesos quedan bloqueados sin manera de desbloquearlos.

Esto sucede particularmente al utilizar `mvar`; en ejecución es imposible saber qué proceso está ejecutándose en un momento dado y si se mata a uno de ellos es posible matar al que se encontraba en la región crítica (notar además que, por diseño, aunque se mate a un writer también se bloquearán los readers, y viceversa).

A su vez, luego de matar a todos los procesos de `mvar`, los semáforos quedan abiertos.

En la rama `PipesMvar` se encuentra una solución parcial que se decidió no incorporar ya que provocaba bugs en `test-sync`.

#### Prioridades
El uso de prioridades presenta una complicación en la cual procesos idénticos demoran cantidades diferentes de ticks en cumplirse, provocando que no se ejecuten en el orden esperado. Los procesos guardan datos como `total_ticks` y `changes` para documentar cuantos ticks tuvieron y cuántas veces cedieron su lugar en el scheduler, mediante los cuales se verificó que las prioridades y el mecanismo round-robin implementados se respetan; pero al ejecutar `mvar` o `test-prio` los resultados no corresponden con lo esperado.

Para replicar este problema, en el modo de `debug` colocar un breakpoint `b new_test_prio.c:80`, continuar, ejecutar dicho test (`test-prio`), y colocar otro en `b proc.c:create_process`. Una vez parado en éste último podrá mirar los ticks y changes hechos por los procesos, y verificar que aunque todos los `zero_to_max` son idénticos y de misma prioridad (ya que corresponden a los de la primer tanda del test), algunos tardan más ticks que los otros. 

## Limitaciones
- Solo están implementadas las teclas especiales del lado izquierdo del teclado (`LCTRL`, `LSHIFT`, `LALT`)
- El scrolleo de pantalla funciona pero por problemas ajenos al alcance de este trabajo práctico presenta errores donde se imprime basura en las últimas líneas en lugar de limpiarlas para continuar la escritura. Aunque no sea visible, ejecutar `clear` limpia la pantalla pero incluso así presenta algunos problemas.
- Los procesos que imprimen en pantalla de forma recurrente, como `loop` y los readers de `mvar`, lo hacen sobre la línea de comandos pero esta escritura no forma parte del buffer de comandos. Es decir, al enviar un comando, no se tendrá en cuenta lo escrito por estos procesos.

## Citas
- Como nota general, se hizo uso de clientes de IA para encarar la implementación. Todo el código generado por IA fue interpretado, modificado y adaptado sobre secciones puntuales del código, por lo que no hay una cita apropiada para el mismo.
- Tests de la cátedra: https://github.com/alejoaquili/ITBA-72.11-SO/tree/dd249451d7e7133f4e653f946dbad0077ee713bc/kernel-development/tests
- https://pdos.csail.mit.edu/6.828/2019/lec/malloc.c, para la implementación del administrador de memoria buddy.