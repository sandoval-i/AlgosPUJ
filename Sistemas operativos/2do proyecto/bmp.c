#include <stdlib.h>
#include "bmp.h"

bool abrir_imagen(BMP *imagen, char *ruta) {
  FILE *archivo;	//Puntero FILE para el archivo de imágen a abrir
  int i,j,k;
  unsigned char P[3];
  //Abrir el archivo de imágen
  archivo = fopen(ruta, "rb+");
  if(!archivo) {
    //Si la imágen no se encuentra en la ruta dada
    printf( "La imágen \"%s\" no se encontro\n",ruta);
    return false;
  }
  //Leer la cabecera de la imagen y almacenarla en la estructura a la que apunta imagen
  fseek( archivo,0, SEEK_SET);
  fread(&imagen->bm,sizeof(char),2, archivo);
  fread(&imagen->tamano,sizeof(int),1, archivo);
  fread(&imagen->reservado,sizeof(int),1, archivo);
  fread(&imagen->offset,sizeof(int),1, archivo);
  fread(&imagen->tamanoMetadatos,sizeof(int),1, archivo);
  fread(&imagen->alto,sizeof(int),1, archivo);
  fread(&imagen->ancho,sizeof(int),1, archivo);
  fread(&imagen->numeroPlanos,sizeof(short int),1, archivo);
  fread(&imagen->profundidadColor,sizeof(short int),1, archivo);
  fread(&imagen->tipoCompresion,sizeof(int),1, archivo);
  fread(&imagen->tamanoEstructura,sizeof(int),1, archivo);
  fread(&imagen->pxmh,sizeof(int),1, archivo);
  fread(&imagen->pxmv,sizeof(int),1, archivo);
  fread(&imagen->coloresUsados,sizeof(int),1, archivo);
  fread(&imagen->coloresImportantes,sizeof(int),1, archivo);
  //Validar ciertos datos de la cabecera de la imágen
  if(imagen->bm[0] != 'B' || imagen->bm[1] != 'M') {
    printf ("La imagen debe ser un bitmap.\n");
    return false;
  }
  if(imagen->profundidadColor != 24) {
    printf ("La imagen debe ser de 24 bits.\n");
    return false;
  }
  //Pasar la imágen a el arreglo reservado en escala de grises
  //unsigned char R,B,G;
  for(i=0;i<imagen->alto;i++) {
    for(j=0;j<imagen->ancho;j++){
      for(k=0;k<3;k++) {
        fread(&P[k],sizeof(char),1, archivo);  //Byte Blue del pixel
        imagen->pixel[i][j][k]=(unsigned char)P[k]; 	//Formula correcta
      }
    }
  }
  //Cerrrar el archivo
  fclose(archivo);
  return true;
}
