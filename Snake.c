//Rodriguez Gomez Erick Eduardo - 753535
//Sanchez Arana Jorge Sebastian - 753517
//Organizacion Y Arquitectura de Computadoras
//ITESO Universidad Jesuita de Guadalajara
//21/11/2024
#include "ripes_system.h"
#include <stdbool.h>
#include <stdlib.h>

#define FRUIT_COLOR 0x00FF00 // Verde
#define SNAKE_COLOR 0xFF0000 // Rojo
#define EMPTY_COLOR 0x000000 // Negro

// D-Pad para movimiento
volatile unsigned int *d_pad_up = D_PAD_0_UP;
volatile unsigned int *d_pad_down = D_PAD_0_DOWN;
volatile unsigned int *d_pad_left = D_PAD_0_LEFT;
volatile unsigned int *d_pad_right = D_PAD_0_RIGHT;

// Swicth para reiniciar
volatile unsigned* switch_base = SWITCHES_0_BASE;

// LED Matrix
volatile unsigned int *led_base = LED_MATRIX_0_BASE;

// Variables del tablero
int board_width;
int board_height;

// Representación de la serpiente
typedef struct
{
    int x;
    int y;
} Point;

Point *snake;     // Lista dinámica de posiciones de la serpiente
int snake_length; // Longitud inicial
Point fruit;      // Posición de la fruta

// Variables de dirección
int dir_x = 0;
int dir_y = -2;

//funcion para obtener las posiciones validas del tablero para colocar la fruta
bool is_position_occupied_by_snake(int x, int y) {
    for (int i = 0; i < snake_length; i++) {
        if (snake[i].x == x && snake[i].y == y) {
            return true; // La posición está ocupada por la serpiente
        }
    }
    return false; // La posición está libre
}

//Generacion de una nueva posicion de la fruta:
void generate_fruit_position() {
    do {
        fruit.x = rand() % board_width;
        fruit.y = rand() % board_height;

        // Asegurarse de que las coordenadas sean pares
        if (fruit.x % 2 != 0) {
            if (fruit.x < board_width - 1) fruit.x += 1;
            else fruit.x -= 1;
        }
        if (fruit.y % 2 != 0) {
            if (fruit.y < board_height - 1) fruit.y += 1;
            else fruit.y -= 1;
        }
        //printf("Ubicacion: (%d , %d) \n",fruit.x,fruit.y);
    } while (is_position_occupied_by_snake(fruit.x, fruit.y)); // Repetir si la posición está ocupada
}

// Inicializar tablero
void draw_board()
{
    // Dibujar fruta
    for (int i = 0; i < 2; i++)
    {
        *(led_base + fruit.y * board_width + fruit.x + i) = FRUIT_COLOR;
        *(led_base + (fruit.y + 1) * board_width + fruit.x + i) = FRUIT_COLOR;
    }

    // Dibujar serpiente
    for (int i = 0; i < snake_length; i++)
    {
        *(led_base + snake[i].y * board_width + snake[i].x) = SNAKE_COLOR;
        *(led_base + (snake[i].y+1) * board_width + snake[i].x) = SNAKE_COLOR;
        *(led_base + snake[i].y * board_width + snake[i].x+1) = SNAKE_COLOR;
        *(led_base + (snake[i].y+1) * board_width + (snake[i].x+1)) = SNAKE_COLOR;
        // *(led_base + i * board_width) = SNAKE_COLOR;
    }

    for (int i = 0; i < 10000; i++);
}

void black_cls(){
    for (int y = 0; y < board_height; y++)
    {
        for (int x = 0; x < board_width; x++)
        {
            *(led_base + ((y * board_width) + x)) = EMPTY_COLOR;
        }
    }
}

// Actualizar posición de la serpiente
bool update_snake_position()
{
        //eliminamos la cola para poder dibujar el desplazamiento
        *(led_base + snake[snake_length-1].y * board_width + snake[snake_length-1].x) = EMPTY_COLOR;
        *(led_base + (snake[snake_length-1].y+1) * board_width + snake[snake_length-1].x) = EMPTY_COLOR;
        *(led_base + snake[snake_length-1].y * board_width + snake[snake_length-1].x+1) = EMPTY_COLOR;
        *(led_base + (snake[snake_length-1].y+1) * board_width + (snake[snake_length-1].x+1)) = EMPTY_COLOR; 

    // Nueva cabeza
    int new_x = snake[0].x + dir_x;
    int new_y = snake[0].y + dir_y;

    // Validar colisión con bordes
    if (new_x < 0 || new_y < 0 || new_x >= board_width || new_y >= board_height)
    {
        return false; // Fin del juego
    }

    // Validar colisión con el cuerpo
    for (int i = 0; i < snake_length; i++)
    {
        if (snake[i].x == new_x && snake[i].y == new_y)
        {
            return false; // Fin del juego
        }
    }

    
    // Mover cuerpo: desplazar cada segmento al lugar del anterior
    for (int i = snake_length - 1; i > 0; i--)
    {
        snake[i] = snake[i - 1];
    }

    // Actualizar cabeza
    snake[0].x = new_x;
    snake[0].y = new_y;

    // Detectar si come la fruta
    if (new_x == fruit.x && new_y == fruit.y)
    {
        snake_length++; // Incrementar longitud
        generate_fruit_position();
    }
    return true;
}

// Leer entradas del D-Pad
void read_input()
{
    if (*d_pad_up && dir_y == 0)
    {
        dir_x = 0;
        dir_y = -2;
        //printf("Sube\n");
    }
    if (*d_pad_down && dir_y == 0)
    {
        dir_x = 0;
        dir_y = 2;
        //printf("Baja\n");
    }
    if (*d_pad_left && dir_x == 0)
    {
        dir_x = -2;
        dir_y = 0;
        //printf("Izquierda\n");
    }
    if (*d_pad_right && dir_x == 0)
    {
        dir_x = 2;
        dir_y = 0;
        //printf("Derecha\n");
    }
}

// Configurar juego
void setup_game(int w,int h)
{
    board_width = w;
    board_height = h;

    // Inicializar posiciones iniciales de la serpiente
    snake = (Point *)malloc(board_width * board_height * sizeof(Point));
    snake_length = 2;
    snake[0].x = board_width / 2;
    snake[0].y = board_height / 2;
    //evitamos que la snake aparezca en una posicion impar
    if(snake[0].x %2 != 0) snake[0].x +=1;
    if(snake[0].y %2 != 0) snake[0].y +=1;

    // Inicializar fruta
    generate_fruit_position();
}

// Main del juego
void main()
{
    int wi = LED_MATRIX_0_WIDTH;
    int he = LED_MATRIX_0_HEIGHT;
    while(1){
    setup_game(wi,he);
    black_cls();
        while (1)
        {
            
            read_input(); // Leer movimientos del usuario
            if (!update_snake_position())
            {
                break; // Terminar el juego si hay colisión
            }
            draw_board(); // Dibujar el tablero
            //evaluar si el switch 0 esta encendido
            if ((*switch_base >> 0) & 0x1) {
                for (int i = 0; i < 10000; i++);
                break;
            }
            //podria usarse un goto para reiniciar el juego
            //parar el tiempo con el switch 1
            while((*switch_base >> 1) & 0x1) {
                //espera xd
            }
        }
        // Fin del juego
        for (int i = 0; i < board_width * board_height; i++)
        {
            *(led_base + i) = 0xFF0000; // Rellenar matriz LED en rojo
        }
    free(snake); // Liberar memoria

    while(1){
        //evaluar si el switch 0 esta encendido
        if ((*switch_base >> 0) & 0x1) {
            break;
        }
        for (int i = 0; i < 10000; i++);
    }
    }
}