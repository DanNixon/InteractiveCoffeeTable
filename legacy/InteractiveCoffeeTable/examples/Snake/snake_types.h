/*
 * Data types uesed for snake game.
 *
 * Interactive Coffee Table
 * Dan Nixon, dan-nixon.com
 * 21/06/2014
 */

#ifndef SNAKETYPES_H
#define SNAKETYPES_H

enum Direction
{
  DIR_UP    = 0,
  DIR_DOWN,
  DIR_LEFT,
  DIR_RIGHT
};

enum LCD_Message
{
  LCD_START,
  LCD_IN_GAME,
  LCD_PAUSED,
  LCD_GAME_OVER,
  LCD_GAME_OVER_SNAKE_HIT,
  LCD_GAME_OVER_EDGE_HIT,
  LCD_HIGH_SCORES
};

struct SnakeNode
{
  SnakeNode *next;
  SnakeNode *prev;

  int8_t x;
  int8_t y;
};

struct Pickup
{
  uint8_t x;
  uint8_t y;

  uint8_t point_value;
};

#endif
