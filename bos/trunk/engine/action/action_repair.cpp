//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name action_repair.cpp - The repair action. */
//
//      (c) Copyright 1999-2007 by Vladi Shabanski and Jimmy Salmon
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

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "stratagus.h"
#include "unittype.h"
#include "animation.h"
#include "player.h"
#include "unit.h"
#include "missile.h"
#include "actions.h"
#include "sound.h"
#include "tileset.h"
#include "map.h"
#include "pathfinder.h"
#include "interface.h"
#include "ai.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Move to build location
*/
static void MoveToLocation(CUnit *unit)
{
	// First entry
	if (!unit->SubAction) {
		unit->SubAction = 1;
		NewResetPath(unit);
	}

	if (unit->Wait) {
		// FIXME: show still animation while we wait?
		unit->Wait--;
		return;
	}

	CUnit *goal = unit->Orders[0]->Goal;
	int err = 0;

	switch (DoActionMove(unit)) { // reached end-point?
		case PF_FAILED:
		case PF_UNREACHABLE:
			//
			// Some tries to reach the goal
			//
			if (unit->SubAction++ < 10) {
				// To keep the load low, retry each 10 cycles
				// NOTE: we can already inform the AI about this problem?
				unit->Wait = 10;
				return;
			}

			unit->Player->Notify(NotifyYellow, unit->X, unit->Y,
				_("You cannot reach building place"));
			if (unit->Player->AiEnabled) {
				AiCanNotReach(unit, unit->Orders[0]->Type);
			}

			if (goal) { // release reference
				goal->RefsDecrease();
				unit->Orders[0]->Goal = NoUnitP;
			}
			unit->Orders[0]->Action = UnitActionStill;
			unit->SubAction = 0;
			if (unit->Selected) { // update display for new action
				SelectedUnitChanged();
			}
			return;

		case PF_REACHED:
			break;

		default:
			// Moving...
			break;
	}

	if (unit->Anim.Unbreakable) {
		return;
	}

	//
	// Target is dead, choose new one.
	//
	// Check if goal is correct unit.
	if (goal) {
		if (!goal->IsVisibleAsGoal(unit->Player)) {
			DebugPrint("repair target gone.\n");
			unit->Orders[0]->X = goal->X;
			unit->Orders[0]->Y = goal->Y;
			goal->RefsDecrease();
			unit->Orders[0]->Goal = goal = NULL;
			unit->SubAction = 0;
		}
	} else if (unit->Player->AiEnabled) {
		// Ai players workers should stop if target is killed
		err = -1;
	}

	//
	// Have reached target?
	// FIXME: could use return value of DoActionMove
	//
	if (goal && MapDistanceBetweenUnits(unit, goal) <= unit->Type->RepairRange &&
			goal->Variable[HP_INDEX].Value < goal->Variable[HP_INDEX].Max) {
		unit->State = 0;
		unit->SubAction = 20;
		UnitHeadingFromDeltaXY(unit,
			goal->X + (goal->Type->TileWidth - 1) / 2 - unit->X,
			goal->Y + (goal->Type->TileHeight - 1) / 2 - unit->Y);

		if (goal->Orders[0]->Action == UnitActionBuilt) {
			int costs[MaxCosts];
			for (int i = 0; i < MaxCosts; ++i) {
				costs[i] = std::min<int>(unit->Type->MaxUtilizationRate[i],
					unit->Orders[0]->Goal->Type->ProductionCosts[i]);
			}
			unit->Player->AddToUnitsConsumingResources(unit, costs);
		} else {
			unit->Player->AddToUnitsConsumingResources(unit, unit->Type->MaxUtilizationRate);
		}
	} else if (err < 0) {
		if (goal) { // release reference
			goal->RefsDecrease();
			unit->Orders[0]->Goal = NoUnitP;
		}
		unit->Orders[0]->Action = UnitActionStill;
		unit->State = unit->SubAction = 0;
		if (unit->Selected) { // update display for new action
			SelectedUnitChanged();
		}
		return;
	}

	// FIXME: Should be it already?
	Assert(unit->Orders[0]->Action == UnitActionRepair);
}

/**
**  Animate unit repair
**
**  @param unit  Unit to animate.
*/
static int AnimateActionRepair(CUnit *unit)
{
	UnitShowAnimation(unit, unit->Type->Animations->Repair);
	return 0;
}

/**
**  Do the actual repair.
**
**  @param unit  unit repairing
**  @param goal  unit being repaired
*/
static void DoRepair(CUnit *unit, CUnit *goal)
{
	CPlayer *player = unit->Player;
	int *pcosts = goal->Type->ProductionCosts;
	int pcost = CYCLES_PER_SECOND * (pcosts[1] ? pcosts[1] : pcosts[2]);

	if (goal->Orders[0]->Action != UnitActionBuilt) {
		Assert(goal->Variable[HP_INDEX].Max);

		//
		// Check if enough resources are available
		//
		for (int i = 1; i < MaxCosts; ++i) {
			if (goal->Type->ProductionCosts[i] != 0 && player->ProductionRate[i] == 0 &&
					player->StoredResources[i] == 0) {
				char buf[100];
				snprintf(buf, sizeof(buf) - 1, _("We need more %s for repair!"),
					DefaultResourceNames[i].c_str());
				buf[sizeof(buf) - 1] = '\0';
				player->Notify(NotifyYellow, unit->X, unit->Y, buf);
				if (player->AiEnabled) {
					// FIXME: callback to AI?
					goal->RefsDecrease();
					unit->Orders[0]->Goal = NULL;
					unit->Orders[0]->Action = UnitActionStill;
					unit->State = unit->SubAction = 0;
					if (unit->Selected) { // update display for new action
						SelectedUnitChanged();
					}
				}
				// FIXME: We shouldn't animate if no resources are available.
				return;
			}
		}

		int *costs = unit->Player->UnitsConsumingResourcesActual[unit];
		int cost = costs[1] ? costs[1] : costs[2];

		int hp = goal->Variable[HP_INDEX].Max * cost / pcost;
		goal->Variable[HP_INDEX].Value += hp;
		if (goal->Variable[HP_INDEX].Value > goal->Variable[HP_INDEX].Max) {
			goal->Variable[HP_INDEX].Value = goal->Variable[HP_INDEX].Max;
		}
	} else {
		// hp is the current damage taken by the unit.
		int hp = (goal->Data.Built.Progress * goal->Variable[HP_INDEX].Max) / pcost - goal->Variable[HP_INDEX].Value;

		// Update build progress
		int *costs = unit->Player->UnitsConsumingResourcesActual[unit];
		int cost = costs[1] ? costs[1] : costs[2];
		goal->Data.Built.Progress += cost * SpeedBuild;
		if (goal->Data.Built.Progress > pcost) {
			goal->Data.Built.Progress = pcost;
		}

		// Keep the same level of damage while increasing HP.
		goal->Variable[HP_INDEX].Value = (goal->Data.Built.Progress * goal->Stats->Variables[HP_INDEX].Max) / pcost - hp;
		if (goal->Variable[HP_INDEX].Value > goal->Variable[HP_INDEX].Max) {
			goal->Variable[HP_INDEX].Value = goal->Variable[HP_INDEX].Max;
		}
	}
}

/**
**  Repair unit
*/
static void RepairUnit(CUnit *unit)
{
	CUnit *goal = unit->Orders[0]->Goal;

	if (goal && MapDistanceBetweenUnits(unit, goal) <= unit->Type->RepairRange) {
		DoRepair(unit, goal);
		goal = unit->Orders[0]->Goal;
	}

	AnimateActionRepair(unit);
	if (unit->Anim.Unbreakable) {
		return;
	}

	// Check if goal is gone.
	if (goal) {
		if (!goal->IsVisibleAsGoal(unit->Player)) {
			DebugPrint("repair goal is gone\n");
			unit->Orders[0]->X = goal->X;
			unit->Orders[0]->Y = goal->Y;
			goal->RefsDecrease();
			// FIXME: should I clear this here?
			unit->Orders[0]->Goal = goal = NULL;
			NewResetPath(unit);
		}
	}

	if (goal && MapDistanceBetweenUnits(unit, goal) > unit->Type->RepairRange) {
		// If goal has moved, chase after it
		unit->State = 0;
		unit->SubAction = 0;
	}


	//
	// Target is fine, choose new one.
	//
	if (!goal || goal->Variable[HP_INDEX].Value >= goal->Variable[HP_INDEX].Max) {
		unit->Player->RemoveFromUnitsConsumingResources(unit);
		if (goal) { // release reference
			goal->RefsDecrease();
			unit->Orders[0]->Goal = NULL;
		}
		unit->Orders[0]->Action = UnitActionStill;
		unit->SubAction = unit->State = 0;
		if (unit->Selected) { // update display for new action
			SelectedUnitChanged();
		}
		return;
	}

	// FIXME: automatic repair
}

/**
**  Unit repairs
**
**  @param unit  Unit, for that the attack is handled.
*/
void HandleActionRepair(CUnit *unit)
{
	if (unit->SubAction <= 10) {
		MoveToLocation(unit);
	}
	if (unit->SubAction == 20) {
		RepairUnit(unit);
	}
}

//@}
