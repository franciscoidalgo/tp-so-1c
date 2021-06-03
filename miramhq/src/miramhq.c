#include "miramhq.h"
int main(int argc, char ** argv){

    // variables - instanciacion
    int conexion;
    char* mensaje;
    
    
    // creacion del logger y del config
    t_log* logger = iniciar_logger("miramhq");
    t_config* config = leer_config("miramhq");

    // imprime si alguno de los dos no se pudo crear
    validar_logger(logger);
    validar_config(config);


    // se testea que haya vinculado el logger y funciona el shared
    log_info(logger, "Soy el Mi-RAMar en HD! %s", mi_funcion_compartida());
    // se testea que haya vinculado el config
    mensaje = config_get_string_value(config,"CLAVE");
    log_info(logger,mensaje);

    // generamos la conexion
    conexion = crear_conexion((char*)config_get_string_value(config,"IP"),(char*)config_get_string_value(config,"PUERTO"));

    // enviamos un mensaje al servidor con la conexion que creamos
    enviar_mensaje(mensaje,conexion);


    // terminamos el proceso, eliminamos todo
    terminar_programa(conexion,logger,config);
}