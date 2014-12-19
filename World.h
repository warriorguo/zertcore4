/*
 * World.h
 *
 *  Created on: 2013-3-22
 *      Author: Administrator
 */

#ifndef WORLD_H_
#define WORLD_H_

class World
{
public:
	void init();
	void update();

	bool addWorldObject(WorldObject::pointer object);
	bool removeWorldObject(WorldObject::pointer object);

private:
	Map _map; // quadtree & cells
};


#endif /* WORLD_H_ */
