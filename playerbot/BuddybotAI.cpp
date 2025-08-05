
#include "BuddybotAI.h"
#include "playerbot/AiFactory.h"
#include "playerbot/ServerFacade.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

//=================================================================================================================================================================================

BuddybotAI::BuddybotAI(Player* player) : PlayerbotAI(player) {}

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
    //TellPlayer(bot, BOT_TEXT("We're ticking!"));

    //engines[(uint8)BotState::BOT_STATE_NON_COMBAT]->DoNextAction(NULL, 0, minimal, bot->IsTaxiFlying());

    //if (bot->IsInCombat())
    //    engines[(uint8)BotState::BOT_STATE_COMBAT]->DoNextAction(NULL, 0, minimal, bot->IsTaxiFlying());

    PlayerbotAI::UpdateAI(elapsed, minimal);
}

void BuddybotAI::DoNextAction(bool min)
{
    if (!bot->IsInWorld() || bot->IsBeingTeleported() || (GetMaster() && GetMaster()->IsBeingTeleported()))
    {
        SetAIInternalUpdateDelay(sPlayerbotAIConfig.globalCoolDown);
        return;
    }

    // if in combat but stuck with old data - clear targets
    if (currentEngine == engines[(uint8)BotState::BOT_STATE_NON_COMBAT] && sServerFacade.IsInCombat(bot))
    {
        if (aiObjectContext->GetValue<Unit*>("current target")->Get() != NULL || aiObjectContext->GetValue<ObjectGuid>("attack target")->Get() != ObjectGuid() ||
            aiObjectContext->GetValue<Unit*>("dps target")->Get() != NULL)
        {
            Reset();
        }
    }

    bool minimal = !AllowActivity();

    currentEngine->DoNextAction(NULL, 0, (minimal || min), bot->IsTaxiFlying());

    if (!bot->IsInWorld()) //Teleport out of bg
        return;

    /*if (minimal)
    {
        if (!bot->isAFK() && !bot->InBattleGround() && !HasRealPlayerMaster())
            bot->ToggleAFK();

        SetAIInternalUpdateDelay(sPlayerbotAIConfig.passiveDelay);
        return;
    }*/
    /*  else if (bot->isAFK())
        bot->ToggleAFK();*/
}

bool BuddybotAI::CanDoSpecificAction(const std::string& name, bool isUseful, bool isPossible)
{
    //TellPlayer(bot, name);
    return PlayerbotAI::CanDoSpecificAction(name, isUseful, isPossible);
}

bool BuddybotAI::DoSpecificAction(const std::string& name, Event event, bool silent)
{
    //TellPlayer(bot, name);
    return PlayerbotAI::DoSpecificAction(name, event, silent);
}

void BuddybotAI::ChangeStrategy(const std::string& name, BotState type)
{
    //TellPlayer(bot, name);
    PlayerbotAI::ChangeStrategy(name, type);
}

void BuddybotAI::SetMaster(Player* newMaster)
{
    master = newMaster;

    if (master == bot)
    {
        bot->clearUnitState(UNIT_STAT_FOLLOW);
        ChangeStrategy("-follow", BotState::BOT_STATE_NON_COMBAT);
    }
    else
    {
        bot->addUnitState(UNIT_STAT_FOLLOW);
        ChangeStrategy("+follow", BotState::BOT_STATE_NON_COMBAT);
    }
}

Player* BuddybotAI::GetGroupMaster() { return master; }

void BuddybotAI::ResetBuddyStrategies(bool autoLoad)
{

    for (uint8 e = 0; e < (uint8)BotState::BOT_STATE_ALL; e++)
    {
        if (auto engine = engines[e])
        {
            engine->removeStrategy("rpg", false);
            engine->removeStrategy("dps assist", false);
            engine->removeStrategy("quest", false);
            engine->removeStrategy("accept all quests", false);
            engine->removeStrategy("emote", false);
            engine->removeStrategy("talk", false);
            engine->removeStrategy("return to stay position", false);
            engine->removeStrategy("greet", false);
            engine->removeStrategy("free", false);
            engine->removeStrategy("runaway", false);
            engine->removeStrategy("grind", false);
            engine->removeStrategy("explore", false);
            engine->removeStrategy("kite", false);
            engine->removeStrategy("flee", false);
            engine->removeStrategy("flee from adds", false);
            engine->removeStrategy("follow jump", false);
            engine->Init();
        }
    }

    for (uint8 e = 0; e < (uint8)BotState::BOT_STATE_ALL; e++)
    {
        if (auto engine = engines[e])
        {
            auto strategies = engine->GetStrategies();
            if (strategies.empty())
                return;

            // Build a comma-separated list with '-' prefix
            std::string names;
            for (auto it = strategies.begin(); it != strategies.end(); ++it)
            {
                std::string strat = std::string(*it);

                if (strat == "default")
                    continue;
                if (strat == "chat")
                    continue;

                TellPlayer(bot, strat);
                if (it != strategies.begin())
                    names += ",";
                names += "-";
                names += std::string(*it); // convert string_view to string
            }

            // Use existing method to handle removal
            ChangeStrategy(names, BotState(e));
        }
    }
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
            auto factory = std::make_unique<PlayerbotFactory>(player, bot->GetLevel(), ITEM_QUALITY_LEGENDARY, gs);
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
        // //uint8 level = bot->GetLevel();

        // onLevelChange(player, 0);

        // messages.push_back("Learn spells for level.");

        // return messages;
        // }

        if (!strcmp(cmd, "showBank"))
        {
            bot->GetSession()->SendAreaTriggerMessage("%s", "Open the bank.");
            bot->GetSession()->SendShowBank(bot->GetObjectGuid());

            //messages.push_back("Learn spells for level.");

            return messages;
        }

        if (!strcmp(cmd, "teleportTo"))
        {
            /* auto target = bot->GetTarget();
            if (target)
            {
                bot->TeleportTo(target->GetMapId(), target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), bot->GetOrientation());
                bot->CastSpell(bot, 52096, true);

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
            //if (auto group = bot->GetGroup())
            //{
            //    for (GroupReference* gr = group->GetFirstMember(); gr; gr = gr->next())
            //    {
            //        auto member = gr->getSource();

            //        if (member == bot)
            //            continue;

            //        ChatHandler(bot->GetSession()).PSendSysMessage("%s", member->GetName());
            //        member->GetMotionMaster()->Clear();
            //        member->TeleportTo(bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), bot->GetOrientation());
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

    Map* map = bot->GetMap();
    if (!map)
        return NULL;

    return sObjectAccessor.GetUnit(*bot, guid);
}

Creature* BuddybotAI::GetCreature(ObjectGuid guid) const
{
    if (!guid)
        return NULL;

    Map* map = bot->GetMap();
    if (!map)
        return NULL;

    return map->GetCreature(guid);
}

ObjectGuid BuddybotAI::GetGUID() { return bot->GetObjectGuid(); }

//============================================================================================
//============================================================================================

void BuddybotAI::MoveToPosition(Position pos)
{
    auto distanceToDest = bot->GetDistance(pos.x, pos.y, pos.z);

    if (distanceToDest > 1.0 && !bot->IsMoving())
    {
        const auto mm = bot->GetMotionMaster();
        mm->Clear();
        mm->MovePoint(0, pos, ForcedMovement::FORCED_MOVEMENT_NONE, true);
    }
}

void BuddybotAI::MoveToRange(WorldObject* wo, float range)
{
    const auto mm         = bot->GetMotionMaster();

    auto distanceToTarget = bot->GetDistance(wo);

    ObjectGuid guid       = wo->GetObjectGuid();

    Unit* target          = nullptr;

    if (guid.IsCreature())
    {
        target = GetCreature(guid);
    }

    if (guid.IsUnit())
    {
        target = GetUnit(guid);
    }

    if (distanceToTarget > range && (!bot->IsMoving() || (target && target->IsMoving())))
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
    //bot->TeleportTo(target->GetMapId(), target->GetPositionX(), loc.GetPositionY(), loc.GetPositionZ(), loc.GetOrientation());
    //bot->CastSpell(target->ToUnit(), 69087, true);
    //bot->CastSpell(player, 51908, true);

    //const auto mm = bot->GetMotionMaster();

    //auto distanceToTarget = bot->GetDistance(target);

    //if (distanceToTarget > range && (!bot->isMoving() || (target->ToUnit() && target->ToUnit()->isMoving())))
    //{
    //    float myangle = Position::NormalizeOrientation(target->GetAbsoluteAngle(player) + float(M_PI));
    //    //float mydist = bot->GetCombatReach();
    //    Position position;
    //    target->GetNearPoint(player, position.m_positionX, position.m_positionY, position.m_positionZ, 0.f, range, myangle);

    //    float x = target->GetPositionX() + cos(myangle) * range;
    //    float y = target->GetPositionY() + sin(myangle) * range;
    //    float z = target->GetPositionZ();
    //    bot->UpdateGroundPositionZ(x, y, z);

    //    mm->Clear();
    //    mm->MovePoint(bot->GetMapId(), x, y, z, true);
    //}
}

bool BuddybotAI::Follow(Unit* target, float followDistance)
{
    float angle = 0; // GetFollowAngle(player, target);
    Position targetPos;
    Position playerPos;
    float distance = bot->GetDistance(target);

    if (target->IsFriend(bot) && bot->IsMounted())
        distance += angle;

    if (!bot->InBattleGround() && sServerFacade.IsDistanceLessOrEqualThan(sServerFacade.GetDistance2d(bot, target), followDistance))
    {
        // botAI->TellError("No need to follow");
        return false;
    }

    if (bot->IsNonMeleeSpellCasted(true))
    {
        bot->CastStop();
    }

    const auto mm = bot->GetMotionMaster();

    /*if (bot->stopCheckDelay < 100 && bot->stopQueued)
    {
        bot->stopQueued = false;
        mm->Clear();
        bot->StopMoving();
    }*/

    if (distance > followDistance && (!bot->IsMoving() || target->IsMoving()))
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
    const auto mm = bot->GetMotionMaster();
    mm->Clear();
    bot->StopMoving();
}

Position BuddybotAI::GetPositionBehindTarget(WorldObject* target, float distance)
{
    Position targetPos;
    target->GetPosition(targetPos.x, targetPos.y, targetPos.z);

    // Get the target's orientation and normalize it
    float orientation = targetPos.GetPositionO();

    // Calculate the offset (ensure negative for "behind" the target)
    float offsetX     = -std::cos(orientation) * distance;
    float offsetY     = -std::sin(orientation) * distance;

    // Apply the offset to the target's current position
    float newX        = targetPos.GetPositionX() + offsetX;
    float newY        = targetPos.GetPositionY() + offsetY;
    float newZ        = targetPos.GetPositionZ(); // Maintain Z (height)

    // Return the new position
    return Position(newX, newY, newZ, orientation);
}

//===================================================================================================
//===================================================================================================
