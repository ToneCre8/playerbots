#pragma once
#include "Entities/ObjectGuid.h"
#include "playerbot/PlayerbotAI.h"

class WorldPacket;
class Player;
class Unit;
class Object;
class Item;

class BuddybotAI : public PlayerbotAI
{
public:
    BuddybotAI(Player* p);
    ~BuddybotAI();

   
    //============================================================================================
    // Called on every world tick (don't execute too heavy code here).
    void UpdateAI(uint32 elapsed, bool minimal = false) override;
    void DoNextAction(bool minimal = false) override;
    bool CanDoSpecificAction(const std::string& name, bool isUseful = true, bool isPossible = true) override;
    bool DoSpecificAction(const std::string& name, Event event = Event(), bool silent = false) override;
    void ChangeStrategy(const std::string& name, BotState type) override;
    void SetMaster(Player* master) override;
    Player* GetGroupMaster() override;
    bool HasRealPlayerMaster() override { return true; }
    void ResetBuddyStrategies(bool autoLoad = true);
    //============================================================================================

    bool HandleBuddyMgrCommand(ChatHandler* handler, char const* args);
    std::vector<std::string> HandleCommand(char const* args);

    //============================================================================================
    //============================================================================================

    void MoveToPosition(Position);
    void MoveToRange(WorldObject* target, float range);
    void MoveBehind(WorldObject* target, float range);
    bool Follow(Unit* target, float followDistance);
    void ClearMovement();
    Position GetPositionBehindTarget(WorldObject* target, float distance);

    //============================================================================================
    //============================================================================================
   
    Unit* GetUnit(ObjectGuid guid);
    Creature* GetCreature(ObjectGuid guid) const;
    ObjectGuid GetGUID();

private:

};