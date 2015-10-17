# LIN
===============
Arquitectura interna de Linux y Android (Curso 15/16 Facultad de Informática UCM)

Profesor: Juan Carlos Sáez Alcaide

Práctica 1 "Uso avanzado de módulos del kernel"
===============

 Crear un módulo modlist que gestione una lista enlazada de enteros 
 
    -El módulo permitirá al usuario insertar/eliminar elementos de la lista mediante la entrada /proc/modlist 
  
    -Cuando el módulo se cargue/descargue se creará/eliminará dicha entrada 
  
    -La memoria asociada a los nodos de la lista debe gestionarse de forma dinámica empleando vmalloc() y vfree() 
  
    -Al descargar el módulo → liberar memoria si lista no vacía

Práctica 2 "Implementación de llamadas al sistema. Desarrollo de driver para un dispositivo USB"
===============

Práctica 3 "Gestión de procesos y sincronización en el kernel"
===============


Práctica 4 "Gestión de interrupciones y trabajos diferidos"
===============

