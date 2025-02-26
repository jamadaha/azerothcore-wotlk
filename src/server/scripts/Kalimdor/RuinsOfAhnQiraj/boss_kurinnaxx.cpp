/*
 * This file is part of the AzerothCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "CreatureTextMgr.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ruins_of_ahnqiraj.h"

enum Spells
{
    SPELL_MORTALWOUND       = 25646,
    SPELL_SANDTRAP          = 25648,
    SPELL_ENRAGE            = 26527,
    SPELL_SUMMON_PLAYER     = 26446,
    SPELL_WIDE_SLASH        = 25814
};

enum Events
{
    EVENT_MORTAL_WOUND      = 1,
    EVENT_SANDTRAP          = 2,
    EVENT_WIDE_SLASH        = 3
};

enum Texts
{
    SAY_KURINAXX_DEATH      = 5 // Yell by 'Ossirian the Unscarred'
};

class boss_kurinnaxx : public CreatureScript
{
public:
    boss_kurinnaxx() : CreatureScript("boss_kurinnaxx") { }

    struct boss_kurinnaxxAI : public BossAI
    {
        boss_kurinnaxxAI(Creature* creature) : BossAI(creature, DATA_KURINNAXX) {}

        void Reset() override
        {
            _Reset();
            _enraged = false;
            events.ScheduleEvent(EVENT_MORTAL_WOUND, urand(8000, 10000));
            events.ScheduleEvent(EVENT_SANDTRAP, urand(5000, 15000));
            events.ScheduleEvent(EVENT_WIDE_SLASH, urand(10000, 15000));
        }

        void DamageTaken(Unit*, uint32& /*damage*/, DamageEffectType, SpellSchoolMask) override
        {
            if (!_enraged && HealthBelowPct(30))
            {
                DoCastSelf(SPELL_ENRAGE);
                _enraged = true;
            }
        }

        void JustDied(Unit* /*killer*/) override
        {
            _JustDied();
            if (Creature* Ossirian = me->GetMap()->GetCreature(instance->GetGuidData(DATA_OSSIRIAN)))
            {
                sCreatureTextMgr->SendChat(Ossirian, SAY_KURINAXX_DEATH, nullptr, CHAT_MSG_ADDON, LANG_ADDON, TEXT_RANGE_ZONE);
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_MORTAL_WOUND:
                        DoCastVictim(SPELL_MORTALWOUND);
                        events.ScheduleEvent(EVENT_MORTAL_WOUND, urand(8000, 10000));
                        break;
                    case EVENT_SANDTRAP:
                        if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0, 100, true))
                        {
                            target->CastSpell(target, SPELL_SANDTRAP, true);
                        }
                        events.ScheduleEvent(EVENT_SANDTRAP, urand(5000, 15000));
                        break;
                    case EVENT_WIDE_SLASH:
                        DoCastSelf(SPELL_WIDE_SLASH);
                        events.ScheduleEvent(EVENT_WIDE_SLASH, urand(12000, 15000));
                        break;
                    default:
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    private:
        bool _enraged;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return GetRuinsOfAhnQirajAI<boss_kurinnaxxAI>(creature);
    }
};

void AddSC_boss_kurinnaxx()
{
    new boss_kurinnaxx();
}
