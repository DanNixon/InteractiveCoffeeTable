/*
 * Snake on LED matrix
 *
 * TODO:
 *  -Add pickups
 *  -Add LCD score (EEPROM high scores?)
 *
 * Interactive Coffee Table
 * Dan Nixon, dan-nixon.com
 * 20/06/2014
 */

#include <LiquidCrystal.h>
#include <LedControl.h>

#include <InteractiveCoffeeTable.h>
#include "snake_types.h"

InteractiveCoffeeTable table;

#define SNAKE_UPDATE_TIME 500

uint8_t in_game = 0;

Direction requested_direction;
Direction current_direction;

uint32_t last_snake_motion_time = 0;
uint8_t increment_snake_length = 0;

SnakeNode *snake_head = NULL;
SnakeNode *snake_tail = NULL;

void setup()
{
  Serial.begin(9600);

  table.setButtonCallback(&button_handler);
}

void loop()
{
  table.pollButtons();

  /*
   * Light green LED when in play
   */
  table.setLED(LED_GREEN, in_game);

  /*
   * If a game is in progress and it is time to move the snake
   */
  if(in_game && ((millis() - last_snake_motion_time) > SNAKE_UPDATE_TIME))
  {
    game_loop();
    last_snake_motion_time = millis();
  }
}

void game_loop()
{
  /*
   * If the snake is turning a corner create a new node and assign it to be
   * the head
   */
  if(requested_direction != current_direction)
  {
    SnakeNode *new_head = new SnakeNode;
    new_head->prev = NULL;
    new_head->next = snake_head;
    new_head->x = snake_head->x;
    new_head->y = snake_head->y;

    new_head->next->prev = new_head;
    snake_head = new_head;

    current_direction = requested_direction;
  }
  /*
   * In both cases move the head and light up the LED for the new head position
   */
  switch(current_direction)
  {
    case DIR_UP:
      snake_head->y++;
      break;
    case DIR_DOWN:
      snake_head->y--;
      break;
    case DIR_LEFT:
      snake_head->x--;
      break;
    case DIR_RIGHT:
      snake_head->x++;
      break;
  }
  table.matrixSetPixel(snake_head->x, snake_head->y, 1);

  /*
   * If the snake length should be incremented ignore moving the tail
   */
  if(!increment_snake_length)
  {
    /*
     * Turn off the LED for the last tail position
     */
    table.matrixSetPixel(snake_tail->x, snake_tail->y, 0);

    /*
     * Deternine direction of tail segment and move tail node accordingly
     */
    Direction tail_dir = get_dir_between_nodes(snake_tail, snake_tail->prev);
    switch(tail_dir)
    {
      case DIR_UP:
        snake_tail->y++;
        break;
      case DIR_DOWN:
        snake_tail->y--;
        break;
      case DIR_LEFT:
        snake_tail->x--;
        break;
      case DIR_RIGHT:
        snake_tail->x++;
        break;
    }
  }
  increment_snake_length = 0;

  /*
   * If the tail node overlaps the next node then it is no longer needed
   * (i.e. tail is turning a corner)
   * Remove it and assign next node as tail
   */
  if((snake_tail->x == snake_tail->prev->x) &&
      (snake_tail->y == snake_tail->prev->y))
  {
    SnakeNode *new_tail = snake_tail->prev;
    delete snake_tail;
    snake_tail = new_tail;
    snake_tail->next = NULL;
  }

  /*
   * Check the snake is still in game area and has not collided with its self
   */
  if(!snake_head_in_game_area() ||
      point_on_snake_path(snake_head->x, snake_head->y))
  {
    end_game();
  }
}

/*
 * Checks if the head of the snake has left the boundry of the LED matrix
 */
uint8_t snake_head_in_game_area()
{
  return (
      (snake_head->x < NUM_MATRIX_ROW_COL) &&
      (snake_head->y < NUM_MATRIX_ROW_COL) &&
      (snake_head->x >= 0) &&
      (snake_head->y >= 0)
    );
}

/*
 * Checks if a given point is on the path of the snake
 */
uint8_t point_on_snake_path(uint8_t x, uint8_t y)
{
  SnakeNode *node = snake_head;
  uint8_t sx, sy;

  while(node->next != NULL)
  {
    Direction node_dir = get_dir_between_nodes(node, node->next);
    sx = node->x;
    sy = node->y;
    
    while((sx != node->next->x) || (sy != node->next->y))
    {
      switch(node_dir)
      {
        case DIR_UP:
          sy++;
          break;
        case DIR_DOWN:
          sy--;
          break;
        case DIR_LEFT:
          sx--;
          break;
        case DIR_RIGHT:
          sx++;
          break;
      }

      if((sx == x) && (sy == y))
        return 1;
    }

    node = node->next;
  }

  return 0;
}

/*
 * Determines the direction between two nodes
 */
Direction get_dir_between_nodes(SnakeNode *from, SnakeNode *to)
{
  if     ((from->x == to->x) && (from->y > to->y))
    return DIR_DOWN;
  else if((from->x == to->x) && (from->y < to->y))
    return DIR_UP;
  else if((from->y == to->y) && (from->x < to->x))
    return DIR_RIGHT;
  else
    return DIR_LEFT;
}

/*
 * Start a new game with a pre defined snake
 */
void start_game()
{
  uint8_t i;

  uint8_t y = 7;
  current_direction = DIR_RIGHT;
  requested_direction = DIR_RIGHT;
  
  snake_head = new SnakeNode;
  snake_tail = new SnakeNode;

  snake_head->prev = NULL;
  snake_head->next = snake_tail;
  snake_head->x = 8;
  snake_head->y = y;

  snake_tail->prev = snake_head;
  snake_tail->next = NULL;
  snake_tail->x = 6;
  snake_tail->y = y;

  for(i = snake_tail->x; i <= snake_head->x; i++)
    table.matrixSetPixel(i, y, 1);

  last_snake_motion_time = millis();
  in_game = 1;
}

/*
 * End game and clear LED matrix
 */
void end_game()
{
  in_game = 0;

  delete snake_head;
  delete snake_tail;

  table.matrixClear();
}

/*
 * Handler for button input
 */
void button_handler(ICT_Button button, uint8_t state)
{
  //Ignore button releases
  if(!state)
    return;
  
  switch(button)
  {
    case BUTTON_UP:
      if(current_direction != DIR_DOWN)
        requested_direction = DIR_UP;
      break;
    case BUTTON_DOWN:
      if(current_direction != DIR_UP)
      requested_direction = DIR_DOWN;
      break;
    case BUTTON_LEFT:
      if(current_direction != DIR_RIGHT)
      requested_direction = DIR_LEFT;
      break;
    case BUTTON_RIGHT:
      if(current_direction != DIR_LEFT)
      requested_direction = DIR_RIGHT;
      break;
    case BUTTON_BLUE:
      if(in_game)
        end_game();
      else
        start_game();
      break;

    //TODO: for testing
    case BUTTON_RED:
      increment_snake_length = 1;
      break;
  }
}
