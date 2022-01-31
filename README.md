# ServidorTCP_Concurrente
Programa servidor concurrente que permita a un cliente que se conecta utilizando el protocolo TCP obtener distintas informaciones.
El servidor acepta los siguientes comandos:
1. echo: Una vez recibido el comando, el servidor responde exactamente lo mismo que recibe, hasta que reciba nuevamente el comando echo o bien reciba el comando quit. Si recibe nuevamente echo, queda a la espera de otro comando si recibe quit termina la conexión.
2. hora: Cuando recibe el comando, el servidor contesta hora minutos y segundos en formato hh:mm:ss, luego queda a la espera de otro comando.
3. fecha: Cuando recibe el comando, el servidor contesta la fecha en el formato: Martes 08 de Junio de 2021, luego queda a la espera de otro comando.
4. tiempo: Cuando recibe el comando, el servidor contesta El servidor está activo hace xx horas, yy minutos y zz segundos, si el tiempo no llega a una hora solo se escribirán minutos y segundos y si no llega a 1 minuto solo se escribirán los segundos, luego queda a la espera de otro comando.
5. quit: Termina la conexión.
Además el servidor lleva un archivo de registro (log) de todas las conexiones entrantes, con fecha y hora de inicio y finalización.
