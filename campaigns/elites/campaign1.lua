--       _________ __                 __                               
--      /   _____//  |_____________ _/  |______     ____  __ __  ______
--      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
--      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ \ 
--     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
--             \/                  \/          \//_____/            \/ 
--  ______________________                           ______________________
--        T H E   W A R   B E G I N S
--     Stratagus - A free fantasy real time strategy game engine
--
--  campaign.lua  -  Define the Survival campaign 1.
--
--  (c) Copyright 2005 by Lo�s Taulelle
--
--      This program is free software; you can redistribute it and/or modify
--      it under the terms of the GNU General Public License as published by
--      the Free Software Foundation; either version 2 of the License, or
--      (at your option) any later version.
--  
--      This program is distributed in the hope that it will be useful,
--      but WITHOUT ANY WARRANTY; without even the implied warranty of
--      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--      GNU General Public License for more details.
--  
--      You should have received a copy of the GNU General Public License
--      along with this program; if not, write to the Free Software
--      Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
--
--  $Id: campaign1.lua,v 1.3 2004/08/04 09:36:07 gulldarek Exp $

--=============================================================================
--  Define the campaign
--
DefineCampaign("elites", "name", "~!Elites",
    "campaign", {
    "play-level", "campaigns/elites/level01.cm",
    "play-level", "campaigns/elites/level02.cm",
    "play-level", "campaigns/elites/level03.cm",
    "play-level", "campaigns/elites/level04.cm",
    "play-level", "campaigns/elites/level05.cm",
    "play-level", "campaigns/elites/level06.cm",
    "play-level", "campaigns/elites/level07.cm",
    "play-level", "campaigns/elites/level08.cm",
    "play-level", "campaigns/elites/level09.cm",
    "play-level", "campaigns/elites/level10.cm" })
