
#include "BuddybotAI.h"
#include <iostream>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <sstream>
#include <vector>
#include "playerbot/ServerFacade.h"
#include "playerbot/AiFactory.h"

//=================================================================================================================================================================================

BuddybotAI::BuddybotAI(Player* p) :
    buddy(p)
{
    this->bot = bot;
   
    for (uint8 i = 0; i < (uint8)BotState::BOT_STATE_ALL; i++)
        engines[i] = NULL;

    for (int i = 0; i < MAX_ACTIVITY_TYPE; i++)
    {
        allowActiveCheckTimer[i] = time(nullptr);
        allowActive[i] = false;
    }

    accountId = sObjectMgr.GetPlayerAccountIdByGUID(bot->GetObjectGuid());

    aiObjectContext = AiFactory::createAiObjectContext(bot, this);

    UpdateTalentSpec();

    engines[(uint8)BotState::BOT_STATE_COMBAT] = AiFactory::createCombatEngine(bot, this, aiObjectContext);
    engines[(uint8)BotState::BOT_STATE_NON_COMBAT] = AiFactory::createNonCombatEngine(bot, this, aiObjectContext);
    engines[(uint8)BotState::BOT_STATE_DEAD] = AiFactory::createDeadEngine(bot, this, aiObjectContext);
    engines[(uint8)BotState::BOT_STATE_REACTION] = reactionEngine = AiFactory::createReactionEngine(bot, this, aiObjectContext);

    for (uint8 e = 0; e < (uint8)BotState::BOT_STATE_ALL; e++)
    {
        engines[e]->initMode = false;
        engines[e]->Init();
    }

    currentEngine = engines[(uint8)BotState::BOT_STATE_NON_COMBAT];
    currentState = BotState::BOT_STATE_NON_COMBAT;
   
}

BuddybotAI::~BuddybotAI()
{
    // Stop the server
    //server_.stop();
}

//============================================================================================
//============================================================================================
// Called on every world tick (don't execute too heavy code here).

void BuddybotAI::UpdateAI(uint32 elapsed, bool minimal)
{
    TellPlayer(buddy, BOT_TEXT("We're ticking!"));
}

bool BuddybotAI::HandleBuddyMgrCommand(ChatHandler* handler, char const* args)
{
    WorldSession* m_session = handler->GetSession();
    if (!m_session)
    {
        handler->PSendSysMessage("There is no session active?");
        return false;
    }

    //Player* player = m_session->GetPlayer();

    std::vector<std::string> messages = HandleCommand(args);
    if (messages.empty())
        return true;

    for (std::vector<std::string>::iterator i = messages.begin(); i != messages.end(); ++i)
    {
        handler->PSendSysMessage("{}", i->c_str());
    }

    return true;
}

std::vector<std::string> BuddybotAI::HandleCommand(char const* args)
{

    std::vector<std::string> messages;

    if (!*args)
    {
        messages.push_back("usage: list/reload/tweak/self or add/init/remove PLAYERNAME\n");
        messages.push_back("usage: addclass CLASSNAME");
        return messages;
    }

    char* cmd = strtok((char*)args, " ");

    //std::string input(args);
    //size_t firstDelimiterPos = input.find('-');
    //size_t secondDelimiterPos = input.find('-', firstDelimiterPos + 1);
    //bool isDelimited = true;

    //// Ensure the input has two delimiters
    //if (firstDelimiterPos == std::string::npos || secondDelimiterPos == std::string::npos) {
    //    isDelimited = false;
    //}

    //if (isDelimited) {
    //    // Extract cmd, GUID, and distance as substrings
    //    std::string cmd = input.substr(0, firstDelimiterPos);
    //    std::string guidStr = input.substr(firstDelimiterPos + 1, secondDelimiterPos - firstDelimiterPos - 1);
    //    std::string distanceStr = input.substr(secondDelimiterPos + 1);

    //    // Parse the GUID
    //    ObjectGuid tarGetObjectGuid;
    //    try {
    //        tarGetObjectGuid = ObjectGuidFromRawHex(guidStr); // Use your implementation for GUID parsing
    //    }
    //    catch (const std::exception& ex) {
    //        messages.push_back("Failed to parse GUID: " + guidStr);
    //        return messages;
    //    }

    //    // Parse the distance
    //    float distance = 0.f;
    //    std::string errorMessage;
    //    try {
    //        distance = WordToFloat(distanceStr, errorMessage); // std::stof(distanceStr); // Convert distance to float
    //        messages.push_back(errorMessage);
    //    }
    //    catch (const std::exception& ex) {
    //        messages.push_back("Failed to parse distance: " + distanceStr);
    //        return messages;
    //    }

    //    // Retrieve the target unit
    //    WorldObject* target = ObjectAccessor::GetWorldObject(*player, tarGetObjectGuid);
    //    if (!target) {
    //        messages.push_back("Failed to find unit with GUID: " + guidStr);
    //    }
    //    else {
    //        // Handle specific commands
    //        if (cmd == "moveToRange") {
    //            MoveToRange(player, target, distance);
    //            //messages.push_back("Moved to range distance: " + std::to_string(distance));
    //        }
    //        else if (cmd == "moveBehind") {
    //            MoveBehind(player, target, distance);
    //            //messages.push_back("Moved behind distance: " + std::to_string(distance));
    //        }
    //        else if (cmd == "follow") {
    //            Follow(player, target->ToUnit(), distance);
    //        }
    //        else {
    //            messages.push_back("Unknown command.");
    //            messages.push_back(cmd);
    //        }
    //    }

    //    return messages;
    //}
    //else
    {     
        /*else if (cmd.starts_with("init=") && sscanf(cmd.c_str(), "init=%d", &gs) != -1)
        {
            auto factory = std::make_unique<PlayerbotFactory>(player, buddy->GetLevel(), ITEM_QUALITY_LEGENDARY, gs);
            factory.InitEquipment(false);
            messages.push_back("ok, gear score limit: " + std::to_string(gs / PlayerbotAI::GetItemScoreMultiplier(ItemQualities(ITEM_QUALITY_EPIC))) + "(for epic)");
        }*/

       
        if (!strcmp(cmd, "clearMove"))
        {
            ClearMovement();
            //messages.push_back("Clear motion master.");
            return messages;
        }

        // if (!strcmp(cmd, "learnSpells"))
        // {
            // //uint8 level = buddy->GetLevel();

            // onLevelChange(player, 0);

            // messages.push_back("Learn spells for level.");

            // return messages;
        // }

        if (!strcmp(cmd, "showBank"))
        {
            buddy->GetSession()->SendAreaTriggerMessage("%s", "Open the bank.");
            buddy->GetSession()->SendShowBank(buddy->GetObjectGuid());

            //messages.push_back("Learn spells for level.");

            return messages;
        }

        if (!strcmp(cmd, "teleportTo"))
        {
           /* auto target = buddy->GetTarget();
            if (target)
            {
                buddy->TeleportTo(target->GetMapId(), target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), buddy->GetOrientation());
                buddy->CastSpell(buddy, 52096, true);

                std::ostringstream out;
                out << "Teleporting to: ";
                out << target->GetName();
                out << ".";

                messages.push_back(out.str());
                return messages;
            }*/
        }

        if (!strcmp(cmd, "summon"))
        {
            //if (auto group = buddy->GetGroup())
            //{
            //    for (GroupReference* gr = group->GetFirstMember(); gr; gr = gr->next())
            //    {
            //        auto member = gr->getSource();

            //        if (member == buddy)
            //            continue;

            //        ChatHandler(buddy->GetSession()).PSendSysMessage("%s", member->GetName());
            //        member->GetMotionMaster()->Clear();
            //        member->TeleportTo(buddy->GetMapId(), buddy->GetPositionX(), buddy->GetPositionY(), buddy->GetPositionZ(), buddy->GetOrientation());
            //        /*   member->Relocate(botPos.GetPositionX(), botPos.GetPositionY(), botPos.GetPositionZ(), botPos.GetOrientation());*/
            //        //member->SendMovementFlagUpdate();
            //        member->CastSpell(member, 52096, true);
            //    }
            //}

            //return messages;
        }
    }

    return messages;
}

Unit* BuddybotAI::GetUnit(ObjectGuid guid)
{
    if (!guid)
        return NULL;

    Map* map = buddy->GetMap();
    if (!map)
        return NULL;

    return sObjectAccessor.GetUnit(*buddy, guid);
}

Creature* BuddybotAI::GetCreature(ObjectGuid guid) const
{
    if (!guid)
        return NULL;

    Map* map = buddy->GetMap();
    if (!map)
        return NULL;

    return map->GetCreature(guid);
}

ObjectGuid BuddybotAI::GetGUID()
{
    return buddy->GetObjectGuid();
}

//============================================================================================
//============================================================================================

void BuddybotAI::MoveToPosition(Position pos)
{
    auto distanceToDest = buddy->GetDistance(pos.x, pos.y, pos.z);

    if (distanceToDest > 1.0 && !buddy->IsMoving())
    {
        const auto mm = buddy->GetMotionMaster();
        mm->Clear();
        mm->MovePoint(0, pos, ForcedMovement::FORCED_MOVEMENT_NONE, true);
    }

}

void BuddybotAI::MoveToRange(WorldObject* wo, float range)
{
    const auto mm = buddy->GetMotionMaster();

    auto distanceToTarget = buddy->GetDistance(wo);

    ObjectGuid guid = wo->GetObjectGuid();

    Unit* target = nullptr;

    if (guid.IsCreature())
    {
        target = GetCreature(guid);
    }

    if (guid.IsUnit())
    {
        target = GetUnit(guid);
    }

    if (distanceToTarget > range && (!buddy->IsMoving() || (target && target->IsMoving())))
    {
        float tx = target->GetPositionX();
        float ty = target->GetPositionY();
        float tz = target->GetPositionZ();

        mm->Clear();
        mm->MovePoint(0, tx, ty, tz, ForcedMovement::FORCED_MOVEMENT_NONE, true);
    }
}

void BuddybotAI::MoveBehind(WorldObject* target, float range)
{

    //auto loc = GetPositionBehindTarget(player, target, range);
    //buddy->TeleportTo(target->GetMapId(), target->GetPositionX(), loc.GetPositionY(), loc.GetPositionZ(), loc.GetOrientation());
    //buddy->CastSpell(target->ToUnit(), 69087, true);
    //buddy->CastSpell(player, 51908, true);

    //const auto mm = buddy->GetMotionMaster();

    //auto distanceToTarget = buddy->GetDistance(target);

    //if (distanceToTarget > range && (!buddy->isMoving() || (target->ToUnit() && target->ToUnit()->isMoving())))
    //{
    //    float myangle = Position::NormalizeOrientation(target->GetAbsoluteAngle(player) + float(M_PI));
    //    //float mydist = buddy->GetCombatReach();
    //    Position position;
    //    target->GetNearPoint(player, position.m_positionX, position.m_positionY, position.m_positionZ, 0.f, range, myangle);

    //    float x = target->GetPositionX() + cos(myangle) * range;
    //    float y = target->GetPositionY() + sin(myangle) * range;
    //    float z = target->GetPositionZ();
    //    buddy->UpdateGroundPositionZ(x, y, z);

    //    mm->Clear();
    //    mm->MovePoint(buddy->GetMapId(), x, y, z, true);
    //}
}

bool BuddybotAI::Follow(Unit* target, float followDistance)
{
    float angle = 0;// GetFollowAngle(player, target);
    Position targetPos;
    Position playerPos;
    float distance = buddy->GetDistance(target);

    if (target->IsFriend(buddy) && buddy->IsMounted())
        distance += angle;

    if (!buddy->InBattleGround() && sServerFacade.IsDistanceLessOrEqualThan(sServerFacade.GetDistance2d(buddy, target),
        followDistance))
    {
        // botAI->TellError("No need to follow");
        return false;
    }

    if (buddy->IsNonMeleeSpellCasted(true))
    {
        buddy->CastStop();
    }

    const auto mm = buddy->GetMotionMaster();

    /*if (buddy->stopCheckDelay < 100 && buddy->stopQueued)
    {
        buddy->stopQueued = false;
        mm->Clear();
        buddy->StopMoving();
    }*/


    if (distance > followDistance && (!buddy->IsMoving() || target->IsMoving()))
    {
        float tx = target->GetPositionX();
        float ty = target->GetPositionY();
        float tz = target->GetPositionZ();

        mm->Clear();
        mm->MovePoint(0, tx, ty, tz, ForcedMovement::FORCED_MOVEMENT_NONE, true);
    }

    return false;
}

void BuddybotAI::ClearMovement()
{
    const auto mm = buddy->GetMotionMaster();
    mm->Clear();
    buddy->StopMoving();
}

Position BuddybotAI::GetPositionBehindTarget(WorldObject* target, float distance) {
    Position targetPos;
    target->GetPosition(targetPos.x, targetPos.y, targetPos.z);

    // Get the target's orientation and normalize it
    float orientation = targetPos.GetPositionO();

    // Calculate the offset (ensure negative for "behind" the target)
    float offsetX = -std::cos(orientation) * distance;
    float offsetY = -std::sin(orientation) * distance;

    // Apply the offset to the target's current position
    float newX = targetPos.GetPositionX() + offsetX;
    float newY = targetPos.GetPositionY() + offsetY;
    float newZ = targetPos.GetPositionZ(); // Maintain Z (height)

    // Return the new position
    return Position(newX, newY, newZ, orientation);
}

//===================================================================================================
//===================================================================================================
