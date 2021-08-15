/*
 * Snake on LED matrix
 *
 * Interactive Coffee Table
 * Dan Nixon, dan-nixon.com
 * 21/06/2014
 */

#include <avr/eeprom.h>

#include <LiquidCrystal.h>
#include <LedControl.h>

#include <InteractiveCoffeeTable.h>
#include "snake_types.h"

InteractiveCoffeeTable table;

#define DEFAULT_UPDATE_TIME 500
#define LOWER_UPDATE_TIME 250
#define DELTA_UPDATE_TIME 50
#define DELTA_UPDATE_TIME_SCORE 200

#define NUM_HIGH_SCORE 3
uint32_t eeprom_score_addr[NUM_HIGH_SCORE] = {0, 4, 8};

uint8_t in_game = 0;
uint8_t paused = 0;

uint16_t update_time = DEFAULT_UPDATE_TIME;

Direction requested_direction;
Direction current_direction;

uint32_t last_snake_motion_time = 0;
uint8_t increment_snake_length = 0;

SnakeNode *snake_head = NULL;
SnakeNode *snake_tail = NULL;

Pickup *active_pickup = NULL;

uint32_t score;

void setup()
{
  Serial.begin(9600);
  uint8_t i;

  randomSeed(analogRead(5));

  /*
   * Clear EEPROM high scores if red button held on start up
   */
  uint16_t old_debounce_time = table.getButtonDebounceTime();
  table.setButtonDebounceTime(0);
  table.pollButtons();
  if(table.getButtonState(BUTTON_RED))
  {
    table.setLED(LED_ORANGE, 1);

    for(i = 0; i < NUM_HIGH_SCORE; i++)
      eeprom_write_dword((uint32_t *) eeprom_score_addr[i], 0);
    
    delay(500);
    table.setLED(LED_ORANGE, 0);
  }
  table.setButtonDebounceTime(old_debounce_time);

  table.setButtonCallback(&button_handler);

  lcd_show_message(LCD_START);
  lcd_show_message(LCD_HIGH_SCORES);
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
  if(in_game && !paused &&
      ((millis() - last_snake_motion_time) > update_time))
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
   * Check if the pickup has been collected
   */
  if(active_pickup != NULL)
  {
    if((active_pickup->x == snake_head->x) &&
        (active_pickup->y == snake_head->y))
    {
      increment_snake_length = 1;
      score += active_pickup->point_value;
      delete active_pickup;
      generate_new_pickup();

      lcd_show_message(LCD_IN_GAME);
    }
  }

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
     * Determine direction of tail segment and move tail node accordingly
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
  if(!snake_head_in_game_area())
  {
    lcd_show_message(LCD_GAME_OVER_EDGE_HIT);
    end_game();
  }

  if(point_on_snake_path(snake_head->x, snake_head->y))
  {
    lcd_show_message(LCD_GAME_OVER_SNAKE_HIT);
    end_game();
  }

  /*
   * Set the game update speed
   */
  update_time = DEFAULT_UPDATE_TIME - (DELTA_UPDATE_TIME * (score / DELTA_UPDATE_TIME_SCORE));
  if(update_time < LOWER_UPDATE_TIME)
    update_time = LOWER_UPDATE_TIME;
}

void lcd_show_message(LCD_Message msg)
{
  char score_string[40];
  sprintf(score_string, "Score: %5d", score);

  switch(msg)
  {
    case LCD_START:
      table.lcdClear(LCD_40X2);
      table.lcdPrint(LCD_40X2, 0, 0, "Snake!");
      table.lcdPrint(LCD_40X2, 1, 0, "Press BLUE to start");
      break;
    case LCD_IN_GAME:
      table.lcdClear(LCD_40X2);
      table.lcdPrint(LCD_40X2, 0, 0, "In Game");
      table.lcdPrint(LCD_40X2, 0, 20, score_string);
      break;
    case LCD_PAUSED:
      table.lcdClear(LCD_40X2);
      table.lcdPrint(LCD_40X2, 0, 0, "Pause");
      table.lcdPrint(LCD_40X2, 0, 20, score_string);
      break;
    case LCD_GAME_OVER:
      table.lcdClear(LCD_40X2);
      table.lcdPrint(LCD_40X2, 0, 0, "GAME OVER!");
      table.lcdPrint(LCD_40X2, 0, 20, score_string);
      break;
    case LCD_GAME_OVER_SNAKE_HIT:
      table.lcdClear(LCD_40X2);
      table.lcdPrint(LCD_40X2, 0, 0, "GAME OVER!");
      table.lcdPrint(LCD_40X2, 0, 20, score_string);
      table.lcdPrint(LCD_40X2, 1, 0, "Hit snake");
      break;
    case LCD_GAME_OVER_EDGE_HIT:
      table.lcdClear(LCD_40X2);
      table.lcdPrint(LCD_40X2, 0, 0, "GAME OVER!");
      table.lcdPrint(LCD_40X2, 0, 20, score_string);
      table.lcdPrint(LCD_40X2, 1, 0, "Hit edge of board");
      break;
    case LCD_HIGH_SCORES:
      table.lcdClear(LCD_20X4);
      table.lcdPrint(LCD_20X4, 0, 0, "High Scores");
      
      char buff[20];
      uint8_t i;
      for(i = 0; i < NUM_HIGH_SCORE; i++)
      {
        sprintf(buff, "%d) %4d",
            (i + 1), eeprom_read_dword((uint32_t *) eeprom_score_addr[i]));
        table.lcdPrint(LCD_20X4, (i + 1), 0, buff);
      }
      break;
  }
}

/*
 * Checks if the head of the snake has left the boundary of the LED matrix
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
 * Creates a new randomly located pickup
 */
void generate_new_pickup()
{
  active_pickup = new Pickup();
  
  active_pickup->point_value = 100;

  active_pickup->x = random(0, NUM_MATRIX_ROW_COL);
  active_pickup->y = random(0, NUM_MATRIX_ROW_COL);

  /*
   * This crappy boolean expression can be completely avoided by
   * un-distracted soldering
   */
  uint8_t on_dead_pixel = 
    (
     ((active_pickup->x == 0) && (active_pickup->y == 5)) ||
     ((active_pickup->x == 10) && (active_pickup->y == 3)) ||
     ((active_pickup->x == 6) && (active_pickup->y == 4)) ||
     ((active_pickup->x == 5) && (active_pickup->y == 11)) ||
     ((active_pickup->x == 14) && (active_pickup->y == 13))
    );

  /*
   * Make sure that the new pickup will not be on a dead pixel or already
   * on the snake body
   */
  if(on_dead_pixel || point_on_snake_path(active_pickup->x, active_pickup->y))
    generate_new_pickup();
  else
    table.matrixSetPixel(active_pickup->x, active_pickup->y, 1);
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

  score = 0;
  lcd_show_message(LCD_IN_GAME);

  last_snake_motion_time = millis();
  update_time = DEFAULT_UPDATE_TIME;
  paused = 0;
  in_game = 1;

  generate_new_pickup();
}

/*
 * End game and clear LED matrix
 */
void end_game()
{
  paused = 0;
  in_game = 0;

  delete snake_head;
  delete snake_tail;
  delete active_pickup;

  table.matrixClear();

  int8_t i;
  for(i = NUM_HIGH_SCORE - 1; i >= 0; i--)
  {
    if(eeprom_read_dword((uint32_t *) eeprom_score_addr[i]) < score)
    {
      if(i < (NUM_HIGH_SCORE - 1))
      {
        uint32_t temp_score =
          eeprom_read_dword((uint32_t *) eeprom_score_addr[i]);
        eeprom_write_dword((uint32_t *) eeprom_score_addr[i + 1], temp_score);
      }

      eeprom_write_dword((uint32_t *) eeprom_score_addr[i], score);
    }
  }

  lcd_show_message(LCD_HIGH_SCORES);
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
      {
        paused = !paused;

        if(paused)
          lcd_show_message(LCD_PAUSED);
        else
          lcd_show_message(LCD_IN_GAME);
      }
      else
      {
        start_game();
      }
      break;
    case BUTTON_RED:
      if(in_game)
      {
        lcd_show_message(LCD_GAME_OVER);
        end_game();
      }
      break;
  }
}
