#include <stdio.h>
#include <stdlib.h>
 
typedef struct nodo_s
{
 int dato;
 struct nodo_s *siguiente;
} nodo_t;
 
typedef nodo_t *ptrNodo;
typedef nodo_t *ptrLista;
 
/*
Crea un nuevo nodo y en el campo dato almacena el valor que recibe como parámetro
regresa el nodo recien creado
*/
ptrNodo crea_nodo(int valor)
{
 ptrNodo nuevo_nodo = (ptrNodo)malloc(sizeof(nodo_t));
 if (nuevo_nodo != NULL)
    {
     nuevo_nodo->dato = valor;
     nuevo_nodo->siguiente = NULL;
    }
 
 return nuevo_nodo;
}
 
/*
Agrega a la lista que recibe como parámetro, un nodo enseguida del nodo que recibe como parámetro
Si el nodo que recibe como parámetro es NULL, significa que se desea insertar el nodo al inicio de la lista
*/
void inserta_despues(ptrLista *lista, ptrNodo nodo, int valor)
{
 ptrNodo nuevo_nodo = crea_nodo(valor);
 
 if (nodo != NULL)
    {
     /* El apuntador nuevo_nodo->siguiente va a apuntar a la misma dirección a donde apunta
        el apuntador "siguiente" del nodo que recibe como parámetro
     */
     nuevo_nodo->siguiente = nodo->siguiente;
     /* El apuntador "siguiente" del nodo que recibe como parámetro va a apuntar al nodo recien creado
        con esto, el nodo recien creado se ha insertado adelante del nodo que se recibe como parámetro
     */
     nodo->siguiente = nuevo_nodo;
    }
 else
    {
     // Si la lista no está vacía, hace que el apuntador "siguiente" del nuevo nodo apunte al primer elemento de la lista
     if (*lista != NULL)
         nuevo_nodo->siguiente = *lista;
     // Hace que la lista apunte hacia el nuevo nodo para que sea el primer nodo de la lista
     *lista = nuevo_nodo;
    }
}
 
/*
  Elimina el nodo que se encuentra enseguida del nodo que recibe como parámetro
  Si el nodo que recibe como parámetro es NULL, y la lista no está vacía,
  significa que se desea borrar el primer nodo de la lista
*/
int elimina_despues(ptrLista *lista, ptrNodo nodo)
{
 int x=0;
 ptrNodo borrar_nodo = NULL;
 
 if (nodo != NULL)
    {
     if (nodo->siguiente != NULL)
        {
         /* El apuntador borrar_nodo va a apuntar a la misma dirección a donde apunta
            el apuntador "siguiente" del nodo que recibe como parámetro
         */
         borrar_nodo = nodo->siguiente;
         /* El apuntador "siguiente" del nodo que recibe como parámetro va a apuntar al nodo que está
            a continuación del que se va a borrar
         */
         nodo->siguiente = borrar_nodo->siguiente;
        }
    }
 else
     {
      // Si la lista no está vacia, significa que quiere borrar el primer nodo de la lista
      if (*lista != NULL)
         {
          borrar_nodo = *lista;
          *lista = borrar_nodo->siguiente;
         }
     }
 
 /* Si el apuntador "siguiente" del nodo que recibe como parámetro apunta a NULL, significa que el apuntador
    que se recibió como parámetro es el último de la lista, por lo tanto, no hay nodo siguiente
    Otro caso por el cual borrar_nodo puede ser null, es que la lista esté vacía, y también es imposible hacer la eliminación de un nodo
 */
 if (borrar_nodo != NULL)
    {
     x=borrar_nodo->dato;
     free(borrar_nodo);
    }
 else
     printf("Borrado prohibido\n");
 
 return x;
}
 
/*
  Regresa 1 si no hay nodos en la lista ligada y cero en caso contrario
  *lista es el apuntador que apunta a la lista ligada
*/
int lista_vacia(ptrLista *lista)
{
 return (*lista == NULL ? 1:0);
}
 
/*
  Muestra los datos de los nodos
*/
void nodos_lista(ptrNodo nodo)
{
 if (nodo == NULL)
     printf("La lista está vacia\n");
 else
     {
      while (nodo != NULL)
            {
             printf("%d",nodo->dato);
             nodo = nodo->siguiente;
             if (nodo != NULL)
                 printf(" -> ");
            }
      printf("\n");
     }
}

//funcion
void insertar_orden(int numero,ptrLista *lista,ptrNodo nodo)
{
    int insertado=0, encontrado=0;

    if (numero!=0)
      {
       if (lista_vacia(lista) == 1)
           inserta_despues(lista, NULL, numero);
       else
           {
            nodo = *lista;
            if (numero <= nodo->dato)
                // Inserta el número al inicio de la lista
                inserta_despues(lista, NULL, numero);
            else
                while (nodo != NULL && insertado == 0)
                      {
                       if (numero > nodo->dato)
                          {
                           if (nodo->siguiente != NULL)
                              {
                               if (numero > nodo->siguiente->dato)
                                   nodo = nodo->siguiente;
                               else
                                   {
                                    inserta_despues(lista, nodo, numero);
                                    insertado = 1;
                                   }
                              } /* if */
                           else
                               {
                                inserta_despues(lista, nodo, numero);
                                insertado = 1;
                               }
                          } /* if */
                      } /* while */
           } /* else */
       // Muestra los elementos que están en la lista
      } /* if */
  }
  