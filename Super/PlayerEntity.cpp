#include "PlayerEntity.h"
#include "Scene.h"
#include "NpcData.h"
#include "SceneNpc.h"
#include "FightManager.h"
#include "PlayerManager.h"

using namespace loki;

PlayerEntity::PlayerEntity(TcpConnPtr conn): ConnEntity(conn),
	searchFight(false)
{
	conn->SetData(this);
	{
		base.reset(new NpcData());
		auto npc = base;
		npc->id = 1;
		npc->level = 1;
		npc->name = "aaa";
		npc->type = 1;
		npc->maxhp = 200;
		npc->damage = 10;
		npc->attackRange = 1.2f;
		npc->attackInterval = 1.2f;
		npc->moveSpeed = 0;
	}

	{
		std::shared_ptr<NpcData> npc(new NpcData());
		npc->id = 1;
		npc->level = 1;
		npc->name = "aaa";
		npc->type = 1;
		npc->maxhp = 200;
		npc->damage = 10;
		npc->attackRange = 1.2f;
		npc->attackInterval = 1.2f;
		npc->moveSpeed = 3;
		troop.Add(npc->id, npc);
	}

	{
		std::shared_ptr<NpcData> npc(new NpcData());
		npc->id = 2;
		npc->level = 2;
		npc->name = "bbb";
		npc->type = 1;
		npc->maxhp = 200;
		npc->damage = 10;
		npc->attackRange = 1.2f;
		npc->attackInterval = 1.2f;
		npc->moveSpeed = 3;
		troop.Add(npc->id, npc);
	}
}

PlayerEntity::~PlayerEntity()
{
	LOG(INFO)<<__func__;
}

void PlayerEntity::EnterScene(Scene* scene)
{
	for(auto it = troop.data.begin(); it != troop.data.end(); ++it)
	{
		SceneNpc* npc(new SceneNpc(it->second, accid));
		scene->AddSceneNpc(npc);
	}
}

void PlayerEntity::SendCardToMe()
{
	//拥有的卡片
	Super::stPlayerAllCards send;
	for(auto it = troop.data.begin(); it != troop.data.end(); ++it)
	{
		Super::stCardInfo* card = send.add_card();
		card->set_id(it->second->id);
		card->set_level(it->second->level);
	}
	SendCmd(&send);

	//双方的基地信息
	scene->SendBaseInfoToUser(this);
}

void PlayerEntity::DispatchCard(std::shared_ptr<Super::stDispatchCard> msg)
{
	if (!troop.ContainKey(msg->id()))
	{
		LOG(INFO)<<"Error id="<<msg->id();
		return ;
	}
	auto npcdata = troop.Get(msg->id());
	SceneNpc* npc(new SceneNpc(npcdata, accid));
	//TODO: check position and dir validation
	npc->data->mutable_position()->CopyFrom(msg->position());
	npc->data->set_direction(msg->direction());
	scene->AddSceneNpc(npc);
}

void PlayerEntity::SetOnline(bool on)
{
	online = on;
	LOG(INFO)<<"Player Online = "<<online;
}

void PlayerEntity::Offline()
{
	LOG(INFO)<<"User offline name = "<<name;
	SetOnline(false);
	FightManager::instance().Remove(accid);
	//PlayerManager::instance().Remove(accid);
}

void PlayerEntity::Login()
{
	LOG(INFO)<<__func__;
	SetOnline(true);

	Super::stLoginGameServerResult send;
	send.set_ret(0);
	SendCmd(&send);

	PlayerManager::instance().Add(accid, this);
}

void PlayerEntity::Relogin()
{
	LOG(INFO)<<__func__;

	SetOnline(true);

	Super::stLoginGameServerResult send;
	send.set_ret(0);
	SendCmd(&send);
}
