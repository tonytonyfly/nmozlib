/*
	This file is part of NMozLib, "Navi Mozilla Library", a wrapper for Mozilla Gecko

	Copyright (C) 2008 Adam J. Simmons
	http://code.google.com/p/nmozlib/

	Portions Copyright (C) 2006 Callum Prentice and Linden Lab Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "BrowserWindow.h"
#include <stdio.h>

using namespace NMozLib;

CaretInfo::CaretInfo() : visible(false), x(0), y(0), height(0) 
{
}

CaretInfo::CaretInfo(bool visible, int x, int y, int height) : visible(visible), x(x), y(y), height(height)
{
}

bool CaretInfo::operator==(const CaretInfo& rhs) const
{
	return (visible == rhs.visible) && (x == rhs.x) && (y == rhs.y) && (height == rhs.height);
}

bool CaretInfo::operator!=(const CaretInfo& rhs) const
{
	return !(*this == rhs);
}

RenderSurface::RenderSurface() : width(0), height(0), depth(0), rowPitch(0), buffer(0)
{
}

RenderSurface::RenderSurface(short width, short height, short depth) : width(width), height(height), depth(depth), rowPitch(width*depth), buffer(0)
{
	buffer = new unsigned char[height*rowPitch];
}

RenderSurface::~RenderSurface()
{
	delete[] buffer;
}

void RenderSurface::resize(short width, short height, short depth, int rowPitch)
{
	if(width != this->width || height != this->height || depth != this->depth || rowPitch != this->rowPitch)
	{
		this->width = width;
		this->height = height;
		this->depth = depth;
		this->rowPitch = rowPitch;

		if(!buffer)
			delete[] buffer;

		buffer = new unsigned char[height*rowPitch];
	}
}