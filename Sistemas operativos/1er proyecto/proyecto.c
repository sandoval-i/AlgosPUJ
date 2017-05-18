/*
Archivo: proyecto.c
Realizado por: Ivan Dario Sandoval
Contiene: Implementacion de tratamiento de imagenes BMP, con hilos.
Fecha ultima modificacion: 11/03/2017
*/

//*****************************************************************
//LIBRERIAS INCLUIDAS
//*****************************************************************
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>

//********************************************************************************
//DECLARACION DE ESTRUCTURAS
//********************************************************************************
//Estructura para almacenar la cabecera de la imagen BMP y un apuntador a la matriz de pixeles
typedef struct BMP {
	char bm[2];					//(2 Bytes) BM (Tipo de archivo)
	int tamano;					//(4 Bytes) Tamaño del archivo en bytes
	int reservado;					//(4 Bytes) Reservado
	int offset;						//(4 Bytes) offset, distancia en bytes entre la img y los píxeles
	int tamanoMetadatos;			//(4 Bytes) Tamaño de Metadatos (tamaño de esta estructura = 40)
	int alto;						//(4 Bytes) Ancho (numero de píxeles horizontales)
	int ancho;					//(4 Bytes) Alto (numero de pixeles verticales)
	short int numeroPlanos;			//(2 Bytes) Numero de planos de color
	short int profundidadColor;		//(2 Bytes) Profundidad de color (debe ser 24 para nuestro caso)
	int tipoCompresion;				//(4 Bytes) Tipo de compresión (Vale 0, ya que el bmp es descomprimido)
	int tamanoEstructura;			//(4 Bytes) Tamaño de la estructura Imagen (Paleta)
	int pxmh;					//(4 Bytes) Píxeles por metro horizontal
	int pxmv;					//(4 Bytes) Píxeles por metro vertical
	int coloresUsados;				//(4 Bytes) Cantidad de colores usados
	int coloresImportantes;			//(4 Bytes) Cantidad de colores importantes
	unsigned char ***pixel; 			//Puntero a una tabla dinamica de caracteres de 3 dimensiones para almacenar los pixeles
}BMP;

//Estructura para almacenar el rango de la imagen que debe procesar el respectivo hilo
typedef struct INFO_HILO {
  int x_inicial , y_inicial , x_final , y_final , indice;
}INFO_HILO;

//*****************************************************************
//VARIABLES GLOBALES
//*****************************************************************
INFO_HILO *informacion_hilos; // Variable para guardar informacion de cada uno de los posibles hilos
BMP imagen; // Variable para guardar informacion de la imagen a tratar

//*****************************************************************
//DECLARACIÓN DE FUNCIONES
//*****************************************************************
//Funciones tomadas de BMP2.c
void abrir_imagen(char ruta[]);		//Función para abrir la imagen BMP
void crear_imagen(char ruta[]);	//Función para crear una imagen BMP

// Funciones realizadas por Ivan Dario Sandoval
void Resolver(int opcion , int numero_hilos); //Funcion con la logica de division de la imagen y la creacion de hilos
void* Opcion1(void* param); //Funcion que usa el respectivo hilo cuando se transforma la imagen usando la opcion 1
void* Opcion2(void* param); //Funcion que usa el respectivo hilo cuando se transforma la imagen usando la opcion 2
void* Opcion3(void* param); //Funcion que usa el respectivo hilo cuando se transforma la imagen usando la opcion 3
int To_num(int x , int y); // Funcion que brinda un numero unico dado un par de puntos en el plano
void Num_to(int num , int *x  , int *y); // Funcion que brinda un par de puntos en el plano dado un numero unico
void ObtenerFin(int x_inicio , int y_inicio , int salto , int *guardar_x , int *guardar_y); // Funcion que obtiene un par de puntos dado un par de puntos y un rango de salto
void Transformacion1(int i , int j); // Funcion con la logica de la tranformada de la opcion 1
void Transformacion2(int i , int j); // Funcion con la logica de la tranformada de la opcion 2
void Transformacion3(int i , int j); // Funcion con la logica de la tranformada de la opcion 3


/*********************************************************************************************************
//PROGRAMA PRINCIPAL
//*********************************************************************************************************/
int main(int argc , char **argv) {
  int opcion , numero_hilos;
  if(argc != 5) {
    printf("Error en los parametros, uso : %s imagen archivo_salida opcion numero_de_hilos\n" , argv[0]);
    exit(1);
  }
  opcion = atoi(argv[3]);
  numero_hilos = atoi(argv[4]);
  if(numero_hilos <= 0) {
    puts("Error en el numero de hilos, no puede ser negativo o cero");
    exit(1);
  }
  if(opcion < 1 || opcion > 3 ) {
    puts("Error en la opcion, debe ser un numero entre 1 y 3");
    exit(1);
  }
	informacion_hilos = malloc(numero_hilos * sizeof(INFO_HILO));
	if(informacion_hilos == NULL) {
		perror("Error");
		exit(1);
	}
  abrir_imagen(argv[1]);
  Resolver(opcion , numero_hilos);
  crear_imagen(argv[2]);
	free(informacion_hilos);
	exit(0);
}

//************************************************************************
//FUNCIONES
//************************************************************************
/*
Funcion: Transformacion1
Parametros de entrada: Posicion [i,j] de la imagen a transformar usando la opcion 1
Valor de salida: no tiene
Descripcion: Transforma la posicion [i,j] de la imagen usando la opcion 1
*/
void Transformacion1(int i , int j) {
  unsigned char R,G,B;
  B = (unsigned char)(imagen.pixel[i][j][0] * 0.11);
  G = (unsigned char)(imagen.pixel[i][j][1] * 0.59);
  R = (unsigned char)(imagen.pixel[i][j][2] * 0.3);
  imagen.pixel[i][j][0] = imagen.pixel[i][j][1] = imagen.pixel[i][j][2] = (unsigned char)(R+G+B - 10);
}

/*
Funcion: Transformacion2
Parametros de entrada: Posicion [i,j] de la imagen a transformar usando la opcion 2
Valor de salida: no tiene
Descripcion: Transforma la posicion [i,j] de la imagen usando la opcion 2
*/
void Transformacion2(int i , int j) {
  unsigned char R,G,B;
  B = (unsigned char)imagen.pixel[i][j][0];
  G = (unsigned char)imagen.pixel[i][j][1];
  R = (unsigned char)imagen.pixel[i][j][2];
  imagen.pixel[i][j][0] = imagen.pixel[i][j][1] = imagen.pixel[i][j][2] = (unsigned char)((R+G+B)/3.0);
}

/*
Funcion: Transformacion3
Parametros de entrada: Posicion [i,j] de la imagen a transformar usando la opcion 3
Valor de salida: no tiene
Descripcion: Transforma la posicion [i,j] de la imagen usando la opcion 3
*/
void Transformacion3(int i , int j) {
	unsigned char R,G,B;
  B = (unsigned char)imagen.pixel[i][j][0];
  G = (unsigned char)imagen.pixel[i][j][1];
  R = (unsigned char)imagen.pixel[i][j][2];
  imagen.pixel[i][j][0] = imagen.pixel[i][j][1] = imagen.pixel[i][j][2] = R;
}

/*
Funcion: Opcion1
Parametros de entrada: Indice del arreglo informacion_hilos, el cual contiene informacion util para el respectivo hilo
Valor de salida: no tiene
Descripcion: Recorre y tranforma la imagen usando la opcion 1, desde la posicion [x_inicial][y_inicial] hasta [x_final][y_final]
*/
void* Opcion1(void* param) {
  int i,j, p = *(int*)param;
  INFO_HILO info = informacion_hilos[p];
  if(info.x_inicial == info.x_final)
    for(j = info.y_inicial ; j <= info.y_final ; ++j)
      Transformacion1(info.x_inicial , j);
  else {
    for(j = info.y_inicial ; j < imagen.ancho ; ++j)
      Transformacion1(info.x_inicial , j);
    for(i = 1 + info.x_inicial ; i < info.x_final ; ++i )
      for(j = 0 ; j < imagen.ancho ; ++j)
        Transformacion1(i , j);
    for(j = 0 ; j <= info.y_final ; ++j)
      Transformacion1(info.x_final , j);
  }
  pthread_exit(NULL);
}

/*
Funcion: Opcion2
Parametros de entrada: Indice del arreglo informacion_hilos, el cual contiene informacion util para el respectivo hilo
Valor de salida: no tiene
Descripcion: Recorre y tranforma la imagen usando la opcion 2, desde la posicion [x_inicial][y_inicial] hasta [x_final][y_final]
*/
void* Opcion2(void* param) {
  int i,j, p = *(int*)param;
  INFO_HILO info = informacion_hilos[p];
  if(info.x_inicial == info.x_final)
    for(j = info.y_inicial ; j <= info.y_final ; ++j)
      Transformacion2(info.x_inicial , j);
  else {
    for(j = info.y_inicial ; j < imagen.ancho ; ++j)
      Transformacion2(info.x_inicial , j);
    for(i = 1 + info.x_inicial ; i < info.x_final ; ++i )
      for(j = 0 ; j < imagen.ancho ; ++j)
        Transformacion2(i , j);
    for(j = 0 ; j <= info.y_final ; ++j)
      Transformacion2(info.x_final , j);
  }
  pthread_exit(NULL);
}

/*
Funcion: Opcion3
Parametros de entrada: Indice del arreglo informacion_hilos, el cual contiene informacion util para el respectivo hilo
Valor de salida: no tiene
Descripcion: Recorre y tranforma la imagen usando la opcion 3, desde la posicion [x_inicial][y_inicial] hasta [x_final][y_final]
*/
void* Opcion3(void* param) {
  int i,j, p = *(int*)param;
  INFO_HILO info = informacion_hilos[p];
  if(info.x_inicial == info.x_final)
    for(j = info.y_inicial ; j <= info.y_final ; ++j)
      Transformacion3(info.x_inicial , j);
  else {
    for(j = info.y_inicial ; j < imagen.ancho ; ++j)
      Transformacion3(info.x_inicial , j);
    for(i = 1 + info.x_inicial ; i < info.x_final ; ++i )
      for(j = 0 ; j < imagen.ancho ; ++j)
        Transformacion3(i , j);
    for(j = 0 ; j <= info.y_final ; ++j)
      Transformacion3(info.x_final , j);
  }
  pthread_exit(NULL);
}

/*
Funcion: ObtenerFin
Parametros de entrada: Posicion x inicial , posicion y inicial , rango del salto
Valor de salida: no tiene
Descripcion: Halla la posicion i,j que corresponde a la posicion x_inicial,y_inicial despues de haber recorrido "salto" posiciones de la matriz en zig-zag
*/
void ObtenerFin(int x_inicio , int y_inicio , int salto , int *guardar_x , int *guardar_y) {
  Num_to(To_num(x_inicio , y_inicio) + salto , guardar_x , guardar_y);
}

/*
Funcion: Resolver
Parametros de entrada: Opcion(transformada) , numero de hilos a usar
Valor de salida: no tiene
Descripcion: Divide la imagen y asigna a cada fraccion de esta un hilo
*/
void Resolver(int opcion , int numero_hilos) {
  int total_pixeles = imagen.alto * imagen.ancho , pixeles_por_hilo , i = 0 , x_inicio , y_inicio , x_final , y_final,err;
  pthread_t thread;
  INFO_HILO informacion_hilo;
  pixeles_por_hilo = total_pixeles / numero_hilos;
  if(!pixeles_por_hilo) // Maximo 1 hilo por cada pixel
  {
    numero_hilos = total_pixeles;
    pixeles_por_hilo = 1;
  }
  --pixeles_por_hilo;
  x_inicio = y_inicio = 0;
  for(i = 0 ; i < numero_hilos - 1 ; ++i) {
    ObtenerFin(x_inicio , y_inicio , pixeles_por_hilo , &x_final , &y_final);
    informacion_hilos[i].x_inicial = x_inicio;
    informacion_hilos[i].y_inicial = y_inicio;
    informacion_hilos[i].x_final = x_final;
    informacion_hilos[i].y_final = y_final;
    informacion_hilos[i].indice = i;
    err = pthread_create(&thread , NULL , (void*)(opcion == 1 ? Opcion1 :(opcion == 2 ? Opcion2 : Opcion3)) , (void*)&informacion_hilos[i].indice);
    if(err) {
      perror("Error: ");
      exit(-1);
    }
    if(++y_final == imagen.ancho)
      ++x_final , y_final = 0;
    x_inicio = x_final;
    y_inicio = y_final;
  }
  informacion_hilos[numero_hilos - 1].x_inicial = x_inicio;
  informacion_hilos[numero_hilos - 1].y_inicial = y_inicio;
  informacion_hilos[numero_hilos - 1].x_final = imagen.alto - 1;
  informacion_hilos[numero_hilos - 1].y_final = imagen.ancho - 1;
  informacion_hilos[numero_hilos - 1].indice = numero_hilos - 1;
  err = pthread_create(&thread , NULL , (void*)(opcion == 1 ? Opcion1 :(opcion == 2 ? Opcion2 : Opcion3)) , (void*)&informacion_hilos[numero_hilos - 1].indice);
  if(err) {
    perror("Error: ");
    exit(-1);
  }
}

/*
Funcion: To_num
Parametros de entrada: fila , columna
Valor de salida: Valor unico secuencial de la matriz que se puede observar haciendo un recorrido en zig-zag
Descripcion: Halla el valor unico secuencial de la matriz que se puede observar haciendo un recorrido en zig-zag
*/
int To_num(int x , int y) {
  return x * imagen.ancho + y;
}

/*
Funcion: Num_to
Parametros de entrada: Valor unico secuencial
Valor de salida: no tiene
Descripcion: Obtiene la posicion [x][y] que corresponde al valor unico secuencial en la matriz
*/
void Num_to(int num , int *x  , int *y) {
  *x = num / imagen.ancho;
  *y = num % imagen.ancho;
}

void abrir_imagen(char *ruta) {
	FILE *archivo;	//Puntero FILE para el archivo de imágen a abrir
	int i,j,k;
  unsigned char P[3];
	//Abrir el archivo de imágen
	archivo = fopen(ruta, "rb+");
	if(!archivo) {
		//Si la imágen no se encuentra en la ruta dada
		printf("La imágen %s no se encontro\n",ruta);
		exit(1);
	}

	//Leer la cabecera de la imagen y almacenarla en la estructura a la que apunta imagen
	fseek( archivo,0, SEEK_SET);
	fread(&imagen.bm,sizeof(char),2, archivo);
	fread(&imagen.tamano,sizeof(int),1, archivo);
	fread(&imagen.reservado,sizeof(int),1, archivo);
	fread(&imagen.offset,sizeof(int),1, archivo);
	fread(&imagen.tamanoMetadatos,sizeof(int),1, archivo);
	fread(&imagen.alto,sizeof(int),1, archivo);
	fread(&imagen.ancho,sizeof(int),1, archivo);
	fread(&imagen.numeroPlanos,sizeof(short int),1, archivo);
	fread(&imagen.profundidadColor,sizeof(short int),1, archivo);
	fread(&imagen.tipoCompresion,sizeof(int),1, archivo);
	fread(&imagen.tamanoEstructura,sizeof(int),1, archivo);
	fread(&imagen.pxmh,sizeof(int),1, archivo);
	fread(&imagen.pxmv,sizeof(int),1, archivo);
	fread(&imagen.coloresUsados,sizeof(int),1, archivo);
	fread(&imagen.coloresImportantes,sizeof(int),1, archivo);

	//Validar ciertos datos de la cabecera de la imágen
	if(imagen.bm[0] != 'B' || imagen.bm[1] != 'M') {
		printf("La imagen debe ser un bitmap.\n");
		exit(1);
	}
	if(imagen.profundidadColor != 24) {
		printf("La imagen debe ser de 24 bits.\n");
		exit(1);
	}

	//Reservar memoria para la matriz de pixels

	imagen.pixel = malloc(imagen.alto * sizeof(char*));
	for(i = 0 ; i < imagen.alto ; ++i)
		imagen.pixel[i] = malloc(imagen.ancho * sizeof(char*));
  for(i = 0; i < imagen.alto ; ++i)
      for(j = 0 ; j < imagen.ancho ; ++j)
        imagen.pixel[i][j]=malloc(3*sizeof(char));
	//Pasar la imágen a el arreglo reservado en escala de grises
	//unsigned char R,B,G;
	for(i = 0 ; i < imagen.alto ; ++i)
		for(j = 0 ; j < imagen.ancho ; ++j)
		  for(k = 0 ; k < 3 ; ++k) {
        fread(&P[k] , sizeof(char) , 1 , archivo);  //Byte Blue del pixel
        imagen.pixel[i][j][k] = (unsigned char)P[k]; 	//Formula correcta
      }
	//Cerrar el archivo
	fclose(archivo);
}

void crear_imagen(char ruta[]) {
	FILE *archivo;	//Puntero FILE para el archivo de imágen a abrir
	int i,j,k;
	//Abrir el archivo de imágen
	archivo = fopen(ruta, "wb+");
	if(!archivo) {
		//Si la imágen no se encuentra en la ruta dada
		printf( "La imágen %s no se pudo crear\n",ruta);
		exit(1);
	}
	//Escribir la cabecera de la imagen en el archivo
	fseek(archivo,0,SEEK_SET);
	fwrite(&imagen.bm,sizeof(char),2, archivo);
	fwrite(&imagen.tamano,sizeof(int),1, archivo);
	fwrite(&imagen.reservado,sizeof(int),1, archivo);
	fwrite(&imagen.offset,sizeof(int),1, archivo);
	fwrite(&imagen.tamanoMetadatos,sizeof(int),1, archivo);
	fwrite(&imagen.alto,sizeof(int),1, archivo);
	fwrite(&imagen.ancho,sizeof(int),1, archivo);
	fwrite(&imagen.numeroPlanos,sizeof(short int),1, archivo);
	fwrite(&imagen.profundidadColor,sizeof(short int),1, archivo);
	fwrite(&imagen.tipoCompresion,sizeof(int),1, archivo);
	fwrite(&imagen.tamanoEstructura,sizeof(int),1, archivo);
	fwrite(&imagen.pxmh,sizeof(int),1, archivo);
	fwrite(&imagen.pxmv,sizeof(int),1, archivo);
	fwrite(&imagen.coloresUsados,sizeof(int),1, archivo);
	fwrite(&imagen.coloresImportantes,sizeof(int),1, archivo);
	//Pasar la imágen del arreglo reservado en escala de grises a el archivo (Deben escribirse los valores BGR)
	for(i = 0 ; i < imagen.alto ; ++i)
    for(j = 0 ; j < imagen.ancho ; ++j)
      for(k = 0 ; k < 3 ; ++k)
        fwrite(&imagen.pixel[i][j][k] , sizeof(char) , 1 , archivo);  //Escribir el Byte Blue del pixel
	//Cerrrar el archivo
	fclose(archivo);
}
