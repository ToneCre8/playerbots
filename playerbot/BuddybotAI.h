#pragma once
#include "Entities/ObjectGuid.h"
#include "playerbot/PlayerbotAI.h"
#include "playerbot/strategy/actions/MovementActions.h"

class WorldPacket;
class Player;
class Unit;
class Object;
class Item;

class BuddyMoveAction : public MovementAction
{
public:
    BuddyMoveAction(PlayerbotAI* ai) : MovementAction(ai, "buddy move action") {}
    void BuddyFollow(Unit* target, float distance);
};

class BuddybotAI : public PlayerbotAI
{
public:
    BuddybotAI(Player* p);
    ~BuddybotAI();

   
    //============================================================================================
    // Called on every world tick (don't execute too heavy code here).
    void UpdateAI(uint32 elapsed, bool minimal = false) override;
    void DoNextAction(bool minimal = false) override;
    void SetMaster(Player* master) override;
    Player* GetGroupMaster() override;
    bool HasRealPlayerMaster() override { return true; }
    void HandleCommand(uint32 type, const std::string& text, Player& fromPlayer, const uint32 lang = LANG_UNIVERSAL) override;
    bool CanMove() override;
    void ResetBuddyStrategies(bool autoLoad = true);
    //============================================================================================

    bool HandleBuddyMgrCommand(ChatHandler* handler, char const* args);
    std::vector<std::string> HandleCommand(char const* args);

    //============================================================================================
   
    //============================================================================================

    void MoveToPosition(Position);
    void MoveToRange(WorldObject* target, float range);
    void MoveBehind(WorldObject* target, float range);
    void Follow(Unit* target, float followDistance);
    void ClearMovement();
    Position GetPositionBehindTarget(WorldObject* target, float distance);
    void SetFollowGUID(ObjectGuid);
    ObjectGuid GetFollowGUID() { return followGUID; }
    //============================================================================================
    //============================================================================================
   
    Unit* GetUnit(ObjectGuid guid);
    Creature* GetCreature(ObjectGuid guid) const;
    ObjectGuid GetGUID();

    BuddyMoveAction* GetBuddyMover() { return buddyMover.get(); }

private:
    std::unique_ptr<BuddyMoveAction> buddyMover;
    bool shouldFollow = false;
    ObjectGuid followGUID;

};