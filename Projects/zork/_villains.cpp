// Zork I: The Great Underground Empire
// (c) 1980 by INFOCOM, Inc.
// C port and parser (c) 2021 by Donnie Russell II

// This source code is provided for personal, educational use only.
// You are welcome to use this source code to develop your own works,
// but the story-related content belongs to the original authors of Zork.



#include "def.h"
#include "_enum.h"



#define HERO  0  // never set



int VillainObj[NUM_VILLAINS] = {OBJ_TROLL, OBJ_THIEF, OBJ_CYCLOPS};



extern struct GAMEFLAGS GameFlags;
extern struct GAMEVARS GameVars;
extern struct ROOM_STRUCT Room[NUM_ROOMS];
extern struct OBJ_STRUCT Obj[NUM_OBJECTS];



//*****************************************************************************

void ThiefRecoverStiletto(void)
{
  if (Obj[OBJ_STILETTO].loc == Obj[OBJ_THIEF].loc)
  {
    Obj[OBJ_STILETTO].loc = INSIDE + OBJ_THIEF;

    Obj[OBJ_STILETTO].prop |= PROP_NODESC;
    Obj[OBJ_STILETTO].prop |= PROP_NOTTAKEABLE;
  }
}



int ThiefRob(int loc, int prob)
{
  int flag = 0, obj;

  for (obj=2; obj<NUM_OBJECTS; obj++)
    if (Obj[obj].loc == loc &&
        (Obj[obj].prop & PROP_INVISIBLE) == 0 &&
        (Obj[obj].prop & PROP_SACRED) == 0 &&
        Obj[obj].thiefvalue > 0 &&
        (prob < 0 || PercentChance(prob, -1)))
  {
    flag = 1;
    Obj[obj].loc = INSIDE + OBJ_THIEF;
    Obj[obj].prop |= PROP_MOVEDDESC;
    Obj[obj].prop |= PROP_INVISIBLE;
  }

  return flag;
}



int PlayerFightStrength(int adjust)
{
  int s =  STRENGTH_MIN + (STRENGTH_MAX - STRENGTH_MIN) * Score / SCORE_MAX;

  if (adjust)
    s += PlayerStrength;

  return s;
}



int VillainFightStrength(int i, int player_weapon)
{
  int obj, strength;

  obj = VillainObj[i];
  strength = VillainStrength[i];

  if (strength >= 0)
  {
    int best[NUM_VILLAINS] = {OBJ_SWORD, OBJ_KNIFE, 0};

    if (obj == OBJ_THIEF && ThiefEngrossed)
    {
      ThiefEngrossed = 0;
      if (strength > 2) strength = 2;
    }

    if (player_weapon &&
        (Obj[player_weapon].prop & PROP_WEAPON) &&
        player_weapon == best[i])
    {
      unsigned char advantage[NUM_VILLAINS] = {1, 1, 0};

      strength -= advantage[i];
      if (strength < 1) strength = 1;
    }
  }

  return strength;
}



int ThiefWinning(void)
{
  int vs = VillainStrength[VILLAIN_THIEF];
  int ps = vs - PlayerFightStrength(1);

       if (ps >  3) return PercentChance(90, -1);
  else if (ps >  0) return PercentChance(75, -1);
  else if (ps == 0) return PercentChance(50, -1);
  else if (vs >  1) return PercentChance(25, -1);
  else              return PercentChance(10, -1);
}



int ThiefVsAdventurer(int here)
{
  int prev_darkness;
  int robbed = 0; // 1: player  2: room

  if (YouAreDead == 0 && Obj[OBJ_YOU].loc == ROOM_TREASURE_ROOM)
  {
  }
  else if (ThiefHere == 0)
  {
    if (YouAreDead == 0 && here == 0 && PercentChance(30, -1))
    {
      if (Obj[OBJ_STILETTO].loc == INSIDE + OBJ_THIEF)
      {
        Obj[OBJ_THIEF].prop &= ~PROP_INVISIBLE;
        PrintLineIndex(777);
        ThiefHere = 1;
        return 1;
      }
      else
      {
        Obj[OBJ_STILETTO].loc = INSIDE + OBJ_THIEF;

        Obj[OBJ_STILETTO].prop |= PROP_NODESC;
        Obj[OBJ_STILETTO].prop |= PROP_NOTTAKEABLE;

        Obj[OBJ_THIEF].prop &= ~PROP_INVISIBLE;
        PrintLineIndex(778);
        ThiefHere = 1;
        return 1;
      }
    }
    else if (here && VillainAttacking[VILLAIN_THIEF] && ThiefWinning() == 0)
    {
      PrintLineIndex(779);
      Obj[OBJ_THIEF].prop |= PROP_INVISIBLE;
      VillainAttacking[VILLAIN_THIEF] = 0;
      ThiefRecoverStiletto();
      return 1;
    }
    else if (here && VillainAttacking[VILLAIN_THIEF] && PercentChance(90, -1))
      return 0;
    else if (here && PercentChance(30, -1))
    {
      PrintLineIndex(780);
      Obj[OBJ_THIEF].prop |= PROP_INVISIBLE;
      ThiefRecoverStiletto();
      return 1;
    }
    else if (PercentChance(70, -1))
      return 0;
    else if (YouAreDead == 0)
    {
      prev_darkness = IsPlayerInDarkness();

           if (ThiefRob(Obj[OBJ_YOU].loc, 100))  robbed = 2; // room
      else if (ThiefRob(INSIDE + OBJ_YOU,  -1))  robbed = 1; // player

      ThiefHere = 1;

      if (robbed && here == 0)
      {
        PrintTextIndex(781);

        if (robbed == 2)
          PrintTextIndex(782);
        else
          PrintTextIndex(783);

        PrintLineIndex(784);

        if (IsPlayerInDarkness() != prev_darkness)
          PrintLineIndex(785);
      }
      else if (here)
      {
        ThiefRecoverStiletto();

        if (robbed)
        {
          PrintTextIndex(786);

          if (robbed == 2)
            PrintLineIndex(787);
          else
            PrintLineIndex(788);

          if (IsPlayerInDarkness() != prev_darkness)
            PrintLineIndex(785);
        }
        else
          PrintLineIndex(789);

        Obj[OBJ_THIEF].prop |= PROP_INVISIBLE;
        here = 0;
        return 1;
      }
      else
      {
        PrintLineIndex(790);
        return 1;
      }
    }
  }
  else
  {
    if (here)
    {
      if (PercentChance(30, -1))
      {
        prev_darkness = IsPlayerInDarkness();

             if (ThiefRob(Obj[OBJ_YOU].loc, 100))  robbed = 2; // room
        else if (ThiefRob(INSIDE + OBJ_YOU,  -1))  robbed = 1; // player

        if (robbed)
        {
          PrintTextIndex(786);

          if (robbed == 2)
            PrintLineIndex(787);
          else
            PrintLineIndex(788);

          if (IsPlayerInDarkness() != prev_darkness)
            PrintLineIndex(785);
        }
        else
          PrintLineIndex(789);

        Obj[OBJ_THIEF].prop |= PROP_INVISIBLE;
        here = 0;
        ThiefRecoverStiletto();
      }
    }
  }

  return 0;
}



int ThiefDepositBooty(int room)
{
  int flag = 0, obj;

  for (obj=2; obj<NUM_OBJECTS; obj++)
    if (Obj[obj].loc == INSIDE + OBJ_THIEF &&
        Obj[obj].thiefvalue > 0 &&
        obj != OBJ_STILETTO &&
        obj != OBJ_LARGE_BAG)
  {
    flag = 1;
    Obj[obj].loc = room;
    if (obj == OBJ_EGG)
      Obj[OBJ_EGG].prop |= PROP_OPEN;
  }

  return flag;
}



int ThiefDropJunk(int room)
{
  int flag = 0, obj;

  for (obj=2; obj<NUM_OBJECTS; obj++)
    if (Obj[obj].loc == INSIDE + OBJ_THIEF &&
        Obj[obj].thiefvalue == 0 &&
        PercentChance(30, -1) &&
        obj != OBJ_STILETTO &&
        obj != OBJ_LARGE_BAG)
  {
    if (flag == 0 && room == Obj[OBJ_YOU].loc)
    {
      flag = 1;
      PrintLineIndex(791);
    }
    Obj[obj].loc = room;
    Obj[obj].prop &= ~PROP_INVISIBLE;
  }

  return flag;
}



void ThiefHackTreasures(void)
{
  int obj;

  ThiefRecoverStiletto();

  Obj[OBJ_THIEF].prop |= PROP_INVISIBLE;

  for (obj=2; obj<NUM_OBJECTS; obj++)
    if (Obj[obj].loc == ROOM_TREASURE_ROOM &&
        obj != OBJ_CHALICE &&
        obj != OBJ_THIEF)
  {
    Obj[obj].prop &= ~PROP_INVISIBLE;
  }
}



void ThiefRobMaze(int room)
{
  int obj;

  for (obj=2; obj<NUM_OBJECTS; obj++)
    if (Obj[obj].loc == room &&
        (Obj[obj].prop & PROP_INVISIBLE) == 0 &&
        (Obj[obj].prop & PROP_NOTTAKEABLE) == 0 &&
        PercentChance(40, -1))
  {
    PrintLineIndex(792);
    if (PercentChance(60, 80))
    {
      Obj[obj].loc = INSIDE + OBJ_THIEF;

      Obj[obj].prop |= PROP_MOVEDDESC;

      if (obj == OBJ_STILETTO)
      {
        Obj[OBJ_STILETTO].prop |= PROP_NODESC;
        Obj[OBJ_STILETTO].prop |= PROP_NOTTAKEABLE;
      }
      else
        Obj[obj].prop |= PROP_INVISIBLE;
    }
    break;
  }
}



void ThiefStealJunk(int room)
{
  int obj;

  for (obj=2; obj<NUM_OBJECTS; obj++)
    if (Obj[obj].loc == room &&
        Obj[obj].thiefvalue == 0 &&
        (Obj[obj].prop & PROP_INVISIBLE) == 0 &&
        (Obj[obj].prop & PROP_NOTTAKEABLE) == 0 &&
        (Obj[obj].prop & PROP_SACRED) == 0 &&
        (obj == OBJ_STILETTO || PercentChance(10, -1)))
  {
    Obj[obj].loc = INSIDE + OBJ_THIEF;

    Obj[obj].prop |= PROP_MOVEDDESC;

    if (obj == OBJ_STILETTO)
    {
      Obj[OBJ_STILETTO].prop |= PROP_NODESC;
      Obj[OBJ_STILETTO].prop |= PROP_NOTTAKEABLE;
    }
    else
      Obj[obj].prop |= PROP_INVISIBLE;

    if (obj == OBJ_ROPE) // will never happen because it's sacred
      RopeTiedToRail = 0;

    if (room == Obj[OBJ_YOU].loc)
      PrintLineIndex(793);

    break;
  }
}



void ThiefRoutine(void)
{
  int room, here, once = 0;


  // if thief is dead or unconcious
  if (Obj[OBJ_THIEF].loc == 0 ||
      ThiefDescType == 1) // unconcious
    return;


  for (;;) // used only to allow use of break instead of goto
  {
    room = Obj[OBJ_THIEF].loc;
    here = ((Obj[OBJ_THIEF].prop & PROP_INVISIBLE) == 0);

    if (here)
      room = Obj[OBJ_THIEF].loc;

    if (room == ROOM_TREASURE_ROOM &&
        room != Obj[OBJ_YOU].loc)
    {
      if (here)
      {
        here = 0;
        ThiefHackTreasures();
      }
      ThiefDepositBooty(ROOM_TREASURE_ROOM);
    }
    else if (Obj[OBJ_YOU].loc == room &&
             (Room[room].prop & R_LIT) == 0 &&
             Obj[OBJ_TROLL].loc != Obj[OBJ_YOU].loc)
    {
      if (ThiefVsAdventurer(here))
        break; // break out of for(;;)

      if (Obj[OBJ_THIEF].prop & PROP_INVISIBLE)
        here = 0;
    }
    else
    {
      if (Obj[OBJ_THIEF].loc == room &&
          (Obj[OBJ_THIEF].prop & PROP_INVISIBLE) == 0)
      {
        Obj[OBJ_THIEF].prop |= PROP_INVISIBLE;
        here = 0;
      }

      if (Room[room].prop & R_DESCRIBED)
      {
        ThiefRob(room, 75);

        if ((Room[room].prop & R_MAZE) &&
            (Room[Obj[OBJ_YOU].loc].prop & R_MAZE))
          ThiefRobMaze(room);
        else
          ThiefStealJunk(room);
      }
    }

    once = 1-once;
    if (once && here == 0)
    {
      ThiefRecoverStiletto();

      for (;;)
      {
        room++;
        if (room == NUM_ROOMS) room = 1;

        if ((Room[room].prop & R_SACRED) == 0 &&
            (Room[room].prop & R_BODYOFWATER) == 0)
        {
          Obj[OBJ_THIEF].loc = room;
          Obj[OBJ_THIEF].prop |= PROP_INVISIBLE;
          VillainAttacking[VILLAIN_THIEF] = 0;
          ThiefHere = 0;
          break;
        }
      }
    }

    break; // break out of for(;;)
  }

  if (room != ROOM_TREASURE_ROOM)
    ThiefDropJunk(room);
}

//*****************************************************************************



//*****************************************************************************

void PrintWeaponName(int weapon)
{
  switch (weapon)
  {
    case OBJ_STILETTO:    PrintTextIndex(794);    break;
    case OBJ_AXE:         PrintTextIndex(795);  break;
    case OBJ_SWORD:       PrintTextIndex(796);       break;
    case OBJ_KNIFE:       PrintTextIndex(797); break;
    case OBJ_RUSTY_KNIFE: PrintTextIndex(798); break;
  }
}



void PrintVillainName(int i)
{
  switch (i)
  {
    case VILLAIN_TROLL:   PrintTextIndex(799);   break;
    case VILLAIN_THIEF:   PrintTextIndex(800);   break;
    case VILLAIN_CYCLOPS: PrintTextIndex(801); break;
  }
}



// i:       0 - NUM_VILLAINS-1
// blow:    1 - 9
// weapon:  OBJ_*

void PrintBlowRemark(int player_flag, int i, int blow, int weapon)
{
  int j, index, num, msg;
  unsigned char offset[10 * 4] =
  {
    0, 6, 11, 14, 18, 22, 27, 29, 30, 31,
    0, 4,  5,  8, 12, 15, 19, 22, 24, 25,
    0, 4,  6,  9, 13, 17, 20, 23, 26, 28,
    0, 2,  3,  4,  6,  8, 10, 12, 13, 14
  };

  j = player_flag ? 0 : 1+i;
  index = 10*j + (blow-1);
  num = offset[index+1] - offset[index];
  msg = 100*j + offset[index] + GetRandom(num);

  switch (msg)
  {
    case 100*0 +  0: PrintTextIndex(802); PrintWeaponName(weapon); PrintTextIndex(803); PrintVillainName(i); PrintTextIndex(804); break;
    case 100*0 +  1: PrintTextIndex(805); PrintVillainName(i); PrintTextIndex(806); break;
    case 100*0 +  2: PrintTextIndex(807); PrintVillainName(i); PrintTextIndex(808); break;
    case 100*0 +  3: PrintTextIndex(809); PrintVillainName(i); PrintTextIndex(810); break;
    case 100*0 +  4: PrintTextIndex(811); PrintVillainName(i); PrintTextIndex(812); break;
    case 100*0 +  5: PrintTextIndex(813); PrintVillainName(i); PrintTextIndex(814); break;
    case 100*0 +  6: PrintTextIndex(802); PrintWeaponName(weapon); PrintTextIndex(815); PrintVillainName(i); PrintTextIndex(816); break;
    case 100*0 +  7: PrintTextIndex(252); PrintVillainName(i); PrintTextIndex(817); break;
    case 100*0 +  8: PrintTextIndex(818); PrintVillainName(i); PrintTextIndex(819); break;
    case 100*0 +  9: PrintTextIndex(820); PrintWeaponName(weapon); PrintTextIndex(821); PrintVillainName(i); PrintTextIndex(56); break;
    case 100*0 + 10: PrintTextIndex(252); PrintVillainName(i); PrintTextIndex(819); break;
    case 100*0 + 11: PrintTextIndex(822); PrintVillainName(i); PrintTextIndex(823); PrintWeaponName(weapon); PrintTextIndex(824); break;
    case 100*0 + 12: PrintTextIndex(825); PrintVillainName(i); PrintTextIndex(826); break;
    case 100*0 + 13: PrintTextIndex(252); PrintVillainName(i); PrintTextIndex(827); break;
    case 100*0 + 14: PrintTextIndex(252); PrintVillainName(i); PrintTextIndex(828); break;
    case 100*0 + 15: PrintTextIndex(802); PrintWeaponName(weapon); PrintTextIndex(829); PrintVillainName(i); PrintTextIndex(830); break;
    case 100*0 + 16: PrintTextIndex(831); break;
    case 100*0 + 17: PrintTextIndex(832); PrintVillainName(i); PrintTextIndex(833); break;
    case 100*0 + 18: PrintTextIndex(252); PrintVillainName(i); PrintTextIndex(834); break;
    case 100*0 + 19: PrintTextIndex(835); PrintVillainName(i); PrintTextIndex(836); break;
    case 100*0 + 20: PrintTextIndex(837); break;
    case 100*0 + 21: PrintTextIndex(838); break;
    case 100*0 + 22: PrintTextIndex(252); PrintVillainName(i); PrintTextIndex(839); break;
    case 100*0 + 23: PrintTextIndex(252); PrintVillainName(i); PrintTextIndex(840); break;
    case 100*0 + 24: PrintTextIndex(841); PrintVillainName(i); PrintTextIndex(842); break;
    case 100*0 + 25: PrintTextIndex(252); PrintVillainName(i); PrintTextIndex(843); break;
    case 100*0 + 26: PrintTextIndex(844); PrintVillainName(i); PrintTextIndex(842); break;
    case 100*0 + 27: PrintTextIndex(252); PrintVillainName(i); PrintTextIndex(845); break;
    case 100*0 + 28: PrintTextIndex(252); PrintVillainName(i); PrintTextIndex(846); break;
    case 100*0 + 29: PrintTextIndex(847); break;
    case 100*0 + 30: PrintTextIndex(847); break;

    case 100*1 +  0: PrintTextIndex(848); break;
    case 100*1 +  1: PrintTextIndex(849); break;
    case 100*1 +  2: PrintTextIndex(850); break;
    case 100*1 +  3: PrintTextIndex(851); break;
    case 100*1 +  4: PrintTextIndex(852); break;
    case 100*1 +  5: PrintTextIndex(853); break;
    case 100*1 +  6: PrintTextIndex(854); break;
    case 100*1 +  7: PrintTextIndex(855); break;
    case 100*1 +  8: PrintTextIndex(856); break;
    case 100*1 +  9: PrintTextIndex(857); break;
    case 100*1 + 10: PrintTextIndex(858); break;
    case 100*1 + 11: PrintTextIndex(859); break;
    case 100*1 + 12: PrintTextIndex(860); PrintWeaponName(weapon); PrintTextIndex(861); break;
    case 100*1 + 13: PrintTextIndex(862); break;
    case 100*1 + 14: PrintTextIndex(863); break;
    case 100*1 + 15: PrintTextIndex(864); break;
    case 100*1 + 16: PrintTextIndex(865); break;
    case 100*1 + 17: PrintTextIndex(866); break;
    case 100*1 + 18: PrintTextIndex(867); break;
    case 100*1 + 19: PrintTextIndex(868); PrintWeaponName(weapon); PrintTextIndex(869); break;
    case 100*1 + 20: PrintTextIndex(870); PrintWeaponName(weapon); PrintTextIndex(871); break;
    case 100*1 + 21: PrintTextIndex(872); PrintWeaponName(weapon); PrintTextIndex(873); break;
    case 100*1 + 22: PrintTextIndex(874); break;
    case 100*1 + 23: PrintTextIndex(875); break;
    case 100*1 + 24: PrintTextIndex(876); break;

    case 100*2 +  0: PrintTextIndex(877); break;
    case 100*2 +  1: PrintTextIndex(878); break;
    case 100*2 +  2: PrintTextIndex(879); break;
    case 100*2 +  3: PrintTextIndex(880); break;
    case 100*2 +  4: PrintTextIndex(881); break;
    case 100*2 +  5: PrintTextIndex(882); break;
    case 100*2 +  6: PrintTextIndex(883); break;
    case 100*2 +  7: PrintTextIndex(884); break;
    case 100*2 +  8: PrintTextIndex(885); break;
    case 100*2 +  9: PrintTextIndex(886); break;
    case 100*2 + 10: PrintTextIndex(887); break;
    case 100*2 + 11: PrintTextIndex(888); break;
    case 100*2 + 12: PrintTextIndex(889); break;
    case 100*2 + 13: PrintTextIndex(890); break;
    case 100*2 + 14: PrintTextIndex(891); break;
    case 100*2 + 15: PrintTextIndex(892); break;
    case 100*2 + 16: PrintTextIndex(893); break;
    case 100*2 + 17: PrintTextIndex(894); break;
    case 100*2 + 18: PrintTextIndex(895); break;
    case 100*2 + 19: PrintTextIndex(896); break;
    case 100*2 + 20: PrintTextIndex(897); PrintWeaponName(weapon); PrintTextIndex(898); PrintWeaponName(weapon); PrintTextIndex(899); break;
    case 100*2 + 21: PrintTextIndex(900); PrintWeaponName(weapon); PrintTextIndex(901); break;
    case 100*2 + 22: PrintTextIndex(902); PrintWeaponName(weapon); PrintTextIndex(903); break;
    case 100*2 + 23: PrintTextIndex(904); break;
    case 100*2 + 24: PrintTextIndex(905); break;
    case 100*2 + 25: PrintTextIndex(906); break;
    case 100*2 + 26: PrintTextIndex(907); break;
    case 100*2 + 27: PrintTextIndex(908); break;

    case 100*3 +  0: PrintTextIndex(909); break;
    case 100*3 +  1: PrintTextIndex(910); break;
    case 100*3 +  2: PrintTextIndex(911); break;
    case 100*3 +  3: PrintTextIndex(912); break;
    case 100*3 +  4: PrintTextIndex(913); break;
    case 100*3 +  5: PrintTextIndex(914); break;
    case 100*3 +  6: PrintTextIndex(915); break;
    case 100*3 +  7: PrintTextIndex(916); break;
    case 100*3 +  8: PrintTextIndex(917); break;
    case 100*3 +  9: PrintTextIndex(918); break;
    case 100*3 + 10: PrintTextIndex(919); PrintWeaponName(weapon); PrintTextIndex(920); break;
    case 100*3 + 11: PrintTextIndex(921); PrintWeaponName(weapon); PrintTextIndex(922); break;
    case 100*3 + 12: PrintTextIndex(923); break;
    case 100*3 + 13: PrintTextIndex(924); break;
  }

  PrintNewLine();
}

//-----------------------------------------------------------------------------

enum
{
  BLOW_NULL,
  BLOW_MISSED,         // attacker misses
  BLOW_UNCONSCIOUS,    // defender unconscious
  BLOW_KILLED,         // defender dead
  BLOW_LIGHT_WOUND,    // defender lightly wounded
  BLOW_SERIOUS_WOUND,  // defender seriously wounded
  BLOW_STAGGER,        // defender staggered (miss turn)
  BLOW_LOSE_WEAPON,    // defender loses weapon
  BLOW_HESITATE,       // hesitates (miss on free swing)
  BLOW_SITTING_DUCK    // sitting duck (crunch!)
};



int GetBlow(int attack, int defense)
{
  if (defense == 1)
  {
    int j = attack - 1;

    unsigned char offset[3] = {0, 2, 4};
    unsigned char blow[13] =
    {
      BLOW_MISSED, BLOW_MISSED, BLOW_MISSED, BLOW_MISSED, BLOW_STAGGER, BLOW_STAGGER,
      BLOW_UNCONSCIOUS, BLOW_UNCONSCIOUS, BLOW_KILLED, BLOW_KILLED, BLOW_KILLED,
      BLOW_KILLED, BLOW_KILLED
    };

    if (j < 0) j = 0; else if (j > 2) j = 2;
    return blow[offset[j] + GetRandom(9)];
  }
  else if (defense == 2)
  {
    int j = attack - 1;

    unsigned char offset[4] = {0, 9, 11, 13};
    unsigned char blow[22] =
    {
      BLOW_MISSED, BLOW_MISSED, BLOW_MISSED, BLOW_MISSED, BLOW_MISSED, BLOW_STAGGER,
      BLOW_STAGGER, BLOW_LIGHT_WOUND, BLOW_LIGHT_WOUND, BLOW_MISSED, BLOW_MISSED, BLOW_MISSED,
      BLOW_MISSED, BLOW_STAGGER, BLOW_STAGGER, BLOW_LIGHT_WOUND, BLOW_LIGHT_WOUND,
      BLOW_LIGHT_WOUND, BLOW_UNCONSCIOUS, BLOW_KILLED, BLOW_KILLED, BLOW_KILLED
    };

    if (j < 0) j = 0; else if (j > 3) j = 3;
    return blow[offset[j] + GetRandom(9)];
  }
  else if (defense > 2)
  {
    int j = attack - defense + 2;

    unsigned char offset[5] = {0, 2, 11, 13, 22};
    unsigned char blow[31] =
    {
      BLOW_MISSED, BLOW_MISSED, BLOW_MISSED, BLOW_MISSED, BLOW_MISSED, BLOW_STAGGER, BLOW_STAGGER,
      BLOW_LIGHT_WOUND, BLOW_LIGHT_WOUND, BLOW_SERIOUS_WOUND, BLOW_SERIOUS_WOUND, BLOW_MISSED,
      BLOW_MISSED, BLOW_MISSED, BLOW_STAGGER, BLOW_STAGGER, BLOW_LIGHT_WOUND, BLOW_LIGHT_WOUND,
      BLOW_LIGHT_WOUND, BLOW_SERIOUS_WOUND, BLOW_SERIOUS_WOUND, BLOW_SERIOUS_WOUND, BLOW_MISSED,
      BLOW_STAGGER, BLOW_STAGGER, BLOW_LIGHT_WOUND, BLOW_LIGHT_WOUND, BLOW_LIGHT_WOUND,
      BLOW_SERIOUS_WOUND, BLOW_SERIOUS_WOUND, BLOW_SERIOUS_WOUND
    };

    if (j < 0) j = 0; else if (j > 4) j = 4;
    return blow[offset[j] + GetRandom(9)];
  }

  return 0;
}



// obj is player or villain obj
int FindWeapon(int obj)
{
  int i;
  int weapon[5] = {OBJ_STILETTO, OBJ_AXE, OBJ_SWORD, OBJ_KNIFE, OBJ_RUSTY_KNIFE};

  for (i=0; i<5; i++)
    if (Obj[weapon[i]].loc == INSIDE + obj) return weapon[i];

  return 0;
}



int PlayerResult(int defense, int blow, int original_defense)
{
  PlayerStrength = (defense == 0) ? -10000 : (defense - original_defense);

  if (defense - original_defense < 0)
    EnableCureRoutine = CURE_WAIT;

  if (PlayerFightStrength(1) <= 0)
  {
    PlayerStrength = 1 - PlayerFightStrength(0);
    PrintLineIndex(925);
    YoureDead(); // ##### RIP #####
    return 0;
  }
  else
    return blow;
}



int VillainBlow(int i, int youre_out)
{
  int attack, defense, original_defense, blow, defense_weapon, next_weapon;

  YouAreStaggered = 0;

  if (VillainStaggered[i])
  {
    VillainStaggered[i] = 0;
    PrintTextIndex(252);
    PrintVillainName(i);
    PrintLineIndex(926);
    return 1;
  }

  attack = VillainFightStrength(i, 0); // don't specify player weapon here

  defense = PlayerFightStrength(1);
  if (defense <= 0) return 1;

  original_defense = PlayerFightStrength(0);

  defense_weapon = FindWeapon(OBJ_YOU);

  blow = GetBlow(attack, defense);

  if (youre_out)
  {
    if (blow == BLOW_STAGGER)
      blow = BLOW_HESITATE;
    else
      blow = BLOW_SITTING_DUCK;
  }

  if (blow == BLOW_STAGGER && defense_weapon && PercentChance(25, HERO ? 10 : 50))
    blow = BLOW_LOSE_WEAPON;

  PrintBlowRemark(0, i, blow, defense_weapon); // 0: villain blow


  if (blow == BLOW_MISSED || blow == BLOW_HESITATE)
    {}
  else if (blow == BLOW_UNCONSCIOUS)
    {}
  else if (blow == BLOW_KILLED || blow == BLOW_SITTING_DUCK)
    defense = 0;
  else if (blow == BLOW_LIGHT_WOUND)
  {
    defense--; if (defense < 0) defense = 0;
    if (LoadAllowed > 50) LoadAllowed -= 10;
  }
  else if (blow == BLOW_SERIOUS_WOUND)
  {
    defense -= 2; if (defense < 0) defense = 0;
    if (LoadAllowed > 50) LoadAllowed -= 20;
  }
  else if (blow == BLOW_STAGGER)
    YouAreStaggered = 1;
  else
  {
    Obj[defense_weapon].loc = Obj[OBJ_YOU].loc;

    next_weapon = FindWeapon(OBJ_YOU);
    if (next_weapon)
    {
      PrintTextIndex(927);
      PrintWeaponName(next_weapon);
      PrintLineIndex(56);
    }
  }


  return PlayerResult(defense, blow, original_defense);
}

//-----------------------------------------------------------------------------

int VillainBusy(int i)
{
  if (i == VILLAIN_TROLL)
  {
    if (Obj[OBJ_AXE].loc == INSIDE + OBJ_TROLL)
    {
    }
    else if (Obj[OBJ_AXE].loc == Obj[OBJ_YOU].loc && PercentChance(75, 90))
    {
      Obj[OBJ_AXE].loc = INSIDE + OBJ_TROLL;

      Obj[OBJ_AXE].prop |= PROP_NODESC;
      Obj[OBJ_AXE].prop |= PROP_NOTTAKEABLE;
      Obj[OBJ_AXE].prop &= ~PROP_WEAPON;

      TrollDescType = 0; // default

      if (Obj[OBJ_TROLL].loc == Obj[OBJ_YOU].loc)
        PrintLineIndex(928);
      return 1;
    }
    else if (Obj[OBJ_TROLL].loc == Obj[OBJ_YOU].loc)
    {
      TrollDescType = 2; // unarmed
      PrintLineIndex(929);
      return 1;
    }
  }
  else if (i == VILLAIN_THIEF)
  {
    if (Obj[OBJ_STILETTO].loc == INSIDE + OBJ_THIEF)
    {
    }
    else if (Obj[OBJ_STILETTO].loc == Obj[OBJ_THIEF].loc)
    {
      Obj[OBJ_STILETTO].loc = INSIDE + OBJ_THIEF;

      Obj[OBJ_STILETTO].prop |= PROP_NODESC;
      Obj[OBJ_STILETTO].prop |= PROP_NOTTAKEABLE;

      if (Obj[OBJ_THIEF].loc == Obj[OBJ_YOU].loc)
        PrintLineIndex(930);
      return 1;
    }
  }

  return 0;
}



void VillainDead(int i)
{
  if (i == VILLAIN_TROLL)
  {
    if (Obj[OBJ_AXE].loc == INSIDE + OBJ_TROLL)
    {
      Obj[OBJ_AXE].loc = Obj[OBJ_YOU].loc;

      Obj[OBJ_AXE].prop &= ~PROP_NODESC;
      Obj[OBJ_AXE].prop &= ~PROP_NOTTAKEABLE;
      Obj[OBJ_AXE].prop |= PROP_WEAPON;
    }
    TrollAllowsPassage = 1;
  }
  else if (i == VILLAIN_THIEF)
  {
    int flag;

    Obj[OBJ_STILETTO].loc = Obj[OBJ_YOU].loc;

    Obj[OBJ_STILETTO].prop &= ~PROP_NODESC;
    Obj[OBJ_STILETTO].prop &= ~PROP_NOTTAKEABLE;

    flag = ThiefDepositBooty(Obj[OBJ_YOU].loc);

    if (Obj[OBJ_YOU].loc == ROOM_TREASURE_ROOM)
    {
      int obj;

      for (obj=2; obj<NUM_OBJECTS; obj++)
        if (Obj[obj].loc == ROOM_TREASURE_ROOM &&
            obj != OBJ_CHALICE &&
            obj != OBJ_THIEF)
      {
        Obj[obj].prop &= ~PROP_INVISIBLE;
      }

      Obj[OBJ_CHALICE].prop |= PROP_INVISIBLE;
      PrintPresentObjects(ROOM_TREASURE_ROOM, "As the thief dies, the power of his magic decreases, and his treasures reappear:", 1); // 1: list, no desc
      Obj[OBJ_CHALICE].prop &= ~PROP_INVISIBLE;

      PrintLineIndex(931);
    }
    else if (flag)
      PrintLineIndex(932);
  }
}



int VillainStrikeFirst(int i)
{
  if (i == VILLAIN_TROLL)
  {
    if (PercentChance(33, -1))
    {
      VillainAttacking[i] = 1;
      return 1;
    }
  }
  else if (i == VILLAIN_THIEF)
  {
    if (ThiefHere && (Obj[OBJ_THIEF].prop & PROP_INVISIBLE) == 0 && PercentChance(20, -1))
    {
      VillainAttacking[i] = 1;
      return 1;
    }
  }

  return 0;
}



void VillainUnconcious(int i)
{
  if (i == VILLAIN_TROLL)
  {
    VillainAttacking[i] = 0;

    if (Obj[OBJ_AXE].loc == INSIDE + OBJ_TROLL)
    {
      Obj[OBJ_AXE].loc = Obj[OBJ_YOU].loc;

      Obj[OBJ_AXE].prop &= ~PROP_NODESC;
      Obj[OBJ_AXE].prop &= ~PROP_NOTTAKEABLE;
      Obj[OBJ_AXE].prop |= PROP_WEAPON;
    }

    TrollDescType = 1; // unconcious
    TrollAllowsPassage = 1;
  }
  else if (i == VILLAIN_THIEF)
  {
    VillainAttacking[i] = 0;

    Obj[OBJ_STILETTO].loc = Obj[OBJ_YOU].loc;

    Obj[OBJ_STILETTO].prop &= ~PROP_NODESC;
    Obj[OBJ_STILETTO].prop &= ~PROP_NOTTAKEABLE;

    ThiefDescType = 1; // unconcious
  }
}



void VillainConscious(int i)
{
  if (i == VILLAIN_TROLL)
  {
    if (Obj[OBJ_TROLL].loc == Obj[OBJ_YOU].loc)
    {
      VillainAttacking[i] = 1;
      PrintLineIndex(933);
    }

    if (Obj[OBJ_AXE].loc == INSIDE + OBJ_TROLL)
      TrollDescType = 0; // default
    else if (Obj[OBJ_AXE].loc == ROOM_TROLL_ROOM)
    {
      Obj[OBJ_AXE].loc = INSIDE + OBJ_TROLL;

      Obj[OBJ_AXE].prop |= PROP_NODESC;
      Obj[OBJ_AXE].prop |= PROP_NOTTAKEABLE;
      Obj[OBJ_AXE].prop &= ~PROP_WEAPON;

      TrollDescType = 0; // default
    }
    else
      TrollDescType = 3; // simple description

    TrollAllowsPassage = 0;
  }
  else if (i == VILLAIN_THIEF)
  {
    if (Obj[OBJ_THIEF].loc == Obj[OBJ_YOU].loc)
    {
      VillainAttacking[i] = 1;
      PrintLineIndex(934);
    }

    ThiefDescType = 0; // default
    ThiefRecoverStiletto();
  }
}



void FightRoutine(void)
{
  int i, obj, youre_attacked = 0, youre_out = 0;

  if (YouAreDead)
    return;

  for (i=0; i<NUM_VILLAINS; i++)
  {
    obj = VillainObj[i];

    if (Obj[obj].loc == Obj[OBJ_YOU].loc &&
        (Obj[obj].prop & PROP_INVISIBLE) == 0)
    {
      if (obj == OBJ_THIEF && ThiefEngrossed)
        ThiefEngrossed = 0;
      else if (VillainStrength[i] < 0)
      {
        if (VillainWakingChance[i] != 0 &&
            PercentChance(VillainWakingChance[i], -1))
        {
          VillainWakingChance[i] = 0;
          if (VillainStrength[i] < 0)
          {
            VillainStrength[i] = -VillainStrength[i];
            VillainConscious(i);
          }
        }
        else
          VillainWakingChance[i] += 25;
      }
      else if (VillainAttacking[i] || VillainStrikeFirst(i))
        youre_attacked = 1;
    }
    else
    {
      if (VillainAttacking[i])
        VillainBusy(i);
      if (obj == OBJ_THIEF)
        ThiefEngrossed = 0;
      YouAreStaggered = 0;
      VillainStaggered[i] = 0;
      VillainAttacking[i] = 0;
      if (VillainStrength[i] < 0)
      {
        VillainStrength[i] = -VillainStrength[i];
        VillainConscious(i);
      }
    }
  }

  if (youre_attacked)
    for (;;)
  {
    for (i=0; i<NUM_VILLAINS; i++)
    {
      if (VillainAttacking[i] == 0) {}
      else if (VillainBusy(i)) {}
      else
      {
        int blow = VillainBlow(i, youre_out);

        if (blow == 0) return;
        else if (blow == BLOW_UNCONSCIOUS)
          youre_out = 1 + 1+GetRandom(3);
      }
    }

    if (youre_out) youre_out--;
    if (youre_out == 0) break;
  }
}

//*****************************************************************************



//*****************************************************************************

void CureRoutine(void)
{
  if (EnableCureRoutine == 0) return;
  EnableCureRoutine--;
  if (EnableCureRoutine != 0) return;

       if (PlayerStrength > 0) PlayerStrength = 0;
  else if (PlayerStrength < 0) PlayerStrength++;

  if (PlayerStrength < 0)
  {
    if (LoadAllowed < LOAD_MAX)
      LoadAllowed += 10;
    EnableCureRoutine = CURE_WAIT;
  }
  else
  {
    LoadAllowed = LOAD_MAX;
    EnableCureRoutine = 0;
  }
}

//*****************************************************************************



//*****************************************************************************

void VillainsRoutine(void)
{
  ThiefRoutine();
  FightRoutine();
  CureRoutine();
}

//*****************************************************************************



//*****************************************************************************

void VillainResult(int i, int defense, int blow)
{
  VillainStrength[i] = defense;

  if (defense == 0)
  {
    PrintTextIndex(935);
    PrintVillainName(i);
    PrintLineIndex(936);

    VillainAttacking[i] = 0;
    Obj[VillainObj[i]].loc = 0;

    VillainDead(i);
  }
  else if (blow == BLOW_UNCONSCIOUS)
    VillainUnconcious(i);
}



// obj is thing being attacked by player

void PlayerBlow(int obj, int player_weapon)
{
  int i, attack, defense, defense_weapon, blow;

  for (i=0; i<NUM_VILLAINS; i++)
    if (VillainObj[i] == obj) break;

  if (i < NUM_VILLAINS)
    VillainAttacking[i] = 1;

  if (YouAreStaggered)
  {
    YouAreStaggered = 0;
    PrintLineIndex(937);
    return;
  }

  if (obj == OBJ_YOU)
  {
    PrintLineIndex(938);
    YoureDead(); // ##### RIP #####
    return;
  }

  attack = PlayerFightStrength(1);
  if (attack < 1) attack = 1;

  if (i < NUM_VILLAINS)
    defense = VillainFightStrength(i, player_weapon);
  else
    defense = 0;

  if (defense == 0) // catches case of i == NUM_VILLAINS
  {
    PrintLineIndex(939);
    return;
  }

  defense_weapon = FindWeapon(obj);

  if ((defense_weapon == 0 && obj != OBJ_CYCLOPS) || defense < 0)
  {
    PrintTextIndex(252);
    if (defense < 0) PrintTextIndex(940);
    else             PrintTextIndex(941);
    PrintVillainName(i);
    PrintLineIndex(942);
    blow = BLOW_KILLED;
  }
  else
  {
    blow = GetBlow(attack, defense);

    if (blow == BLOW_STAGGER && defense_weapon && PercentChance(25, -1))
      blow = BLOW_LOSE_WEAPON;

    PrintBlowRemark(1, i, blow, player_weapon); // 1: player blow
  }


  if (blow == BLOW_MISSED || blow == BLOW_HESITATE)
  {
  }
  else if (blow == BLOW_UNCONSCIOUS)
    defense = -defense;
  else if (blow == BLOW_KILLED || blow == BLOW_SITTING_DUCK)
    defense = 0;
  else if (blow == BLOW_LIGHT_WOUND)
  {
    defense--;
    if (defense < 0) defense = 0;
  }
  else if (blow == BLOW_SERIOUS_WOUND)
  {
    defense -= 2;
    if (defense < 0) defense = 0;
  }
  else if (blow == BLOW_STAGGER)
    VillainStaggered[i] = 1;
  else
  {
    Obj[defense_weapon].loc = Obj[OBJ_YOU].loc;

    Obj[defense_weapon].prop &= ~PROP_NODESC;
    Obj[defense_weapon].prop &= ~PROP_NOTTAKEABLE;

    Obj[defense_weapon].prop |= PROP_WEAPON;
  }


  VillainResult(i, defense, blow);
}

//*****************************************************************************



//*****************************************************************************

// call just before player enters treasure room

void ThiefProtectsTreasure(void)
{
  int obj, flag = 0;

  // if thief is dead or unconcious
  if (Obj[OBJ_THIEF].loc == 0 ||
      ThiefDescType == 1) // unconcious
    return;

  if (Obj[OBJ_THIEF].loc != ROOM_TREASURE_ROOM)
  {
    PrintLineIndex(943);
  
    Obj[OBJ_THIEF].loc = ROOM_TREASURE_ROOM;
    Obj[OBJ_THIEF].prop &= ~PROP_INVISIBLE;
  
    VillainAttacking[VILLAIN_THIEF] = 1;
  
    for (obj=2; obj<NUM_OBJECTS; obj++)
      if (Obj[obj].loc == ROOM_TREASURE_ROOM &&
          obj != OBJ_CHALICE &&
          obj != OBJ_THIEF)
    {
      if (flag == 0)
      {
        flag = 1;
        PrintLineIndex(944);
      }
  
      Obj[obj].prop |= PROP_INVISIBLE;
    }

    PrintNewLine();
  }
}

//*****************************************************************************
