//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name flashparticle.cpp - The flash particle. */
//
//      (c) Copyright 2007 by Jimmy Salmon
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; only version 2 of the License.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//      02111-1307, USA.
//
//      $Id$

//@{

#include "stratagus.h"
#include "particle.h"
#include "video.h"

static CGraphic *flashgraphic;


CFlashParticle::CFlashParticle(CPosition position) :
	CParticle(position)
{
	flash = new GraphicAnimation(flashgraphic, 22);
}

CFlashParticle::~CFlashParticle()
{
	delete flash;
}

void CFlashParticle::init()
{
	if (!flashgraphic) {
		flashgraphic = CGraphic::New("graphics/particle/flash.png", 240, 194);
		flashgraphic->Load();
	}
}

void CFlashParticle::exit()
{
	if (flashgraphic) {
		CGraphic::Free(flashgraphic);
		flashgraphic = NULL;
	}
}

void CFlashParticle::draw()
{
	CPosition screenPos = ParticleManager.getScreenPos(pos);
	flash->draw(static_cast<int>(screenPos.x), static_cast<int>(screenPos.y));
}

void CFlashParticle::update(int ticks)
{
	flash->update(ticks);
	if (flash->isFinished()) {
		destroy();
	}
}

//@}