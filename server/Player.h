#pragma once

enum DirectionEnum
{
	UP,
	DOWN,
	RIGHT,
	LEFT
};

struct Position {
	unsigned char x;
	unsigned char y;

	Position()
	{
		this->x = 0;
		this->y = 0;
	}

	Position(unsigned char x, unsigned char y)
	{
		this->x = x;
		this->y = y;
	}
};

struct Player {
	unsigned char id;
	bool ready;
	bool alive;
	Position position;
	DirectionEnum direction;

	Player() {
		this->id = 0;
		this->ready = false;
		this->alive = false;
		this->direction = RIGHT;
	}

	Player(unsigned char id, DirectionEnum direction, Position startPos) {
		this->id = id;
		this->ready = false;
		this->alive = true;
		this->direction = direction;
		this->position = startPos;
	}
	
	void move(unsigned char x, unsigned char y) {
		position.x += x;
		position.y += y;
	}
};
