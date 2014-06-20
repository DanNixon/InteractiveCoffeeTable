/*
 * Data types uesed for snake game.
 *
 * Interactive Coffee Table
 * Dan Nixon, dan-nixon.com
 * 20/06/2014
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

struct SnakeNode
{
  SnakeNode *next;
  SnakeNode *prev;

  int8_t x;
  int8_t y;
};

#endif
