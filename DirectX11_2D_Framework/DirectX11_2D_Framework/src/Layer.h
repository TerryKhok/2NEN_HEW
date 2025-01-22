#pragma once

enum LAYER
{
	LAYER_BG,
	LAYER_01,
	LAYER_02,
	LAYER_FOG,
	LAYER_UI,
#ifdef DEBUG_TRUE
	LAYER_BOX2D_DEBUG,
#endif
	LAYER_MAX
};

enum FILTER
{
	FILTER_01		= 1 << 0,
	FILTER_02		= 1 << 1,
	F_MAPRAY	= 1 << 2,
	F_PLAYER		= 1 << 3,
	F_TERRAIN		= 1 << 4,
	F_OBSTACLE		= 1 << 5,
	F_WINDOW		= 1 << 6,
	F_OBJECT		= 1 << 7,
	F_PERMEATION	= 1 << 8,
	F_PEROBSTACLE	= 1 << 9,
	F_PERWINDOW		= 1 << 10,
	F_ONLYOBSTACLE	= 1 << 11,
	ALL_BITS = (~0u)
};

