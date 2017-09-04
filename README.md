# tarea1-1
Tarea 1, parte 1
Las sucursales incluyen structs para guardar cuentas y transacciones con todos sus datos, en listas ligadas,
junto con sus metodos de push,delete, etc.
Las sucursales constan de un thread principal que es el encargado de realizar las transacciones aleatorias, y
uno secundario que es el encargado de recibir y gestionar las solicitudes provenientes desde la matriz
La matriz solo consta de un thread que es el de consola, no se ha implementado la funcionalidad de comunicacion entre sucursales
por falta de tiempo. La matriz organiza todas las sucursales con sus datos e incluye pipes para comunicarse a cada sucursal.

