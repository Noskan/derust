/// @file W2DbConvert.cpp
/// @brief Implementação de W2DbConvert.h. Segue campo a campo services/w2db/src/legacy.rs.
#include "persistence/W2DbConvert.h"

#include <cstring>
#include <string>

#include "w2db/v1/w2db.pb.h"

namespace pb = w2db::v1;

namespace
{
// Texto de tamanho fixo até o primeiro zero.
std::string CStr(const char* s, size_t n)
{
	size_t len = 0;
	while (len < n && s[len] != 0)
		len++;
	return std::string(s, len);
}

std::string Raw(const void* p, size_t n)
{
	return std::string(static_cast<const char*>(p), n);
}

// Bytes reservados: vazio quando todos são zero.
std::string Reserved(const void* p, size_t n)
{
	const unsigned char* b = static_cast<const unsigned char*>(p);
	for (size_t i = 0; i < n; i++)
		if (b[i] != 0)
			return Raw(p, n);
	return std::string();
}

bool IsZero(const void* p, size_t n)
{
	const unsigned char* b = static_cast<const unsigned char*>(p);
	for (size_t i = 0; i < n; i++)
		if (b[i] != 0)
			return false;
	return true;
}

// Bytes de tamanho fixo: copia o que houver e deixa o resto zerado.
void PutBytes(void* dst, size_t n, const std::string& v)
{
	memset(dst, 0, n);
	memcpy(dst, v.data(), v.size() < n ? v.size() : n);
}

int S8(char c)
{
	return static_cast<signed char>(c);
}

void ItemToProto(const STRUCT_ITEM& it, pb::Item* out)
{
	out->set_index(it.sIndex);
	for (int i = 0; i < 3; i++)
	{
		pb::ItemEffect* e = out->add_effects();
		e->set_effect(it.stEffect[i].cEffect);
		e->set_value(it.stEffect[i].cValue);
	}
}

void ItemFromProto(const pb::Item& in, STRUCT_ITEM* it)
{
	memset(it, 0, sizeof(*it));
	it->sIndex = static_cast<short>(in.index());
	for (int i = 0; i < in.effects_size() && i < 3; i++)
	{
		it->stEffect[i].cEffect = static_cast<unsigned char>(in.effects(i).effect());
		it->stEffect[i].cValue = static_cast<unsigned char>(in.effects(i).value());
	}
}

template <typename Repeated>
void ItemsToProto(const STRUCT_ITEM* items, const uint64_t* serials, int count, Repeated* out)
{
	for (int slot = 0; slot < count; slot++)
	{
		if (IsZero(&items[slot], sizeof(STRUCT_ITEM)))
			continue;
		pb::StoredItem* s = out->Add();
		s->set_serial(serials[slot]);
		s->set_slot(slot);
		ItemToProto(items[slot], s->mutable_item());
	}
}

template <typename Repeated>
bool ItemsFromProto(const Repeated& in, STRUCT_ITEM* items, uint64_t* serials, int count)
{
	for (const pb::StoredItem& s : in)
	{
		if (s.slot() >= static_cast<unsigned>(count))
			return false;
		ItemFromProto(s.item(), &items[s.slot()]);
		serials[s.slot()] = s.serial();
	}
	return true;
}

void ScoreToProto(const STRUCT_SCORE& s, pb::Score* o)
{
	o->set_level(s.Level);
	o->set_ac(s.Ac);
	o->set_damage(s.Damage);
	o->set_merchant(s.Merchant);
	o->set_attack_run(s.AttackRun);
	o->set_direction(s.Direction);
	o->set_chaos_rate(s.ChaosRate);
	o->set_max_hp(s.MaxHp);
	o->set_max_mp(s.MaxMp);
	o->set_hp(s.Hp);
	o->set_mp(s.Mp);
	o->set_str(s.Str);
	o->set_int_(s.Int);
	o->set_dex(s.Dex);
	o->set_con(s.Con);
	for (int i = 0; i < 4; i++)
		o->add_special(s.Special[i]);
}

void ScoreFromProto(const pb::Score& o, STRUCT_SCORE* s)
{
	s->Level = static_cast<short>(o.level());
	s->Ac = o.ac();
	s->Damage = o.damage();
	s->Merchant = static_cast<unsigned char>(o.merchant());
	s->AttackRun = static_cast<unsigned char>(o.attack_run());
	s->Direction = static_cast<unsigned char>(o.direction());
	s->ChaosRate = static_cast<unsigned char>(o.chaos_rate());
	s->MaxHp = o.max_hp();
	s->MaxMp = o.max_mp();
	s->Hp = o.hp();
	s->Mp = o.mp();
	s->Str = static_cast<short>(o.str());
	s->Int = static_cast<short>(o.int_());
	s->Dex = static_cast<short>(o.dex());
	s->Con = static_cast<short>(o.con());
	for (int i = 0; i < 4 && i < o.special_size(); i++)
		s->Special[i] = static_cast<short>(o.special(i));
}

void MobToProto(const STRUCT_MOB& m, pb::Mob* o)
{
	o->set_clan(S8(m.Clan));
	o->set_merchant(m.Merchant);
	o->set_guild(m.Guild);
	o->set_class_(m.Class);
	o->set_rsv(m.Rsv);
	o->set_quest(m.Quest);
	o->set_coin(m.Coin);
	o->set_exp(m.Exp);
	o->set_home_town_x(m.HomeTownX);
	o->set_home_town_y(m.HomeTownY);
	ScoreToProto(m.BaseScore, o->mutable_base_score());
	ScoreToProto(m.CurrentScore, o->mutable_current_score());
	// Vetor de tamanho fixo sem os zeros do fim.
	int n = 2;
	while (n > 0 && m.LearnedSkill[n - 1] == 0)
		n--;
	for (int i = 0; i < n; i++)
		o->add_learned_skill(m.LearnedSkill[i]);
	o->set_score_bonus(m.ScoreBonus);
	o->set_special_bonus(m.SpecialBonus);
	o->set_skill_bonus(m.SkillBonus);
	o->set_critical(m.Critical);
	o->set_save_mana(m.SaveMana);
	o->set_short_skill(Raw(m.ShortSkill, 4));
	o->set_guild_level(m.GuildLevel);
	o->set_magic(m.Magic);
	o->set_regen_hp(m.RegenHP);
	o->set_regen_mp(m.RegenMP);
	o->set_resist(Raw(m.Resist, 4));
	o->set_reserved_dummy(Reserved(m.dummy, sizeof(m.dummy)));
	o->set_current_kill(m.CurrentKill);
	o->set_total_kill(m.TotalKill);
}

void MobFromProto(const pb::Mob& o, STRUCT_MOB* m)
{
	m->Clan = static_cast<char>(o.clan());
	m->Merchant = static_cast<unsigned char>(o.merchant());
	m->Guild = static_cast<unsigned short>(o.guild());
	m->Class = static_cast<unsigned char>(o.class_());
	m->Rsv = static_cast<unsigned char>(o.rsv());
	m->Quest = static_cast<unsigned short>(o.quest());
	m->Coin = o.coin();
	m->Exp = o.exp();
	m->HomeTownX = static_cast<short>(o.home_town_x());
	m->HomeTownY = static_cast<short>(o.home_town_y());
	ScoreFromProto(o.base_score(), &m->BaseScore);
	ScoreFromProto(o.current_score(), &m->CurrentScore);
	for (int i = 0; i < 2 && i < o.learned_skill_size(); i++)
		m->LearnedSkill[i] = o.learned_skill(i);
	m->ScoreBonus = static_cast<unsigned short>(o.score_bonus());
	m->SpecialBonus = static_cast<unsigned short>(o.special_bonus());
	m->SkillBonus = static_cast<unsigned short>(o.skill_bonus());
	m->Critical = static_cast<unsigned char>(o.critical());
	m->SaveMana = static_cast<unsigned char>(o.save_mana());
	PutBytes(m->ShortSkill, 4, o.short_skill());
	m->GuildLevel = static_cast<unsigned char>(o.guild_level());
	m->Magic = static_cast<unsigned char>(o.magic());
	m->RegenHP = static_cast<unsigned char>(o.regen_hp());
	m->RegenMP = static_cast<unsigned char>(o.regen_mp());
	PutBytes(m->Resist, 4, o.resist());
	PutBytes(m->dummy, sizeof(m->dummy), o.reserved_dummy());
	m->CurrentKill = static_cast<unsigned short>(o.current_kill());
	m->TotalKill = static_cast<unsigned short>(o.total_kill());
}

template <typename SC>
void SaveCelestialToProto(const SC& s, pb::SaveCelestial* o)
{
	o->set_class_(s.Class);
	o->set_exp(s.Exp);
	o->set_spx(s.SPX);
	o->set_spy(s.SPY);
	ScoreToProto(s.BaseScore, o->mutable_base_score());
	o->set_learned_skill(s.LearnedSkill);
	o->set_score_bonus(s.ScoreBonus);
	o->set_special_bonus(s.SpecialBonus);
	o->set_skill_bonus(s.SkillBonus);
	o->set_skill_bar1(Raw(s.SkillBar1, 4));
	o->set_skill_bar2(Raw(s.SkillBar2, 16));
	o->set_soul(S8(s.Soul));
	o->set_reserved(Reserved(s.EMPTY, sizeof(s.EMPTY)));
}

template <typename SC>
void SaveCelestialFromProto(const pb::SaveCelestial& o, SC* s)
{
	s->Class = o.class_();
	s->Exp = o.exp();
	s->SPX = static_cast<short>(o.spx());
	s->SPY = static_cast<short>(o.spy());
	ScoreFromProto(o.base_score(), &s->BaseScore);
	s->LearnedSkill = o.learned_skill();
	s->ScoreBonus = static_cast<unsigned short>(o.score_bonus());
	s->SpecialBonus = static_cast<unsigned short>(o.special_bonus());
	s->SkillBonus = static_cast<unsigned short>(o.skill_bonus());
	PutBytes(s->SkillBar1, 4, o.skill_bar1());
	PutBytes(s->SkillBar2, 16, o.skill_bar2());
	s->Soul = static_cast<char>(o.soul());
	PutBytes(s->EMPTY, sizeof(s->EMPTY), o.reserved());
}

void ExtraToProto(const STRUCT_MOBEXTRA& e, pb::MobExtra* o)
{
	o->set_class_master(e.ClassMaster);
	o->set_citizen(S8(e.Citizen));
	o->set_fame(e.Fame);
	o->set_soul(S8(e.Soul));
	o->set_mortal_face(e.MortalFace);

	const auto& q = e.QuestInfo;
	o->set_mortal_newbie(S8(q.Mortal.Newbie));
	o->set_mortal_terra_mistica(S8(q.Mortal.TerraMistica));
	o->set_mortal_molar_gargula(S8(q.Mortal.MolarGargula));
	o->set_mortal_pilula_orc(S8(q.Mortal.PilulaOrc));
	o->set_mortal_reserved(Reserved(q.Mortal.EMPTY, sizeof(q.Mortal.EMPTY)));
	o->set_arch_mortal_slot(S8(q.Arch.MortalSlot));
	o->set_arch_mortal_level(S8(q.Arch.MortalLevel));
	o->set_arch_level355(S8(q.Arch.Level355));
	o->set_arch_level370(S8(q.Arch.Level370));
	o->set_arch_cristal(S8(q.Arch.Cristal));
	o->set_arch_reserved(Reserved(q.Arch.EMPTY, sizeof(q.Arch.EMPTY)));
	o->set_celestial_arch_level(q.Celestial.ArchLevel);
	o->set_celestial_celestial_level(q.Celestial.CelestialLevel);
	o->set_celestial_sub_celestial_level(q.Celestial.SubCelestialLevel);
	o->set_celestial_lv40(S8(q.Celestial.Lv40));
	o->set_celestial_lv90(S8(q.Celestial.Lv90));
	o->set_celestial_add120(S8(q.Celestial.Add120));
	o->set_celestial_add150(S8(q.Celestial.Add150));
	o->set_celestial_add180(S8(q.Celestial.Add180));
	o->set_celestial_add200(S8(q.Celestial.Add200));
	o->set_celestial_arcana(S8(q.Celestial.Arcana));
	o->set_celestial_reset(S8(q.Celestial.Reset));
	o->set_celestial_reserved(Reserved(q.Celestial.EMPTY, sizeof(q.Celestial.EMPTY)));
	o->set_circle(S8(q.Circle));
	o->set_quest_reserved(Reserved(q.EMPTY, sizeof(q.EMPTY)));

	// Sempre as duas: no codec do importador uma SaveCelestial zerada não é "vazia" (tem o Score).
	for (int i = 0; i < 2; i++)
		SaveCelestialToProto(e.SaveCelestial[i], o->add_save_celestial());

	o->set_last_nt(e.LastNT);
	o->set_nt(e.NT);
	o->set_kefra_ticket(e.KefraTicket);
	o->set_divine_end(e.DivineEnd);
	o->set_hold(e.Hold);
	o->set_day_log_exp(e.DayLog.Exp);
	o->set_day_log_year_day(e.DayLog.YearDay);
	o->set_donate_last_time(e.DonateInfo.LastTime);
	o->set_donate_count(e.DonateInfo.Count);
	o->set_reserved(Reserved(e.EMPTY, sizeof(e.EMPTY)));
}

void ExtraFromProto(const pb::MobExtra& o, STRUCT_MOBEXTRA* e)
{
	e->ClassMaster = static_cast<short>(o.class_master());
	e->Citizen = static_cast<char>(o.citizen());
	e->Fame = o.fame();
	e->Soul = static_cast<char>(o.soul());
	e->MortalFace = static_cast<short>(o.mortal_face());

	auto& q = e->QuestInfo;
	q.Mortal.Newbie = static_cast<char>(o.mortal_newbie());
	q.Mortal.TerraMistica = static_cast<char>(o.mortal_terra_mistica());
	q.Mortal.MolarGargula = static_cast<char>(o.mortal_molar_gargula());
	q.Mortal.PilulaOrc = static_cast<char>(o.mortal_pilula_orc());
	PutBytes(q.Mortal.EMPTY, sizeof(q.Mortal.EMPTY), o.mortal_reserved());
	q.Arch.MortalSlot = static_cast<char>(o.arch_mortal_slot());
	q.Arch.MortalLevel = static_cast<char>(o.arch_mortal_level());
	q.Arch.Level355 = static_cast<char>(o.arch_level355());
	q.Arch.Level370 = static_cast<char>(o.arch_level370());
	q.Arch.Cristal = static_cast<char>(o.arch_cristal());
	PutBytes(q.Arch.EMPTY, sizeof(q.Arch.EMPTY), o.arch_reserved());
	q.Celestial.ArchLevel = static_cast<short>(o.celestial_arch_level());
	q.Celestial.CelestialLevel = static_cast<short>(o.celestial_celestial_level());
	q.Celestial.SubCelestialLevel = static_cast<short>(o.celestial_sub_celestial_level());
	q.Celestial.Lv40 = static_cast<char>(o.celestial_lv40());
	q.Celestial.Lv90 = static_cast<char>(o.celestial_lv90());
	q.Celestial.Add120 = static_cast<char>(o.celestial_add120());
	q.Celestial.Add150 = static_cast<char>(o.celestial_add150());
	q.Celestial.Add180 = static_cast<char>(o.celestial_add180());
	q.Celestial.Add200 = static_cast<char>(o.celestial_add200());
	q.Celestial.Arcana = static_cast<char>(o.celestial_arcana());
	q.Celestial.Reset = static_cast<char>(o.celestial_reset());
	PutBytes(q.Celestial.EMPTY, sizeof(q.Celestial.EMPTY), o.celestial_reserved());
	q.Circle = static_cast<char>(o.circle());
	PutBytes(q.EMPTY, sizeof(q.EMPTY), o.quest_reserved());

	for (int i = 0; i < 2 && i < o.save_celestial_size(); i++)
		SaveCelestialFromProto(o.save_celestial(i), &e->SaveCelestial[i]);

	e->LastNT = o.last_nt();
	e->NT = o.nt();
	e->KefraTicket = o.kefra_ticket();
	e->DivineEnd = o.divine_end();
	e->Hold = o.hold();
	e->DayLog.Exp = o.day_log_exp();
	e->DayLog.YearDay = o.day_log_year_day();
	e->DonateInfo.LastTime = o.donate_last_time();
	e->DonateInfo.Count = o.donate_count();
	PutBytes(e->EMPTY, sizeof(e->EMPTY), o.reserved());
}
}

void W2DbAccountToProto(const STRUCT_ACCOUNTFILE& f, const W2Serials& serials, uint64_t version, pb::Account* o)
{
	o->Clear();
	o->set_name(CStr(f.Info.AccountName, ACCOUNTNAME_LENGTH));
	o->set_version(version);
	o->set_cargo_coin(f.Coin);
	o->set_donate(f.Donate);
	o->set_year(f.Info.Year);
	o->set_year_day(f.Info.YearDay);
	o->set_temp_key(Reserved(f.TempKey, sizeof(f.TempKey)));

	for (int slot = 0; slot < MOB_PER_ACCOUNT; slot++)
	{
		const STRUCT_MOB& mob = f.Char[slot];
		if (mob.MobName[0] == 0)
		{
			// Vaga vazia: os bytes que não forem zero, para a conta voltar igual.
			pb::EmptySlot e;
			e.set_mob(Reserved(&mob, sizeof(mob)));
			e.set_short_skill(Reserved(f.ShortSkill[slot], 16));
			e.set_affects(Reserved(f.affect[slot], sizeof(f.affect[slot])));
			e.set_extra(Reserved(&f.mobExtra[slot], sizeof(f.mobExtra[slot])));
			if (!(e.mob().empty() && e.short_skill().empty() && e.affects().empty() && e.extra().empty()))
			{
				e.set_slot(slot);
				*o->add_empty_slots() = e;
			}
			continue;
		}
		pb::Character* c = o->add_characters();
		c->set_slot(slot);
		c->set_name(CStr(mob.MobName, NAME_LENGTH));
		MobToProto(mob, c->mutable_mob());
		ItemsToProto(mob.Equip, serials.Equip[slot], MAX_EQUIP, c->mutable_equip());
		ItemsToProto(mob.Carry, serials.Carry[slot], MAX_CARRY, c->mutable_carry());
		int n = MAX_AFFECT;
		while (n > 0 && IsZero(&f.affect[slot][n - 1], sizeof(STRUCT_AFFECT)))
			n--;
		for (int i = 0; i < n; i++)
		{
			const STRUCT_AFFECT& a = f.affect[slot][i];
			pb::Affect* pa = c->add_affects();
			pa->set_type(a.Type);
			pa->set_level(a.Level);
			pa->set_value(a.Value);
			pa->set_time(a.Time);
		}
		c->set_short_skill(Raw(f.ShortSkill[slot], 16));
		ExtraToProto(f.mobExtra[slot], c->mutable_extra());
	}

	ItemsToProto(f.Cargo, serials.Cargo, MAX_CARGO, o->mutable_cargo());

	const char* t = f.Info.NumericToken;
	o->set_numeric_set(t[0] != -1 && t[0] != 0);
}

bool W2DbProtoToAccount(const pb::Account& o, STRUCT_ACCOUNTFILE* f, W2Serials* serials)
{
	memset(f, 0, sizeof(*f));
	memset(serials, 0, sizeof(*serials));

	if (o.name().size() >= ACCOUNTNAME_LENGTH)
		return false;
	PutBytes(f->Info.AccountName, ACCOUNTNAME_LENGTH, o.name());
	if (!o.numeric_set())
		f->Info.NumericToken[0] = -1;
	f->Info.Year = o.year();
	f->Info.YearDay = o.year_day();

	for (const pb::Character& c : o.characters())
	{
		const unsigned slot = c.slot();
		if (slot >= MOB_PER_ACCOUNT || c.name().size() >= NAME_LENGTH)
			return false;
		STRUCT_MOB& mob = f->Char[slot];
		PutBytes(mob.MobName, NAME_LENGTH, c.name());
		MobFromProto(c.mob(), &mob);
		if (!ItemsFromProto(c.equip(), mob.Equip, serials->Equip[slot], MAX_EQUIP) ||
			!ItemsFromProto(c.carry(), mob.Carry, serials->Carry[slot], MAX_CARRY))
			return false;
		for (int i = 0; i < c.affects_size() && i < MAX_AFFECT; i++)
		{
			STRUCT_AFFECT& a = f->affect[slot][i];
			a.Type = static_cast<unsigned char>(c.affects(i).type());
			a.Level = static_cast<unsigned char>(c.affects(i).level());
			a.Value = static_cast<unsigned short>(c.affects(i).value());
			a.Time = c.affects(i).time();
		}
		PutBytes(f->ShortSkill[slot], 16, c.short_skill());
		ExtraFromProto(c.extra(), &f->mobExtra[slot]);
	}
	for (const pb::EmptySlot& e : o.empty_slots())
	{
		const unsigned slot = e.slot();
		if (slot >= MOB_PER_ACCOUNT || f->Char[slot].MobName[0] != 0)
			return false;
		PutBytes(&f->Char[slot], sizeof(STRUCT_MOB), e.mob());
		PutBytes(f->ShortSkill[slot], 16, e.short_skill());
		PutBytes(f->affect[slot], sizeof(f->affect[slot]), e.affects());
		PutBytes(&f->mobExtra[slot], sizeof(STRUCT_MOBEXTRA), e.extra());
	}

	if (!ItemsFromProto(o.cargo(), f->Cargo, serials->Cargo, MAX_CARGO))
		return false;
	f->Coin = o.cargo_coin();
	f->Donate = o.donate();
	PutBytes(f->TempKey, sizeof(f->TempKey), o.temp_key());
	return true;
}
