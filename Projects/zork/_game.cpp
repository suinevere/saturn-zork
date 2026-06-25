// Zork I: The Great Underground Empire
// (c) 1980 by INFOCOM, Inc.
// C port and parser (c) 2021 by Donnie Russell II

// This source code is provided for personal, educational use only.
// You are welcome to use this source code to develop your own works,
// but the story-related content belongs to the original authors of Zork.



#include "def.h"
#include "_enum.h"



//game state
struct GAMEFLAGS GameFlags;
struct GAMEVARS GameVars;
struct ROOM_STRUCT Room[NUM_ROOMS];
struct OBJ_STRUCT Obj[NUM_OBJECTS];



//from parser.c
extern int CurWord;
extern int NumStrWords;
extern unsigned int StrWord[];
extern int ItObj;
extern unsigned char TimePassed;
extern unsigned char GameOver;



//*****************************************************************************
// returns 1 if event of x% chance occurred
// second parameter is used instead if it is >=0 and you're not lucky
int PercentChance(int x, int x_not_lucky)
{
  if (NotLucky && x_not_lucky >= 0) x = x_not_lucky;

  if (GetRandom(100) < x) return 1;
  else return 0;
}
//*****************************************************************************



//*****************************************************************************
void ScatterInventory(void)
{
  int obj;

  if (Obj[OBJ_LAMP].loc == INSIDE + OBJ_YOU)
    Obj[OBJ_LAMP].loc = ROOM_LIVING_ROOM;

  if (Obj[OBJ_COFFIN].loc == INSIDE + OBJ_YOU)
    Obj[OBJ_COFFIN].loc = ROOM_EGYPT_ROOM;

  Obj[OBJ_SWORD].thiefvalue = 0;


  for (obj=2; obj<NUM_OBJECTS; obj++)
    if (Obj[obj].loc == INSIDE + OBJ_YOU)
  {
    int room = NUM_ROOMS;

    if (Obj[obj].thiefvalue > 0)
      for (room=1; room<NUM_ROOMS; room++)
        if ((Room[room].prop & R_BODYOFWATER) == 0 &&
            (Room[room].prop & R_LIT) == 0 &&
            GetRandom(2) == 0) break;

    if (room == NUM_ROOMS)
    {
      int above_ground[11] =
        { ROOM_WEST_OF_HOUSE, ROOM_NORTH_OF_HOUSE, ROOM_EAST_OF_HOUSE,
          ROOM_SOUTH_OF_HOUSE, ROOM_FOREST_1, ROOM_FOREST_2, ROOM_FOREST_3,
          ROOM_PATH, ROOM_CLEARING, ROOM_GRATING_CLEARING, ROOM_CANYON_VIEW };

      room = above_ground[GetRandom(11)];
    }

    Obj[obj].loc = room;
  }
}



void YoureDead(void)
{
  if (YouAreDead)
  {
    PrintLineIndex(0);
    GameOver = 1;
    return;
  }

  if (NotLucky)
    PrintLineIndex(1);

  PrintLineIndex(2);

  NumDeaths++;
  if (NumDeaths == 3)
  {
    PrintLineIndex(3);
    GameOver = 1;
    return;
  }

  YouAreInBoat = 0; // in case you're in it
  ExitFound = 1;
  ScatterInventory();

  if (Room[ROOM_SOUTH_TEMPLE].prop & R_DESCRIBED)
  {
    PrintLineIndex(4);

    YouAreDead = 1;
    TrollAllowsPassage = 1;
    Obj[OBJ_LAMP].prop |= PROP_INVISIBLE;
    Obj[OBJ_YOU].prop |= PROP_LIT;
    Obj[OBJ_YOU].loc = ROOM_ENTRANCE_TO_HADES;
    PrintPlayerRoomDesc(0);
  }
  else
  {
    PrintLineIndex(5);

    Obj[OBJ_YOU].loc = ROOM_FOREST_1;
    PrintPlayerRoomDesc(0);
  }
}
//*****************************************************************************



//*****************************************************************************
//these functions return 1 if action completed; otherwise fall through



int GoToRoutine(int newroom)
{
  int prev_darkness;

  if (YouAreInBoat)
  {
    PrintLineIndex(6);
    return 1;
  }

  prev_darkness = IsPlayerInDarkness();

  Obj[OBJ_YOU].loc = newroom;
  TimePassed = 1;

  if (IsPlayerInDarkness())
  {
    if (prev_darkness)
    {
      //kill player that tried to walk from dark to dark
      PrintLineIndex(7);
      YoureDead(); // ##### RIP #####
      return 1;
    }
    else PrintLineIndex(8);
  }

  PrintPlayerRoomDesc(0);
  return 1;
}



int GoFrom_StoneBarrow_West(void)
{
  PrintLineIndex(9);
  GameOver = 1;
  return 1;
}



int GoFrom_WestOfHouse_Southwest(void)
{
  if (WonGame == 0) return 0;
  else return GoToRoutine(ROOM_STONE_BARROW);
}



int GoFrom_EastOfHouse_West(void)
{
  if (KitchenWindowOpen == 0)
  {
    PrintLineIndex(10);
    ItObj = FOBJ_KITCHEN_WINDOW;
    return 1;
  }
  else return GoToRoutine(ROOM_KITCHEN);
}



int GoFrom_Kitchen_East(void)
{
  if (KitchenWindowOpen == 0)
  {
    PrintLineIndex(10);
    ItObj = FOBJ_KITCHEN_WINDOW;
    return 1;
  }
  else return GoToRoutine(ROOM_EAST_OF_HOUSE);
}



int GoFrom_LivingRoom_West(void)
{
  if (CyclopsState == 4) // fled
    return GoToRoutine(ROOM_STRANGE_PASSAGE);
  else
    {PrintLineIndex(11); return 1;}
}



int GoFrom_Cellar_Up(void)
{
  if (TrapOpen == 0)
  {
    PrintLineIndex(12);
    ItObj = FOBJ_TRAP_DOOR;
  }
  else
    return GoToRoutine(ROOM_LIVING_ROOM);

  return 1;
}



int GoFrom_TrollRoom_East(void)
{
  if (TrollAllowsPassage == 0) {PrintLineIndex(13); return 1;}
  else return GoToRoutine(ROOM_EW_PASSAGE);
}



int GoFrom_TrollRoom_West(void)
{
  if (TrollAllowsPassage == 0) {PrintLineIndex(13); return 1;}
  else return GoToRoutine(ROOM_MAZE_1);
}



int GoFrom_GratingRoom_Up(void)
{
  if (GratingOpen == 0)
  {
    PrintLineIndex(14);
    ItObj = FOBJ_GRATE;
  }
  else
  {
    ExitFound = 1;
    return GoToRoutine(ROOM_GRATING_CLEARING);
  }

  return 1;
}



int GoFrom_CyclopsRoom_East(void)
{
  if (CyclopsState == 4) // fled
    return GoToRoutine(ROOM_STRANGE_PASSAGE);
  else
    {PrintLineIndex(15); return 1;}
}



int GoFrom_CyclopsRoom_Up(void)
{
  if (CyclopsState == 3 || Obj[OBJ_CYCLOPS].loc == 0) // sleeping or dead
  {
    if (YouAreInBoat == 0) ThiefProtectsTreasure();
    return GoToRoutine(ROOM_TREASURE_ROOM);
  }
  else
    PrintLineIndex(16);

  return 1;
}



int GoFrom_ReservoirSouth_North(void)
{
  if (LowTide == 0) {PrintLineIndex(17); return 1;}
  else return GoToRoutine(ROOM_RESERVOIR);
}



int GoFrom_ReservoirNorth_South(void)
{
  if (LowTide == 0) {PrintLineIndex(17); return 1;}
  else return GoToRoutine(ROOM_RESERVOIR);
}



int GoFrom_EntranceToHades_South(void)
{
  if (SpiritsBanished == 0) {PrintLineIndex(18); return 1;}
  else return GoToRoutine(ROOM_LAND_OF_LIVING_DEAD);
}



int GoFrom_DomeRoom_Down(void)
{
  if (RopeTiedToRail == 0) {PrintLineIndex(19); return 1;}
  else return GoToRoutine(ROOM_TORCH_ROOM);
}



int GoFrom_OntoRainbowRoutine(void)
{
  if (RainbowSolid == 0) return 0;
  else return GoToRoutine(ROOM_ON_RAINBOW);
}



int GoFrom_Maze2_Down(void)
{
  PrintLineIndex(20);
  return GoToRoutine(ROOM_MAZE_4);
}



int GoFrom_Maze7_Down(void)
{
  PrintLineIndex(20);
  return GoToRoutine(ROOM_DEAD_END_1);
}



int GoFrom_Maze9_Down(void)
{
  PrintLineIndex(20);
  return GoToRoutine(ROOM_MAZE_11);
}



int GoFrom_Maze12_Down(void)
{
  PrintLineIndex(20);
  return GoToRoutine(ROOM_MAZE_5);
}



int GoFrom_GratingClearing_Down(void)
{
  if (GratingRevealed == 0)
    PrintBlockMsg(BL0);
  else
  {
    if (GratingOpen == 0)
    {
      PrintLineIndex(14);
      ItObj = FOBJ_GRATE;
    }
    else
      return GoToRoutine(ROOM_GRATING_ROOM);
  }

  return 1;
}



int GoFrom_LivingRoom_Down(void)
{
  if (TrapOpen)
  {
    if (YouAreInBoat)
      PrintLineIndex(6);
    else
    {
      GoToRoutine(ROOM_CELLAR);
      if (YouAreDead == 0 && ExitFound == 0)
      {
        TrapOpen = 0;
        PrintLineIndex(21);
      }
    }
  }
  else if (RugMoved == 0)
    PrintBlockMsg(BL0);
  else
  {
    PrintLineIndex(12);
    ItObj = FOBJ_TRAP_DOOR;
  }

  return 1;
}



int GoFrom_SouthTemple_Down(void)
{
  if (Obj[OBJ_COFFIN].loc == INSIDE + OBJ_YOU)
    {PrintLineIndex(22); return 1;}
  else return GoToRoutine(ROOM_TINY_CAVE);
}



int GoFrom_WhiteCliffsNorth_South(void)
{
  if (Obj[OBJ_INFLATED_BOAT].loc == INSIDE + OBJ_YOU)
    {PrintLineIndex(23); return 1;}
  else return GoToRoutine(ROOM_WHITE_CLIFFS_SOUTH);
}



int GoFrom_WhiteCliffsNorth_West(void)
{
  if (Obj[OBJ_INFLATED_BOAT].loc == INSIDE + OBJ_YOU)
    {PrintLineIndex(23); return 1;}
  else return GoToRoutine(ROOM_DAMP_CAVE);
}



int GoFrom_WhiteCliffsSouth_North(void)
{
  if (Obj[OBJ_INFLATED_BOAT].loc == INSIDE + OBJ_YOU)
    {PrintLineIndex(23); return 1;}
  else return GoToRoutine(ROOM_WHITE_CLIFFS_NORTH);
}



int GoFrom_TimberRoom_West(void)
{
  if (YouAreDead)
    {PrintLineIndex(24); return 1;}
  else if (GetNumObjectsInLocation(INSIDE + OBJ_YOU) > 0)
    {PrintLineIndex(25); return 1;}
  else return GoToRoutine(ROOM_LOWER_SHAFT);
}



int GoFrom_LowerShaft_East(void)
{
  if (GetNumObjectsInLocation(INSIDE + OBJ_YOU) > 0)
    {PrintLineIndex(25); return 1;}
  else return GoToRoutine(ROOM_TIMBER_ROOM);
}



int GoFrom_Kitchen_Down(void)
{
  if (YouAreSanta == 0)
    PrintLineIndex(26);
  else
    return GoToRoutine(ROOM_STUDIO);

  return 1;
}



int GoFrom_Studio_Up(void)
{
  int count = GetNumObjectsInLocation(INSIDE + OBJ_YOU);

  if (count == 0)
    PrintLineIndex(27);
  else if (count < 3 && Obj[OBJ_LAMP].loc == INSIDE + OBJ_YOU)
    return GoToRoutine(ROOM_KITCHEN);
  else
    PrintLineIndex(28);

  return 1;
}



int GoFrom_LandOfLivingDead_North(void)
{
  return GoToRoutine(ROOM_ENTRANCE_TO_HADES);
}



int GoFrom_StrangePassage_West(void)
{
  return GoToRoutine(ROOM_CYCLOPS_ROOM);
}



int GoFrom_NorthTemple_North(void)
{
  return GoToRoutine(ROOM_TORCH_ROOM);
}



int GoFrom_MineEntrance_West(void)
{
  return GoToRoutine(ROOM_SQUEEKY_ROOM);
}



int GoFrom_DamLobby_North_Or_East(void)
{
  if (MaintenanceWaterLevel > 14) {PrintLineIndex(29); return 1;}
  else return GoToRoutine(ROOM_MAINTENANCE_ROOM);
}



int CallGoFrom(int i)
{
  switch (i)
  {
    case  0: return GoFrom_StoneBarrow_West       ();
    case  1: return GoFrom_StoneBarrow_West       ();
    case  2: return GoFrom_WestOfHouse_Southwest  ();
    case  3: return GoFrom_WestOfHouse_Southwest  ();
    case  4: return GoFrom_EastOfHouse_West       ();
    case  5: return GoFrom_EastOfHouse_West       ();
    case  6: return GoFrom_Kitchen_East           ();
    case  7: return GoFrom_Kitchen_East           ();
    case  8: return GoFrom_LivingRoom_West        ();
    case  9: return GoFrom_Cellar_Up              ();
    case 10: return GoFrom_TrollRoom_East         ();
    case 11: return GoFrom_TrollRoom_West         ();
    case 12: return GoFrom_GratingRoom_Up         ();
    case 13: return GoFrom_CyclopsRoom_East       ();
    case 14: return GoFrom_CyclopsRoom_Up         ();
    case 15: return GoFrom_ReservoirSouth_North   ();
    case 16: return GoFrom_ReservoirNorth_South   ();
    case 17: return GoFrom_EntranceToHades_South  ();
    case 18: return GoFrom_EntranceToHades_South  ();
    case 19: return GoFrom_DomeRoom_Down          ();
    case 20: return GoFrom_OntoRainbowRoutine     ();
    case 21: return GoFrom_OntoRainbowRoutine     ();
    case 22: return GoFrom_OntoRainbowRoutine     ();
    case 23: return GoFrom_OntoRainbowRoutine     ();
    case 24: return GoFrom_OntoRainbowRoutine     ();
    case 25: return GoFrom_Maze2_Down             ();
    case 26: return GoFrom_Maze7_Down             ();
    case 27: return GoFrom_Maze9_Down             ();
    case 28: return GoFrom_Maze12_Down            ();
    case 29: return GoFrom_GratingClearing_Down   ();
    case 30: return GoFrom_LivingRoom_Down        ();
    case 31: return GoFrom_SouthTemple_Down       ();
    case 32: return GoFrom_WhiteCliffsNorth_South ();
    case 33: return GoFrom_WhiteCliffsNorth_West  ();
    case 34: return GoFrom_WhiteCliffsSouth_North ();
    case 35: return GoFrom_TimberRoom_West        ();
    case 36: return GoFrom_LowerShaft_East        ();
    case 37: return GoFrom_LowerShaft_East        ();
    case 38: return GoFrom_Kitchen_Down           ();
    case 39: return GoFrom_Studio_Up              ();
    case 40: return GoFrom_LandOfLivingDead_North ();
    case 41: return GoFrom_StrangePassage_West    ();
    case 42: return GoFrom_NorthTemple_North      ();
    case 43: return GoFrom_MineEntrance_West      ();
    case 44: return GoFrom_DamLobby_North_Or_East ();
    case 45: return GoFrom_DamLobby_North_Or_East ();
  }

  return 0; // should never be reached
}
//*****************************************************************************



//*****************************************************************************
void PrintDesc_LivingRoom(void)
{
  if ((Room[ROOM_LIVING_ROOM].prop & R_DESCRIBED) == 0)
  {
    PrintTextIndex(30);

    if (CyclopsState == 4) // fled
      PrintTextIndex(31);
    else
      PrintTextIndex(32);

    PrintTextIndex(33);

    if (RugMoved)
    {
      if (TrapOpen)
        PrintLineIndex(34);
      else
        PrintLineIndex(35);
    }
    else
    {
      if (TrapOpen)
        PrintLineIndex(36);
      else
        PrintLineIndex(37);
    }
  }

  if (Obj[OBJ_TROPHY_CASE].prop & PROP_OPEN)
    PrintContents(OBJ_TROPHY_CASE, "Your collection of treasures consists of:", 0);
}



void PrintDesc_EastOfHouse(void)
{
  if ((Room[ROOM_EAST_OF_HOUSE].prop & R_DESCRIBED) == 0)
  {
    PrintTextIndex(38);

    if (KitchenWindowOpen)
      PrintLineIndex(39);
    else
      PrintLineIndex(40);
  }
}



void PrintDesc_Kitchen(void)
{
  if ((Room[ROOM_KITCHEN].prop & R_DESCRIBED) == 0)
  {
    PrintTextIndex(41);

    if (KitchenWindowOpen)
      PrintLineIndex(39);
    else
      PrintLineIndex(40);
  }

  PrintContents(OBJ_KITCHEN_TABLE, "On the table you see:", 0);
}



void PrintDesc_Attic(void)
{
  if ((Room[ROOM_ATTIC].prop & R_DESCRIBED) == 0)
    PrintLineIndex(42);

  PrintContents(OBJ_ATTIC_TABLE, "On a table you see:", 0);
}



void PrintDesc_GratingClearing(void)
{
  if ((Room[ROOM_GRATING_CLEARING].prop & R_DESCRIBED) == 0)
    PrintLineIndex(43);

  if (GratingRevealed)
  {
    if (GratingOpen)
      PrintLineIndex(44);
    else
      PrintLineIndex(45);
  }
}



void PrintDesc_GratingRoom(void)
{
  if ((Room[ROOM_GRATING_ROOM].prop & R_DESCRIBED) == 0)
    PrintLineIndex(46);

  if (GratingOpen)
    PrintLineIndex(47);
  else
    PrintLineIndex(48);
}



void PrintDesc_DamRoom(void)
{
  if ((Room[ROOM_DAM_ROOM].prop & R_DESCRIBED) == 0)
  {
    PrintLineIndex(49);

    if (GatesOpen)
    {
      if (LowTide)
        PrintLineIndex(50);
      else
        PrintLineIndex(51);
    }
    else
    {
      if (LowTide)
        PrintLineIndex(52);
      else
        PrintLineIndex(53);
    }
  }

  PrintTextIndex(54);

  if (GatesButton)
    PrintTextIndex(55);

  PrintLineIndex(56);
}



void PrintDesc_ReservoirSouth(void)
{
  if ((Room[ROOM_RESERVOIR_SOUTH].prop & R_DESCRIBED) == 0)
  {
    if (GatesOpen)
    {
      if (LowTide)
        PrintLineIndex(57);
      else
        PrintLineIndex(58);
    }
    else
    {
      if (LowTide)
        PrintLineIndex(59);
      else
        PrintLineIndex(60);
    }

    PrintLineIndex(61);
  }
}



void PrintDesc_Reservoir(void)
{
  if ((Room[ROOM_RESERVOIR].prop & R_DESCRIBED) == 0)
  {
    if (LowTide)
    {
      if (GatesOpen == 0 && YouAreInBoat == 0)
        PrintLineIndex(62);
      else
        PrintLineIndex(63);
    }
    else
      PrintLineIndex(64);
  }
}



void PrintDesc_ReservoirNorth(void)
{
  if ((Room[ROOM_RESERVOIR_NORTH].prop & R_DESCRIBED) == 0)
  {
    if (GatesOpen)
    {
      if (LowTide)
        PrintLineIndex(65);
      else
        PrintLineIndex(66);
    }
    else
    {
      if (LowTide)
        PrintLineIndex(67);
      else
        PrintLineIndex(68);
    }

    PrintLineIndex(69);
  }
}



void PrintDesc_LoudRoom(void)
{
  if ((Room[ROOM_LOUD_ROOM].prop & R_DESCRIBED) == 0)
  {
    PrintTextIndex(70);

    if (LoudRoomQuiet || (GatesOpen == 0 && LowTide))
      PrintLineIndex(71);
    else
      PrintLineIndex(72);
  }
}



void PrintDesc_DeepCanyon(void)
{
  if ((Room[ROOM_DEEP_CANYON].prop & R_DESCRIBED) == 0)
  {
    PrintTextIndex(73);

    if (GatesOpen)
    {
      if (LowTide)
        PrintLineIndex(74);
      else
        PrintLineIndex(75);
    }
    else
    {
      if (LowTide)
        PrintNewLine();
      else
        PrintLineIndex(74);
    }
  }
}



void PrintDesc_MachineRoom(void)
{
  if ((Room[ROOM_MACHINE_ROOM].prop & R_DESCRIBED) == 0)
  {
    PrintTextIndex(76);

    if (Obj[OBJ_MACHINE].prop & PROP_OPEN)
      PrintLineIndex(39);
    else
      PrintLineIndex(77);
  }
}



void PrintDesc_AragainFalls(void)
{
  if ((Room[ROOM_ARAGAIN_FALLS].prop & R_DESCRIBED) == 0)
    PrintLineIndex(78);

  if (RainbowSolid)
    PrintLineIndex(79);
  else
    PrintLineIndex(80);
}



void PrintDesc_WestOfHouse(void)
{
  if ((Room[ROOM_WEST_OF_HOUSE].prop & R_DESCRIBED) == 0)
  {
    PrintTextIndex(81);

    if (WonGame)
      PrintLineIndex(82);
    else
      PrintNewLine();
  }
}



void PrintDesc_MirrorRoom1(void)
{
  if ((Room[ROOM_MIRROR_ROOM_1].prop & R_DESCRIBED) == 0)
    PrintLineIndex(83);

  if (MirrorBroken)
    PrintLineIndex(84);
}



void PrintDesc_MirrorRoom2(void)
{
  if ((Room[ROOM_MIRROR_ROOM_2].prop & R_DESCRIBED) == 0)
    PrintLineIndex(83);

  if (MirrorBroken)
    PrintLineIndex(84);
}



void PrintDesc_TorchRoom(void)
{
  if ((Room[ROOM_TORCH_ROOM].prop & R_DESCRIBED) == 0)
    PrintLineIndex(85);

  if (RopeTiedToRail)
    PrintLineIndex(86);
}



void PrintDesc_DomeRoom(void)
{
  if ((Room[ROOM_DOME_ROOM].prop & R_DESCRIBED) == 0)
    PrintLineIndex(87);

  if (RopeTiedToRail)
    PrintLineIndex(88);
}



void PrintDesc_CyclopsRoom(void)
{
  if ((Room[ROOM_CYCLOPS_ROOM].prop & R_DESCRIBED) == 0)
    PrintLineIndex(89);

  if (CyclopsState == 4)
    PrintLineIndex(90);
}



void PrintDesc_UpATree(void)
{
  if ((Room[ROOM_UP_A_TREE].prop & R_DESCRIBED) == 0)
    PrintLineIndex(91);

  PrintPresentObjects(ROOM_PATH, "On the ground below you can see:", 1); // 1: list, no desc
}



int OverrideRoomDesc(int room)
{
  switch (room)
  {
    case ROOM_LIVING_ROOM       : PrintDesc_LivingRoom      (); return 1;
    case ROOM_EAST_OF_HOUSE     : PrintDesc_EastOfHouse     (); return 1;
    case ROOM_KITCHEN           : PrintDesc_Kitchen         (); return 1;
    case ROOM_ATTIC             : PrintDesc_Attic           (); return 1;
    case ROOM_GRATING_CLEARING  : PrintDesc_GratingClearing (); return 1;
    case ROOM_GRATING_ROOM      : PrintDesc_GratingRoom     (); return 1;
    case ROOM_DAM_ROOM          : PrintDesc_DamRoom         (); return 1;
    case ROOM_RESERVOIR_SOUTH   : PrintDesc_ReservoirSouth  (); return 1;
    case ROOM_RESERVOIR         : PrintDesc_Reservoir       (); return 1;
    case ROOM_RESERVOIR_NORTH   : PrintDesc_ReservoirNorth  (); return 1;
    case ROOM_LOUD_ROOM         : PrintDesc_LoudRoom        (); return 1;
    case ROOM_DEEP_CANYON       : PrintDesc_DeepCanyon      (); return 1;
    case ROOM_MACHINE_ROOM      : PrintDesc_MachineRoom     (); return 1;
    case ROOM_ARAGAIN_FALLS     : PrintDesc_AragainFalls    (); return 1;
    case ROOM_WEST_OF_HOUSE     : PrintDesc_WestOfHouse     (); return 1;
    case ROOM_MIRROR_ROOM_1     : PrintDesc_MirrorRoom1     (); return 1;
    case ROOM_MIRROR_ROOM_2     : PrintDesc_MirrorRoom2     (); return 1;
    case ROOM_TORCH_ROOM        : PrintDesc_TorchRoom       (); return 1;
    case ROOM_DOME_ROOM         : PrintDesc_DomeRoom        (); return 1;
    case ROOM_CYCLOPS_ROOM      : PrintDesc_CyclopsRoom     (); return 1;
    case ROOM_UP_A_TREE         : PrintDesc_UpATree         (); return 1;
  }

  return 0;
}
//*****************************************************************************



//*****************************************************************************
// end newline handled by calling function if desc_flag == 0



void PrintDesc_Ghosts(int desc_flag)
{
  if (desc_flag == 0)
    PrintTextIndex(92);
  else
  {
    if (YouAreDead == 0)
      PrintLineIndex(93);
  }
}



void PrintDesc_Bat(int desc_flag)
{
  if (desc_flag == 0)
    PrintTextIndex(94);
  else
  {
    if (IsObjVisible(OBJ_GARLIC))
      PrintLineIndex(95);
    else
      PrintLineIndex(96);
  }
}



void PrintDesc_Troll(int desc_flag)
{
  if (desc_flag == 0)
    PrintTextIndex(97);
  else
    switch (TrollDescType)
  {
    case 0: PrintLineIndex(98); break;
    case 1: PrintLineIndex(99); break;
    case 2: PrintLineIndex(100); break;
    case 3: PrintLineIndex(101); break;
  }
}



void PrintDesc_Thief(int desc_flag)
{
  if (desc_flag == 0)
    PrintTextIndex(102);
  else
    switch (ThiefDescType)
  {
    case 0: PrintLineIndex(103); break;
    case 1: PrintLineIndex(104); break;
  }
}



void PrintDesc_Cyclops(int desc_flag)
{
  if (desc_flag == 0)
    PrintTextIndex(105);
  else
    switch (CyclopsState)
  {
    case 0: PrintLineIndex(106); break;
    case 1: PrintLineIndex(107); break;
    case 2: PrintLineIndex(108); break;
    case 3: PrintLineIndex(109); break;
  }
}



void PrintDesc_InflatedBoat(int desc_flag)
{
  if (desc_flag == 0)
    PrintTextIndex(110);
  else
  {
    if (YouAreInBoat)
      PrintLineIndex(111);
    else
      PrintLineIndex(112);
  }
}



void PrintDesc_Lamp(int desc_flag)
{
  const char *name;

  if (Obj[OBJ_LAMP].prop & PROP_LIT)
    name = "a lit brass lantern";
  else
    name = "a brass lantern";

  if (desc_flag == 0)
    PrintText(name);
  else
  {
    if (Obj[OBJ_LAMP].prop & PROP_MOVEDDESC)
      {PrintTextIndex(113); PrintText(name); PrintLineIndex(114);}
    else
      PrintLineIndex(115);
  }
}



void PrintDesc_Candles(int desc_flag)
{
  const char *name;

  if (Obj[OBJ_CANDLES].prop & PROP_LIT)
    name = "a pair of burning candles";
  else
    name = "a pair of candles";

  if (desc_flag == 0)
    PrintText(name);
  else
  {
    if (Obj[OBJ_CANDLES].prop & PROP_MOVEDDESC)
      {PrintTextIndex(113); PrintText(name); PrintLineIndex(116);}
    else
      PrintLineIndex(117);
  }
}



int OverrideObjectDesc(int obj, int desc_flag)
{
  switch (obj)
  {
    case OBJ_GHOSTS        : PrintDesc_Ghosts       (desc_flag); return 1;
    case OBJ_BAT           : PrintDesc_Bat          (desc_flag); return 1;
    case OBJ_TROLL         : PrintDesc_Troll        (desc_flag); return 1;
    case OBJ_THIEF         : PrintDesc_Thief        (desc_flag); return 1;
    case OBJ_CYCLOPS       : PrintDesc_Cyclops      (desc_flag); return 1;
    case OBJ_INFLATED_BOAT : PrintDesc_InflatedBoat (desc_flag); return 1;
    case OBJ_LAMP          : PrintDesc_Lamp         (desc_flag); return 1;
    case OBJ_CANDLES       : PrintDesc_Candles      (desc_flag); return 1;
  }

  return 0;
}
//*****************************************************************************



//*****************************************************************************
void PrintUsingMsg(int obj)
{
  PrintTextIndex(118);
  PrintObjectDesc(obj, 0);
  PrintTextIndex(119);
}



void PrintFutileMsg(int obj)
{
  PrintTextIndex(120);

  if (obj > 0 && obj < NUM_OBJECTS)
    PrintObjectDesc(obj, 0);
  else
    PrintTextIndex(121);

  PrintTextIndex(122);
}



void TieRopeToRailingRoutine(void)
{
  if (RopeTiedToRail) {PrintLineIndex(123); return;}

  RopeTiedToRail = 1;
  Obj[OBJ_ROPE].loc = ROOM_DOME_ROOM;
  Obj[OBJ_ROPE].prop |= PROP_NODESC;
  Obj[OBJ_ROPE].prop |= PROP_NOTTAKEABLE;

  PrintLineIndex(124);

  TimePassed = 1;
}



void DoMiscWithTo_tie_rope(int with_to)
{
  if (with_to == 0 && Obj[OBJ_YOU].loc == ROOM_DOME_ROOM) {with_to = FOBJ_RAILING; PrintLineIndex(125);}
  if (with_to == 0) {PrintLineIndex(126); return;}
  if (with_to != FOBJ_RAILING) {PrintLineIndex(127); return;}

  TieRopeToRailingRoutine();
}



void DoMiscWithTo_tie_railing(int with_to)
{
  if (with_to == 0 && (Obj[OBJ_ROPE].loc == INSIDE + OBJ_YOU || Obj[OBJ_ROPE].loc == ROOM_DOME_ROOM))
  {
    with_to = OBJ_ROPE;
    PrintUsingMsg(with_to);
  }
  if (with_to == 0) {PrintLineIndex(128); return;}
  if (with_to != OBJ_ROPE) {PrintLineIndex(129); return;}

  TieRopeToRailingRoutine();
}



void DoMiscWithTo_untie_rope(int with_to)
{
  if (with_to == 0 && Obj[OBJ_YOU].loc == ROOM_DOME_ROOM && RopeTiedToRail) PrintLineIndex(130);
  if (with_to != 0 && with_to != FOBJ_RAILING) {PrintLineIndex(131); return;}

  if (RopeTiedToRail == 0) {PrintLineIndex(132); return;}

  RopeTiedToRail = 0;
  Obj[OBJ_ROPE].prop &= ~PROP_NODESC;
  Obj[OBJ_ROPE].prop &= ~PROP_NOTTAKEABLE;

  PrintLineIndex(133);

  TimePassed = 1;
}



void DoMiscWithTo_turn_bolt(int with_to)
{
  int need = OBJ_WRENCH;

  if (with_to == 0 && Obj[need].loc == INSIDE + OBJ_YOU) {with_to = need; PrintUsingMsg(with_to);}
  if (with_to == 0) {PrintLineIndex(134); return;}
  if (with_to != need) {PrintFutileMsg(with_to); return;}
  if (Obj[with_to].loc != INSIDE + OBJ_YOU) {PrintLineIndex(135); return;}

  if (GatesButton)
  {
    TimePassed = 1;
    Room[ROOM_RESERVOIR_SOUTH].prop &= ~R_DESCRIBED;

    if (GatesOpen)
    {
      GatesOpen = 0;
      Room[ROOM_LOUD_ROOM].prop &= ~R_DESCRIBED;
      ReservoirFillCountdown = 8;
      ReservoirDrainCountdown = 0;
      PrintLineIndex(136);
    }
    else
    {
      GatesOpen = 1;
      ReservoirFillCountdown = 0;
      ReservoirDrainCountdown = 8;
      PrintLineIndex(137);
    }
  }
  else
    PrintLineIndex(134);
}



void DoMiscWithTo_fix_leak(int with_to)
{
  int need = OBJ_PUTTY;

  if (MaintenanceWaterLevel <= 0) {PrintLineIndex(138); return;}

  if (with_to == 0 && Obj[need].loc == INSIDE + OBJ_YOU) {with_to = need; PrintUsingMsg(with_to);}
  if (with_to == 0) {PrintLineIndex(139); return;}
  if (with_to != need) {PrintFutileMsg(with_to); return;}
  if (Obj[with_to].loc != INSIDE + OBJ_YOU) {PrintLineIndex(135); return;}

  TimePassed = 1;
  MaintenanceWaterLevel = -1;
  PrintLineIndex(140);
}



void DoMiscWithTo_inflate_fill_inflatable_boat(int with_to)
{
  int need = OBJ_PUMP;

  if (with_to == 0 && Obj[need].loc == INSIDE + OBJ_YOU) {with_to = need; PrintUsingMsg(with_to);}
  if (with_to == 0) {PrintLineIndex(141); return;}
  if (with_to != need) {PrintFutileMsg(with_to); return;}
  if (Obj[with_to].loc != INSIDE + OBJ_YOU) {PrintLineIndex(142); return;}

  if (Obj[OBJ_INFLATABLE_BOAT].loc != Obj[OBJ_YOU].loc) {PrintLineIndex(143); return;}

  TimePassed = 1;

  PrintLineIndex(144);
  ItObj = OBJ_INFLATED_BOAT;

  if ((Obj[OBJ_BOAT_LABEL].prop & PROP_MOVEDDESC) == 0)
    PrintLineIndex(145);

  Obj[OBJ_INFLATED_BOAT].loc = Obj[OBJ_INFLATABLE_BOAT].loc;
  Obj[OBJ_INFLATABLE_BOAT].loc = 0;
}



void DoMiscWithTo_inflate_fill_inflated_boat(int with_to __attribute__((unused)))
{
  PrintLineIndex(146);
}



void DoMiscWithTo_inflate_fill_punctured_boat(int with_to __attribute__((unused)))
{
  PrintLineIndex(147);
}



void DoMiscWithTo_deflate_inflated_boat(int with_to __attribute__((unused)))
{
  if (YouAreInBoat) {PrintLineIndex(148); return;}

  if (Obj[OBJ_INFLATED_BOAT].loc != Obj[OBJ_YOU].loc) {PrintLineIndex(149); return;}

  TimePassed = 1;

  PrintLineIndex(150);
  ItObj = OBJ_INFLATABLE_BOAT;

  Obj[OBJ_INFLATABLE_BOAT].loc = Obj[OBJ_INFLATED_BOAT].loc;
  Obj[OBJ_INFLATED_BOAT].loc = 0;
}



void DoMiscWithTo_deflate_inflatable_boat(int with_to __attribute__((unused)))
{
  PrintLineIndex(151);
}



void DoMiscWithTo_deflate_punctured_boat(int with_to __attribute__((unused)))
{
  PrintLineIndex(151);
}



void DoMiscWithTo_fix_punctured_boat(int with_to)
{
  int need = OBJ_PUTTY;

  if (with_to == 0 && Obj[need].loc == INSIDE + OBJ_YOU) {with_to = need; PrintUsingMsg(with_to);}
  if (with_to == 0) {PrintLineIndex(139); return;}
  if (with_to != need) {PrintFutileMsg(with_to); return;}
  if (Obj[with_to].loc != INSIDE + OBJ_YOU) {PrintLineIndex(135); return;}

  TimePassed = 1;

  Obj[OBJ_INFLATABLE_BOAT].loc = Obj[OBJ_PUNCTURED_BOAT].loc;
  Obj[OBJ_PUNCTURED_BOAT].loc = 0;

  PrintLineIndex(152);
}



void DoMisc_open_grate(void);



void LockUnlockGrating(int with_to, int lock_flag)
{
  int need = OBJ_KEYS;

  if (GratingRevealed == 0) {PrintLineIndex(138); return;}

  if (with_to == 0 && Obj[need].loc == INSIDE + OBJ_YOU) {with_to = need; PrintUsingMsg(with_to);}
  if (with_to == 0) {PrintLineIndex(153); return;}
  if (with_to != need) {PrintFutileMsg(with_to); return;}
  if (Obj[with_to].loc != INSIDE + OBJ_YOU) {PrintLineIndex(135); return;}

  if (lock_flag)
  {
    if (GratingUnlocked == 0)
      PrintLineIndex(154);
    else if (Obj[OBJ_YOU].loc == ROOM_GRATING_CLEARING)
      PrintLineIndex(155);
    else
    {
      int prev_darkness;

      PrintLineIndex(156);

      TimePassed = 1;
      GratingUnlocked = 0;
      GratingOpen = 0; // grating may already be closed here

      prev_darkness = IsPlayerInDarkness();
      Room[ROOM_GRATING_ROOM].prop &= ~R_LIT; // no light spilling from grate opening
      if (IsPlayerInDarkness() != prev_darkness)
      {
        PrintNewLine();
        PrintPlayerRoomDesc(0);
      }
    }
  }
  else //unlock
  {
    if (GratingUnlocked)
      PrintLineIndex(157);
    else if (Obj[OBJ_YOU].loc == ROOM_GRATING_CLEARING)
      PrintLineIndex(158);
    else
    {
      TimePassed = 1;
      GratingUnlocked = 1;
      // grating is closed here

      DoMisc_open_grate();
    }
  }
}



void DoMiscWithTo_lock_grate(int with_to)
{
  LockUnlockGrating(with_to, 1); //1: lock
}



void DoMiscWithTo_unlock_grate(int with_to)
{
  LockUnlockGrating(with_to, 0); //0: unlock
}



void ActivateObj(int obj)
{
  int prev_darkness;

  if (Obj[obj].prop & PROP_LIT)
  {
    PrintLineIndex(159);
    return;
  }

  TimePassed = 1;
  PrintLineIndex(160);

  prev_darkness = IsPlayerInDarkness();
  Obj[obj].prop |= PROP_LIT;
  if (IsPlayerInDarkness() != prev_darkness)
  {
    PrintNewLine();
    PrintPlayerRoomDesc(1);
  }
}



void DeactivateObj(int obj)
{
  int prev_darkness;

  if ((Obj[obj].prop & PROP_LIT) == 0)
  {
    PrintLineIndex(161);
    return;
  }

  TimePassed = 1;
  PrintLineIndex(162);

  prev_darkness = IsPlayerInDarkness();
  Obj[obj].prop &= ~PROP_LIT;
  if (IsPlayerInDarkness() != prev_darkness)
  {
    PrintNewLine();
    PrintPlayerRoomDesc(1);
  }
}



void DoMiscWithTo_activate_lamp(int with_to)
{
  if (with_to != 0) PrintLineIndex(163);
  else if (LampTurnsLeft == 0) PrintLineIndex(164);
  else ActivateObj(OBJ_LAMP);
}



void DoMiscWithTo_deactivate_lamp(int with_to)
{
  if (with_to != 0) PrintLineIndex(163);
  else if (LampTurnsLeft == 0) PrintLineIndex(165);
  else DeactivateObj(OBJ_LAMP);
}



void DoMiscWithTo_activate_match(int with_to)
{
  int prev_darkness;

  if (with_to != 0)
  {
    PrintLineIndex(163);
    return;
  }

  if (Obj[OBJ_MATCH].loc != INSIDE + OBJ_YOU)
  {
    PrintLineIndex(135);
    return;
  }

  if (Obj[OBJ_MATCH].prop & PROP_LIT)
  {
    PrintLineIndex(166);
    return;
  }

  if (MatchesLeft <= 1)
  {
    PrintLineIndex(167);
    if (MatchesLeft == 0) return;
  }
  MatchesLeft--;

  TimePassed = 1;

  if (Obj[OBJ_YOU].loc == ROOM_LOWER_SHAFT ||
      Obj[OBJ_YOU].loc == ROOM_TIMBER_ROOM)
  {
    PrintLineIndex(168);
    return;
  }

  MatchTurnsLeft = 2;
  PrintLineIndex(169);

  prev_darkness = IsPlayerInDarkness();
  Obj[OBJ_MATCH].prop |= PROP_LIT;
  if (IsPlayerInDarkness() != prev_darkness)
  {
    PrintNewLine();
    PrintPlayerRoomDesc(1);
  }
}



void DoMiscWithTo_deactivate_match(int with_to)
{
  int prev_darkness;

  if (with_to != 0)
  {
    PrintLineIndex(163);
    return;
  }

  if ((Obj[OBJ_MATCH].prop & PROP_LIT) == 0)
  {
    PrintLineIndex(170);
    return;
  }

  TimePassed = 1;
  MatchTurnsLeft = 0;
  PrintLineIndex(171);

  prev_darkness = IsPlayerInDarkness();
  Obj[OBJ_MATCH].prop &= ~PROP_LIT;
  if (IsPlayerInDarkness() != prev_darkness)
  {
    PrintNewLine();
    PrintPlayerRoomDesc(1);
  }
}



void DoMiscWithTo_activate_candles(int with_to)
{
  if (CandleTurnsLeft == 0)
  {
    PrintLineIndex(172);
    return;
  }

  if (Obj[OBJ_CANDLES].loc != INSIDE + OBJ_YOU)
  {
    PrintLineIndex(173);
    return;
  }

  if (with_to == 0 &&
      Obj[OBJ_MATCH].loc == INSIDE + OBJ_YOU &&
      (Obj[OBJ_MATCH].prop & PROP_LIT))
  {
    with_to = OBJ_MATCH;
    PrintLineIndex(174);
  }

  if (with_to == 0)
  {
    PrintLineIndex(175);
    return;
  }

  if (with_to == OBJ_MATCH && (Obj[OBJ_MATCH].prop & PROP_LIT))
  {
    if (Obj[OBJ_MATCH].loc != INSIDE + OBJ_YOU)
      PrintLineIndex(176);
    else if (Obj[OBJ_CANDLES].prop & PROP_LIT)
      PrintLineIndex(177);
    else
    {
      int prev_darkness;

      TimePassed = 1;
      PrintLineIndex(178);

      prev_darkness = IsPlayerInDarkness();
      Obj[OBJ_CANDLES].prop |= PROP_LIT;
      if (IsPlayerInDarkness() != prev_darkness)
      {
        PrintNewLine();
        PrintPlayerRoomDesc(1);
      }
    }
  }
  else if (with_to == OBJ_TORCH && (Obj[OBJ_TORCH].prop & PROP_LIT))
  {
    if (Obj[OBJ_TORCH].loc != INSIDE + OBJ_YOU)
      PrintLineIndex(179);
    else if (Obj[OBJ_CANDLES].prop & PROP_LIT)
      PrintLineIndex(180);
    else
    {
      TimePassed = 1;
      Obj[OBJ_CANDLES].loc = 0;

      PrintLineIndex(181);
    }
  }
  else
    PrintLineIndex(182);
}



void DoMiscWithTo_deactivate_candles(int with_to)
{
  int prev_darkness;

  if (with_to != 0)
  {
    PrintLineIndex(163);
    return;
  }

  if ((Obj[OBJ_CANDLES].prop & PROP_LIT) == 0)
  {
    PrintLineIndex(183);
    return;
  }

  TimePassed = 1;
  PrintLineIndex(184);
  Obj[OBJ_CANDLES].prop |= PROP_MOVEDDESC; // needed since unmoved description of candles says they are burning

  prev_darkness = IsPlayerInDarkness();
  Obj[OBJ_CANDLES].prop &= ~PROP_LIT;
  if (IsPlayerInDarkness() != prev_darkness)
  {
    PrintNewLine();
    PrintPlayerRoomDesc(1);
  }
}



void DoMiscWithTo_activate_machine(int with_to)
{
  int found, obj;

  if (with_to == 0)
    {PrintLineIndex(185); return;}

  if (with_to != OBJ_SCREWDRIVER)
    {PrintLineIndex(186); return;}

  if (Obj[OBJ_SCREWDRIVER].loc != INSIDE + OBJ_YOU)
    {PrintLineIndex(187); return;}

  if (Obj[OBJ_MACHINE].prop & PROP_OPEN)
    {PrintLineIndex(188); return;}

  TimePassed = 1;
  PrintLineIndex(189);

  found = 0;
  for (obj=2; obj<NUM_OBJECTS; obj++)
    if (Obj[obj].loc == INSIDE + OBJ_MACHINE)
  {
    if (found == 0) found = 1;
    if (obj == OBJ_COAL) found = 2;
    Obj[obj].loc = 0;
  }

  if (found == 2)
    Obj[OBJ_DIAMOND].loc = INSIDE + OBJ_MACHINE;
  else if (found == 1)
    Obj[OBJ_GUNK].loc = INSIDE + OBJ_MACHINE;
}



void DoMiscWithTo_dig_sand(int with_to)
{
  int need = OBJ_SHOVEL;

  if (with_to == 0 && Obj[need].loc == INSIDE + OBJ_YOU) {with_to = need; PrintUsingMsg(with_to);}
  if (with_to == 0) {PrintLineIndex(190); return;}
  if (with_to != need) {PrintFutileMsg(with_to); return;}
  if (Obj[with_to].loc != INSIDE + OBJ_YOU) {PrintLineIndex(135); return;}

  TimePassed = 1;
  CaveHoleDepth++;
  switch (CaveHoleDepth)
  {
    case 1: PrintLineIndex(191);                break;
    case 2: PrintLineIndex(192);   break;
    case 3: PrintLineIndex(193); break;

    case 4:
      if (Obj[OBJ_SCARAB].prop & PROP_INVISIBLE)
      {
        Obj[OBJ_SCARAB].prop &= ~PROP_INVISIBLE;

        PrintLineIndex(194);
        ItObj = OBJ_SCARAB;
      }
      else
        PrintLineIndex(195);
    break;

    default:
      CaveHoleDepth = 0;
      if (Obj[OBJ_SCARAB].loc == ROOM_SANDY_CAVE)
        Obj[OBJ_SCARAB].prop |= PROP_INVISIBLE;
      PrintLineIndex(196);
      YoureDead(); // ##### RIP #####
    break;
  }
}



void DoMiscWithTo_fill_bottle(int with_to)
{
  if (with_to == 0 && (Room[Obj[OBJ_YOU].loc].prop & R_WATERHERE))
    {with_to = OBJ_WATER; PrintLineIndex(197);}

  if (with_to == 0) {PrintLineIndex(198); return;}
  if (with_to != OBJ_WATER) {PrintLineIndex(199); return;}
  if ((Room[Obj[OBJ_YOU].loc].prop & R_WATERHERE) == 0) {PrintLineIndex(200); return;}
  if ((Obj[OBJ_BOTTLE].prop & PROP_OPEN) == 0)
  {
    PrintLineIndex(201);
    ItObj = OBJ_BOTTLE;
    return;
  }
  if (Obj[OBJ_WATER].loc == INSIDE + OBJ_BOTTLE) {PrintLineIndex(202); return;}

  TimePassed = 1;
  Obj[OBJ_WATER].loc = INSIDE + OBJ_BOTTLE;
  PrintLineIndex(203);
}



void AttackVillain(int obj, int with_to)
{
  if (with_to >= NUM_OBJECTS)
    {PrintLineIndex(204); return;}

  if (with_to == 0)
  {
    int i;

    for (i=2; i<NUM_OBJECTS; i++)
    {
      with_to = Obj[i].order;
      if (Obj[with_to].loc == INSIDE + OBJ_YOU &&
          (Obj[with_to].prop & PROP_WEAPON)) break;
    }

    if (i == NUM_OBJECTS) with_to = 0;
    else PrintUsingMsg(with_to);
  }

  if (obj == OBJ_BAT)
  {
    PrintLineIndex(205);
    return;
  }
  else if (obj == OBJ_GHOSTS)
  {
    if (with_to == 0) PrintLineIndex(206);
    else              PrintLineIndex(207);
    return;
  }
  else if (obj == OBJ_THIEF && (Obj[OBJ_THIEF].prop & PROP_INVISIBLE))
  {
    PrintLineIndex(208);
    return;
  }

  if (with_to == 0 || with_to == OBJ_YOU)
  {
    PrintTextIndex(209); if (obj == OBJ_YOU) PrintTextIndex(210); else PrintTextIndex(211);
    PrintLineIndex(212);
    return;
  }

  if ((Obj[with_to].prop & PROP_WEAPON) == 0)
  {
    PrintTextIndex(209); if (obj == OBJ_YOU) PrintTextIndex(210); else PrintTextIndex(211);
    PrintLineIndex(213);
    return;
  }

  TimePassed = 1;

  if (with_to == OBJ_RUSTY_KNIFE)
  {
    Obj[OBJ_RUSTY_KNIFE].loc = 0;
    PrintLineIndex(214);
    YoureDead(); // ##### RIP #####
    return;
  }

  if (obj == OBJ_CYCLOPS && CyclopsState == 3) // asleep
  {
    CyclopsState = 0;
    VillainAttacking[VILLAIN_CYCLOPS] = 1;
    PrintLineIndex(215);
    return;
  }

  PlayerBlow(obj, with_to);
}



void DoMiscWithTo_attack_bat     (int with_to) {AttackVillain(OBJ_BAT    , with_to);}
void DoMiscWithTo_attack_ghosts  (int with_to) {AttackVillain(OBJ_GHOSTS , with_to);}
void DoMiscWithTo_attack_cyclops (int with_to) {AttackVillain(OBJ_CYCLOPS, with_to);}
void DoMiscWithTo_attack_thief   (int with_to) {AttackVillain(OBJ_THIEF  , with_to);}
void DoMiscWithTo_attack_troll   (int with_to) {AttackVillain(OBJ_TROLL  , with_to);}
void DoMiscWithTo_attack_yourself(int with_to) {AttackVillain(OBJ_YOU    , with_to);}



int CheckFlameSource(int obj, const char *msg)
{
  if (Obj[obj].loc == INSIDE + OBJ_YOU &&
      (Obj[obj].prop & PROP_LIT))
  {
    PrintLine(msg);
    return obj;
  }
  return 0;
}



void BurnObj(int obj, int with)
{
  if (with == 0) with = CheckFlameSource(OBJ_MATCH  , "(with the match)");
  if (with == 0) with = CheckFlameSource(OBJ_CANDLES, "(with the candles)");
  if (with == 0) with = CheckFlameSource(OBJ_TORCH  , "(with the torch)");

  if (with == 0)
    {PrintLineIndex(216); return;}

  if (Obj[with].loc != INSIDE + OBJ_YOU)
  {
    switch (with)
    {
      case OBJ_MATCH:   PrintLineIndex(176);   break;
      case OBJ_CANDLES: PrintLineIndex(173); break;
      case OBJ_TORCH:   PrintLineIndex(179);   break;
      default:          PrintLineIndex(217);   break;
    }
    return;
  }

  if ((Obj[with].prop & PROP_LIT) == 0)
    {PrintLineIndex(218); return;}

  if (obj == FOBJ_WHITE_HOUSE)
    {PrintLineIndex(219); return;}
  else if (obj == FOBJ_FRONT_DOOR)
    {PrintLineIndex(220); return;}
  else if (obj >= NUM_OBJECTS)
    {PrintLineIndex(221); return;}

  TimePassed = 1;

  if (obj == OBJ_INFLATED_BOAT && YouAreInBoat)
  {
    PrintLineIndex(222);
    YouAreInBoat = 0;
    Obj[obj].loc = 0;
    YoureDead(); // ##### RIP #####
    return;
  }

  if (Obj[obj].loc == INSIDE + OBJ_YOU)
  {
    if (obj == OBJ_LEAVES)
      PrintLineIndex(223);
    else
      PrintLineIndex(224);
    Obj[obj].loc = 0;
    YoureDead(); // ##### RIP #####
    return;
  }

  Obj[obj].loc = 0;

  if (obj == OBJ_LEAVES)
  {
    PrintLineIndex(225);
    if (GratingRevealed == 0)
    {
      GratingRevealed = 1;
      PrintLineIndex(226);
    }
  }
  else if (obj == OBJ_BOOK)
  {
    PrintLineIndex(227);
    YoureDead(); // ##### RIP #####
  }
  else
    PrintLineIndex(228);
}



void DoMiscWithTo_activate_leaves         (int with_to) {BurnObj(OBJ_LEAVES         , with_to);}
void DoMiscWithTo_activate_book           (int with_to) {BurnObj(OBJ_BOOK           , with_to);}
void DoMiscWithTo_activate_sandwich_bag   (int with_to) {BurnObj(OBJ_SANDWICH_BAG   , with_to);}
void DoMiscWithTo_activate_advertisement  (int with_to) {BurnObj(OBJ_ADVERTISEMENT  , with_to);}
void DoMiscWithTo_activate_inflated_boat  (int with_to) {BurnObj(OBJ_INFLATED_BOAT  , with_to);}
void DoMiscWithTo_activate_painting       (int with_to) {BurnObj(OBJ_PAINTING       , with_to);}
void DoMiscWithTo_activate_punctured_boat (int with_to) {BurnObj(OBJ_PUNCTURED_BOAT , with_to);}
void DoMiscWithTo_activate_inflatable_boat(int with_to) {BurnObj(OBJ_INFLATABLE_BOAT, with_to);}
void DoMiscWithTo_activate_coal           (int with_to) {BurnObj(OBJ_COAL           , with_to);}
void DoMiscWithTo_activate_boat_label     (int with_to) {BurnObj(OBJ_BOAT_LABEL     , with_to);}
void DoMiscWithTo_activate_guide          (int with_to) {BurnObj(OBJ_GUIDE          , with_to);}
void DoMiscWithTo_activate_nest           (int with_to) {BurnObj(OBJ_NEST           , with_to);}
void DoMiscWithTo_activate_white_house    (int with_to) {BurnObj(FOBJ_WHITE_HOUSE   , with_to);}
void DoMiscWithTo_activate_front_door     (int with_to) {BurnObj(FOBJ_FRONT_DOOR    , with_to);}



void DoMiscWithTo_activate_torch(int with_to __attribute__((unused)))
{
  PrintLineIndex(229);
}



void DoMiscWithTo_deactivate_torch(int with_to __attribute__((unused)))
{
  PrintLineIndex(230);
}



void DoMiscWithTo_turn_book(int with_to __attribute__((unused)))
{
  PrintLineIndex(231);
}



void DoMiscWithTo_pour_water(int with_to)
{
  if (Obj[OBJ_BOTTLE].loc != INSIDE + OBJ_YOU ||
      Obj[OBJ_WATER].loc != INSIDE + OBJ_BOTTLE)
    PrintLineIndex(232);
  else if ((Obj[OBJ_BOTTLE].prop & PROP_OPEN) == 0)
    PrintLineIndex(233);
  else if (with_to == 0)
    PrintLineIndex(234);
  else
  {
    TimePassed = 1;
    Obj[OBJ_WATER].loc = 0;

    switch (with_to)
    {
      case OBJ_HOT_BELL:
        PrintLineIndex(235);
        BellHotCountdown = 0;
        Obj[OBJ_BELL].loc = ROOM_ENTRANCE_TO_HADES;
        Obj[OBJ_HOT_BELL].loc = 0;
      break;

      case OBJ_TORCH:
        PrintLineIndex(236);
      break;

      case OBJ_MATCH:
      case OBJ_CANDLES:
        if (Obj[with_to].prop & PROP_LIT)
        {
          int prev_darkness = IsPlayerInDarkness();

          PrintLineIndex(237);
          Obj[with_to].prop &= ~PROP_LIT;
          if (with_to == OBJ_MATCH) MatchTurnsLeft = 0;

          if (IsPlayerInDarkness() != prev_darkness)
          {
            PrintNewLine();
            PrintPlayerRoomDesc(1);
          }
        }
        else
          PrintLineIndex(238);
      break;

      default: // note that this includes with_to >= NUM_OBJECTS
        PrintLineIndex(238);
      break;
    }
  }
}



void DoMiscWithTo_pour_putty(int with_to)
{
  if (Obj[OBJ_PUTTY].loc != INSIDE + OBJ_YOU &&
      ( Obj[OBJ_TUBE].loc != INSIDE + OBJ_YOU ||
        Obj[OBJ_PUTTY].loc != INSIDE + OBJ_TUBE))
    PrintLineIndex(135);
  else if ((Obj[OBJ_TUBE].prop & PROP_OPEN) == 0)
    PrintLineIndex(239);
  else if (with_to == 0)
    PrintLineIndex(234);
  else
    switch (with_to)
  {
    case FOBJ_LEAK:
      if (MaintenanceWaterLevel <= 0)
        PrintLineIndex(138);
      else
      {
        TimePassed = 1;
        MaintenanceWaterLevel = -1;
        PrintLineIndex(140);
      }
    break;

    case OBJ_PUNCTURED_BOAT:
      TimePassed = 1;
      Obj[OBJ_INFLATABLE_BOAT].loc = Obj[OBJ_PUNCTURED_BOAT].loc;
      Obj[OBJ_PUNCTURED_BOAT].loc = 0;
      PrintLineIndex(152);
    break;

    default: // note that this includes with_to >= NUM_OBJECTS
      PrintLineIndex(240);
    break;
  }
}



void DoMiscWithTo_oil_bolt(int with_to)
{
  if (with_to == 0)
    PrintLineIndex(241);
  else if (with_to != OBJ_PUTTY)
    PrintLineIndex(242);
  else if (Obj[with_to].loc != INSIDE + OBJ_YOU)
    PrintLineIndex(135);
  else
  {
    TimePassed = 1;
    PrintLineIndex(243);
  }
}



void DoMiscWithTo_brush_teeth(int with_to)
{
  if (with_to == 0)
    PrintLineIndex(244);
  else if (with_to != OBJ_PUTTY)
    PrintLineIndex(245);
  else if (Obj[with_to].loc != INSIDE + OBJ_YOU)
    PrintLineIndex(135);
  else
  {
    TimePassed = 1;
    PrintLineIndex(246);
    YoureDead(); // ##### RIP #####
  }
}



void TieUpRoutine(int i, int with_to)
{
  if (with_to == 0 && Obj[OBJ_ROPE].loc == INSIDE + OBJ_YOU)
  {
    with_to = OBJ_ROPE;
    PrintUsingMsg(with_to);
  }
  if (with_to == 0) {PrintLineIndex(247); return;}
  if (with_to != OBJ_ROPE) {PrintLineIndex(248); return;}


  if (i == VILLAIN_CYCLOPS)
    PrintLineIndex(249);
  else
  {
    const char *name;

    if (i == VILLAIN_THIEF) name = "thief"; else name = "troll";

    if (VillainStrength[i] < 0)
    {
      PrintTextIndex(250); PrintText(name); PrintLineIndex(251);
      VillainStrength[i] = -VillainStrength[i];
      VillainConscious(i);
    }
    else
      {PrintTextIndex(252); PrintText(name); PrintLineIndex(253);}
  }
}



void DoMiscWithTo_tie_cyclops(int with_to) {TieUpRoutine(VILLAIN_CYCLOPS, with_to);}
void DoMiscWithTo_tie_thief  (int with_to) {TieUpRoutine(VILLAIN_THIEF  , with_to);}
void DoMiscWithTo_tie_troll  (int with_to) {TieUpRoutine(VILLAIN_TROLL  , with_to);}



void CallDoMiscWithTo(int i, int with_to)
{
  switch (i)
  {
    case  0: DoMiscWithTo_tie_rope                     (with_to); return;
    case  1: DoMiscWithTo_tie_railing                  (with_to); return;
    case  2: DoMiscWithTo_untie_rope                   (with_to); return;
    case  3: DoMiscWithTo_turn_bolt                    (with_to); return;
    case  4: DoMiscWithTo_fix_leak                     (with_to); return;
    case  5: DoMiscWithTo_inflate_fill_inflatable_boat (with_to); return;
    case  6: DoMiscWithTo_inflate_fill_inflated_boat   (with_to); return;
    case  7: DoMiscWithTo_inflate_fill_punctured_boat  (with_to); return;
    case  8: DoMiscWithTo_inflate_fill_inflatable_boat (with_to); return;
    case  9: DoMiscWithTo_inflate_fill_inflated_boat   (with_to); return;
    case 10: DoMiscWithTo_inflate_fill_punctured_boat  (with_to); return;
    case 11: DoMiscWithTo_deflate_inflated_boat        (with_to); return;
    case 12: DoMiscWithTo_deflate_inflatable_boat      (with_to); return;
    case 13: DoMiscWithTo_deflate_punctured_boat       (with_to); return;
    case 14: DoMiscWithTo_fix_punctured_boat           (with_to); return;
    case 15: DoMiscWithTo_lock_grate                   (with_to); return;
    case 16: DoMiscWithTo_unlock_grate                 (with_to); return;
    case 17: DoMiscWithTo_activate_lamp                (with_to); return;
    case 18: DoMiscWithTo_deactivate_lamp              (with_to); return;
    case 19: DoMiscWithTo_activate_match               (with_to); return;
    case 20: DoMiscWithTo_deactivate_match             (with_to); return;
    case 21: DoMiscWithTo_activate_candles             (with_to); return;
    case 22: DoMiscWithTo_deactivate_candles           (with_to); return;
    case 23: DoMiscWithTo_activate_machine             (with_to); return;
    case 24: DoMiscWithTo_activate_machine             (with_to); return;
    case 25: DoMiscWithTo_activate_machine             (with_to); return;
    case 26: DoMiscWithTo_dig_sand                     (with_to); return;
    case 27: DoMiscWithTo_fill_bottle                  (with_to); return;
    case 28: DoMiscWithTo_attack_bat                   (with_to); return;
    case 29: DoMiscWithTo_attack_ghosts                (with_to); return;
    case 30: DoMiscWithTo_attack_cyclops               (with_to); return;
    case 31: DoMiscWithTo_attack_thief                 (with_to); return;
    case 32: DoMiscWithTo_attack_troll                 (with_to); return;
    case 33: DoMiscWithTo_attack_yourself              (with_to); return;
    case 34: DoMiscWithTo_activate_leaves              (with_to); return;
    case 35: DoMiscWithTo_activate_book                (with_to); return;
    case 36: DoMiscWithTo_activate_sandwich_bag        (with_to); return;
    case 37: DoMiscWithTo_activate_advertisement       (with_to); return;
    case 38: DoMiscWithTo_activate_inflated_boat       (with_to); return;
    case 39: DoMiscWithTo_activate_painting            (with_to); return;
    case 40: DoMiscWithTo_activate_punctured_boat      (with_to); return;
    case 41: DoMiscWithTo_activate_inflatable_boat     (with_to); return;
    case 42: DoMiscWithTo_activate_coal                (with_to); return;
    case 43: DoMiscWithTo_activate_boat_label          (with_to); return;
    case 44: DoMiscWithTo_activate_guide               (with_to); return;
    case 45: DoMiscWithTo_activate_nest                (with_to); return;
    case 46: DoMiscWithTo_activate_white_house         (with_to); return;
    case 47: DoMiscWithTo_activate_front_door          (with_to); return;
    case 48: DoMiscWithTo_activate_torch               (with_to); return;
    case 49: DoMiscWithTo_deactivate_torch             (with_to); return;
    case 50: DoMiscWithTo_turn_book                    (with_to); return;
    case 51: DoMiscWithTo_pour_water                   (with_to); return;
    case 52: DoMiscWithTo_pour_putty                   (with_to); return;
    case 53: DoMiscWithTo_oil_bolt                     (with_to); return;
    case 54: DoMiscWithTo_brush_teeth                  (with_to); return;
    case 55: DoMiscWithTo_tie_cyclops                  (with_to); return;
    case 56: DoMiscWithTo_tie_thief                    (with_to); return;
    case 57: DoMiscWithTo_tie_troll                    (with_to); return;
  }
}
//*****************************************************************************



//*****************************************************************************
void GiveLunchToCyclops(void)
{
  TimePassed = 1;

  CyclopsCounter = 0;
  CyclopsState = 2; // thirsty

  Obj[OBJ_LUNCH].loc = 0;

  PrintLineIndex(254);
}



void GiveBottleToCyclops(void)
{
  TimePassed = 1;

  if (Obj[OBJ_WATER].loc != INSIDE + OBJ_BOTTLE)
    PrintLineIndex(255);
  else if (CyclopsState != 2) // not thirsty
    PrintLineIndex(256);
  else
  {
    CyclopsState = 3; // asleep

    Obj[OBJ_WATER].loc = 0;
    Obj[OBJ_BOTTLE].loc = ROOM_CYCLOPS_ROOM;
    Obj[OBJ_BOTTLE].prop |= PROP_OPEN;

    PrintLineIndex(257);
  }
}



void DoMiscGiveTo_cyclops(int obj)
{
  if (obj == OBJ_WATER)
    obj = OBJ_BOTTLE;

  if (Obj[obj].loc != INSIDE + OBJ_YOU)
    PrintLineIndex(204);
  else if (CyclopsState == 3)
    PrintLineIndex(258);
  else
    switch (obj)
  {
    case OBJ_LUNCH:  GiveLunchToCyclops();                                          break;
    case OBJ_BOTTLE: GiveBottleToCyclops();                                         break;
    case OBJ_GARLIC: PrintLineIndex(259); break;
    default:         PrintLineIndex(260);     break;
  }
}



void DoMiscGiveTo_thief(int obj)
{
  if (Obj[obj].loc != INSIDE + OBJ_YOU)
    {PrintLineIndex(204); return;}

  if (Obj[OBJ_THIEF].prop & PROP_INVISIBLE)
    {PrintLineIndex(261); return;}

  TimePassed = 1;

  if (VillainStrength[VILLAIN_THIEF] < 0)
  {
    VillainStrength[VILLAIN_THIEF] = -VillainStrength[VILLAIN_THIEF];
    VillainAttacking[VILLAIN_THIEF] = 1;
    ThiefRecoverStiletto();
    ThiefDescType = 0; // default
    PrintLineIndex(262);
  }

  Obj[obj].loc = INSIDE + OBJ_THIEF;

  if (obj == OBJ_STILETTO)
  {
    Obj[OBJ_STILETTO].prop |= PROP_NODESC;
    Obj[OBJ_STILETTO].prop |= PROP_NOTTAKEABLE;

    PrintLineIndex(263);
  }
  else if (Obj[obj].thiefvalue > 0)
  {
    Obj[obj].prop |= PROP_INVISIBLE;

    ThiefEngrossed = 1;
    PrintLineIndex(264);
  }
  else
  {
    Obj[obj].prop |= PROP_INVISIBLE;

    PrintLineIndex(265);
  }
}



void DoMiscGiveTo_troll(int obj)
{
  if (Obj[obj].loc != INSIDE + OBJ_YOU)
    {PrintLineIndex(204); return;}

  if (TrollDescType == 1) // unconscious
    {PrintLineIndex(266); return;}

  TimePassed = 1;

  if (obj == OBJ_AXE)
  {
    PrintLineIndex(267);

    Obj[OBJ_AXE].loc = INSIDE + OBJ_TROLL;

    Obj[OBJ_AXE].prop |= PROP_NODESC;
    Obj[OBJ_AXE].prop |= PROP_NOTTAKEABLE;
    Obj[OBJ_AXE].prop &= ~PROP_WEAPON;

    VillainAttacking[VILLAIN_TROLL] = 1;
  }
  else
  {
    PrintTextIndex(268);
    if (obj == OBJ_KNIFE || obj == OBJ_SWORD)
    {
      if (PercentChance(20, -1))
      {
        PrintLineIndex(269);
        Obj[obj].loc = 0;
        Obj[OBJ_TROLL].loc = 0;
        VillainDead(VILLAIN_TROLL);
      }
      else
      {
        PrintLineIndex(270);
        Obj[obj].loc = Obj[OBJ_YOU].loc;
        MoveObjOrderToLast(obj);
        VillainAttacking[VILLAIN_TROLL] = 1;
      }
    }
    else
    {
      int prev_darkness;

      PrintLineIndex(271);

      prev_darkness = IsPlayerInDarkness();
      Obj[obj].loc = 0;
      if (IsPlayerInDarkness() != prev_darkness)
      {
        PrintNewLine();
        PrintPlayerRoomDesc(1);
      }
    }
  }
}



int CallDoMiscGiveTo(int to, int obj)
{
  switch (to)
  {
    case OBJ_CYCLOPS : DoMiscGiveTo_cyclops (obj); return 1;
    case OBJ_THIEF   : DoMiscGiveTo_thief   (obj); return 1;
    case OBJ_TROLL   : DoMiscGiveTo_troll   (obj); return 1;
  }

  return 0;
}
//*****************************************************************************



//*****************************************************************************
void ThrowObjRoutine(int obj, int to)
{
  int prev_darkness = IsPlayerInDarkness();

  switch (obj)
  {
    case OBJ_LAMP:
      PrintLineIndex(272);
      TimePassed = 1;
      Obj[OBJ_LAMP].loc = 0;
      Obj[OBJ_BROKEN_LAMP].loc = Obj[OBJ_YOU].loc;
    break;

    case OBJ_EGG:
      PrintLineIndex(273);
      TimePassed = 1;
      Obj[OBJ_EGG].loc = 0;
      Obj[OBJ_BROKEN_EGG].loc = Obj[OBJ_YOU].loc;
      Obj[OBJ_BROKEN_EGG].prop |= PROP_OPENABLE;
      Obj[OBJ_BROKEN_EGG].prop |= PROP_OPEN;
    break;

    case OBJ_BOTTLE:
      PrintLineIndex(274);
      TimePassed = 1;
      Obj[OBJ_BOTTLE].loc = 0;
    break;

    default:
      if (to == 0) PrintLineIndex(275);
      else         PrintLineIndex(276);
      TimePassed = 1;
      Obj[obj].loc = Obj[OBJ_YOU].loc;
      MoveObjOrderToLast(obj);
    break;
  }

  if (IsPlayerInDarkness() != prev_darkness)
  {
    PrintNewLine();
    PrintPlayerRoomDesc(1);
  }
}



void DoMiscThrowTo_chasm(int obj)
{
  int prev_darkness = IsPlayerInDarkness();

  PrintLineIndex(277);
  TimePassed = 1;
  Obj[obj].loc = 0;
  if (IsPlayerInDarkness() != prev_darkness)
  {
    PrintNewLine();
    PrintPlayerRoomDesc(1);
  }
}



void DoMiscThrowTo_river(int obj)
{
  int prev_darkness = IsPlayerInDarkness();

  PrintLineIndex(278);
  TimePassed = 1;
  Obj[obj].loc = 0;
  if (IsPlayerInDarkness() != prev_darkness)
  {
    PrintNewLine();
    PrintPlayerRoomDesc(1);
  }
}



void DoMiscThrowTo_mirror(int obj)
{
  if (MirrorBroken)
    PrintLineIndex(279);
  else
  {
    PrintLineIndex(280);
    TimePassed = 1;

    MirrorBroken = 1;
    NotLucky = 1;

    ThrowObjRoutine(obj, 0);
  }
}



void DoMiscThrowTo_troll(int obj)
{
  if (TrollDescType == 1) // unconscious
    ThrowObjRoutine(obj, OBJ_TROLL);
  else
  {
    PrintLineIndex(281);
    DoMiscGiveTo_troll(obj);
  }
}



void DoMiscThrowTo_cyclops(int obj)
{
  if (CyclopsState == 3) // sleeping
    ThrowObjRoutine(obj, OBJ_CYCLOPS);
  else
  {
    PrintLineIndex(282);
    ThrowObjRoutine(obj, 0);
  }
}



void ThiefLoseBagContents(void)
{
  int flag = 0, obj;

  PrintTextIndex(283);

  for (obj=2; obj<NUM_OBJECTS; obj++)
    if (Obj[obj].loc == INSIDE + OBJ_THIEF &&
        obj != OBJ_LARGE_BAG &&
        obj != OBJ_STILETTO)
  {
    flag = 1;
    Obj[obj].loc = Obj[OBJ_YOU].loc;
    Obj[obj].prop &= ~PROP_INVISIBLE;
  }

  if (flag)
    PrintLineIndex(284);
  else
    PrintLineIndex(56);
}



void DoMiscThrowTo_thief(int obj)
{
  if (Obj[OBJ_THIEF].prop & PROP_INVISIBLE)
    {PrintLineIndex(261); return;}

  if (ThiefDescType == 1) // unconscious
    ThrowObjRoutine(obj, OBJ_THIEF);
  else
  {
    TimePassed = 1;

    if (obj == OBJ_KNIFE &&
        VillainAttacking[VILLAIN_THIEF] == 0)
    {
      Obj[OBJ_KNIFE].loc = Obj[OBJ_YOU].loc;

      if (PercentChance(10, 0))
      {
        ThiefLoseBagContents();
        Obj[OBJ_THIEF].prop |= PROP_INVISIBLE;
      }
      else
      {
        PrintLineIndex(285);
        VillainAttacking[VILLAIN_THIEF] = 1;
      }
    }
    else
      ThrowObjRoutine(obj, OBJ_THIEF);
  }
}



int CallDoMiscThrowTo(int to, int obj)
{
  switch (to)
  {
    case FOBJ_CHASM           : DoMiscThrowTo_chasm           (obj); return 1;
    case FOBJ_CLIMBABLE_CLIFF : DoMiscThrowTo_river           (obj); return 1;
    case FOBJ_RIVER           : DoMiscThrowTo_river           (obj); return 1;
    case FOBJ_MIRROR1         : DoMiscThrowTo_mirror          (obj); return 1;
    case FOBJ_MIRROR2         : DoMiscThrowTo_mirror          (obj); return 1;
    case OBJ_TROLL            : DoMiscThrowTo_troll           (obj); return 1;
    case OBJ_CYCLOPS          : DoMiscThrowTo_cyclops         (obj); return 1;
    case OBJ_THIEF            : DoMiscThrowTo_thief           (obj); return 1;
  }

  return 0;
}
//*****************************************************************************



//*****************************************************************************
void DoMisc_open_kitchen_window(void)
{
  if (KitchenWindowOpen) PrintLineIndex(286);
  else
  {
    KitchenWindowOpen = 1;
    TimePassed = 1;
    PrintLineIndex(287);
  }
}



void DoMisc_close_kitchen_window(void)
{
  if (KitchenWindowOpen == 0) PrintLineIndex(288);
  else
  {
    KitchenWindowOpen = 0;
    TimePassed = 1;
    PrintLineIndex(289);
  }
}



void DoMisc_move_push_rug(void)
{
  if (RugMoved)
    PrintLineIndex(290);
  else
  {
    RugMoved = 1;
    TimePassed = 1;

    if (TrapOpen == 0)
    {
      PrintLineIndex(291);
      ItObj = FOBJ_TRAP_DOOR;
    }
    else
      PrintLineIndex(292);
  }
}



void DoMisc_open_trap_door(void)
{
  if (TrapOpen)
    PrintLineIndex(286);
  else if (Obj[OBJ_YOU].loc == ROOM_LIVING_ROOM)
  {
    if (RugMoved == 0)
      PrintLineIndex(293);
    else
    {
      TrapOpen = 1;
      TimePassed = 1;
      PrintLineIndex(294);
    }
  }
  else // cellar
  {
    if (ExitFound == 0)
      PrintLineIndex(295);
    else
    {
      TrapOpen = 1;
      TimePassed = 1;
      PrintLineIndex(296);
    }
  }
}



void DoMisc_close_trap_door(void)
{
  if (TrapOpen == 0)
    PrintLineIndex(288);
  else if (Obj[OBJ_YOU].loc == ROOM_LIVING_ROOM)
  {
    TrapOpen = 0;
    TimePassed = 1;
    PrintLineIndex(297);
  }
  else // cellar
  {
    TrapOpen = 0;
    TimePassed = 1;

    if (ExitFound)
      PrintLineIndex(296);
    else
      PrintLineIndex(298);
  }
}



void RaiseLowerBasketRoutine(int raise)
{
  int prev_darkness = IsPlayerInDarkness();

  Obj[OBJ_RAISED_BASKET ].loc = raise ? ROOM_SHAFT_ROOM  : ROOM_LOWER_SHAFT;
  Obj[OBJ_LOWERED_BASKET].loc = raise ? ROOM_LOWER_SHAFT : ROOM_SHAFT_ROOM ;

  TimePassed = 1;

  if (raise) PrintLineIndex(299);
  else       PrintLineIndex(300);

  if (Obj[OBJ_RAISED_BASKET].loc == Obj[OBJ_YOU].loc) ItObj = OBJ_RAISED_BASKET;
  else                                                ItObj = OBJ_LOWERED_BASKET;

  //did room become darkened when basket moved
  if (IsPlayerInDarkness() != prev_darkness && prev_darkness == 0)
  {
    PrintNewLine();
    PrintPlayerRoomDesc(1);
  }
}



void DoMisc_raise_basket(void)
{
  if (Obj[OBJ_RAISED_BASKET].loc == Obj[OBJ_YOU].loc)
  {
    if (Obj[OBJ_YOU].loc == ROOM_LOWER_SHAFT)
      RaiseLowerBasketRoutine(1);
    else PrintLineIndex(301);
  }
  else
  {
    if (Obj[OBJ_YOU].loc == ROOM_SHAFT_ROOM)
      RaiseLowerBasketRoutine(1);
    else PrintLineIndex(302);
  }
}



void DoMisc_lower_basket(void)
{
  if (Obj[OBJ_RAISED_BASKET].loc == Obj[OBJ_YOU].loc)
  {
    if (Obj[OBJ_YOU].loc == ROOM_SHAFT_ROOM)
      RaiseLowerBasketRoutine(0);
    else PrintLineIndex(301);
  }
  else
  {
    if (Obj[OBJ_YOU].loc == ROOM_LOWER_SHAFT)
      RaiseLowerBasketRoutine(0);
    else PrintLineIndex(302);
  }
}



void DoMisc_push_blue_button(void)
{
  TimePassed = 1;

  if (MaintenanceWaterLevel == 0)
  {
    MaintenanceWaterLevel = 1;
    PrintLineIndex(303);
  }
  else
    PrintLineIndex(304);
}



void DoMisc_push_red_button(void)
{
  int prev_darkness = IsPlayerInDarkness();

  TimePassed = 1;

  PrintTextIndex(305);

  if (Room[ROOM_MAINTENANCE_ROOM].prop & R_LIT)
  {
    Room[ROOM_MAINTENANCE_ROOM].prop &= ~R_LIT;
    PrintLineIndex(306);
  }
  else
  {
    Room[ROOM_MAINTENANCE_ROOM].prop |= R_LIT;
    PrintLineIndex(307);
  }

  //did room become darkened
  if (IsPlayerInDarkness() != prev_darkness && prev_darkness == 0)
  {
    PrintNewLine();
    PrintPlayerRoomDesc(1);
  }
}



void DoMisc_push_brown_button(void)
{
  PrintLineIndex(308);
  Room[ROOM_DAM_ROOM].prop &= ~R_DESCRIBED;
  GatesButton = 0;
  TimePassed = 1;
}



void DoMisc_push_yellow_button(void)
{
  PrintLineIndex(308);
  Room[ROOM_DAM_ROOM].prop &= ~R_DESCRIBED;
  GatesButton = 1;
  TimePassed = 1;
}



void DoMisc_enter_inflated_boat(void)
{
  if (Obj[OBJ_INFLATED_BOAT].loc != Obj[OBJ_YOU].loc)
    PrintLineIndex(309);
  else if (YouAreInBoat)
    PrintLineIndex(310);
  else
  {
    int loc = INSIDE + OBJ_YOU;

    TimePassed = 1;

    if (Obj[OBJ_SCEPTRE].loc == loc || Obj[OBJ_KNIFE].loc == loc || Obj[OBJ_SWORD].loc == loc ||
        Obj[OBJ_RUSTY_KNIFE].loc == loc || Obj[OBJ_AXE].loc == loc || Obj[OBJ_STILETTO].loc == loc)
    {
      PrintLineIndex(311);
      ItObj = OBJ_PUNCTURED_BOAT;

      Obj[OBJ_PUNCTURED_BOAT].loc = Obj[OBJ_INFLATED_BOAT].loc;
      Obj[OBJ_INFLATED_BOAT].loc = 0;
    }
    else
    {
      YouAreInBoat = 1;
      Obj[OBJ_INFLATED_BOAT].prop |= PROP_NOTTAKEABLE;
      PrintLineIndex(296);
    }
  }
}



void DoMisc_exit_inflated_boat(void)
{
  if (YouAreInBoat == 0)
    PrintLineIndex(312);
  else if (Room[Obj[OBJ_YOU].loc].prop & R_BODYOFWATER)
    PrintLineIndex(313);
  else
  {
    YouAreInBoat = 0;
    Obj[OBJ_INFLATED_BOAT].prop &= ~PROP_NOTTAKEABLE;
    PrintLineIndex(296);
    TimePassed = 1;
  }
}



void DoMisc_move_leaves(void)
{
  if (GratingRevealed == 0)
  {
    Obj[OBJ_LEAVES].prop |= PROP_MOVEDDESC;
    GratingRevealed = 1;
    TimePassed = 1;
    PrintLineIndex(226);
  }
  else
    PrintLineIndex(314);
}



void DoMisc_open_grate(void)
{
  int leaves_fall = 0, prev_darkness;

  if (GratingRevealed == 0) {PrintLineIndex(138); return;}
  if (GratingOpen) {PrintLineIndex(315); return;}
  if (GratingUnlocked == 0) {PrintLineIndex(156); return;}

  TimePassed = 1;
  GratingOpen = 1;

  if ((Obj[OBJ_LEAVES].prop & PROP_MOVEDDESC) == 0)
  {
    leaves_fall = 1;
    Obj[OBJ_LEAVES].prop |= PROP_MOVEDDESC;
    Obj[OBJ_LEAVES].loc = ROOM_GRATING_ROOM;
  }

  if (Obj[OBJ_YOU].loc == ROOM_GRATING_CLEARING)
    PrintLineIndex(316);
  else
  {
    PrintLineIndex(317);
    if (leaves_fall)
      PrintLineIndex(318);
  }

  prev_darkness = IsPlayerInDarkness();
  Room[ROOM_GRATING_ROOM].prop |= R_LIT; // light spilling from grate opening
  if (IsPlayerInDarkness() != prev_darkness)
  {
    PrintNewLine();
    PrintPlayerRoomDesc(0);
  }
}



void DoMisc_close_grate(void)
{
  int prev_darkness;

  if (GratingRevealed == 0) {PrintLineIndex(138); return;}
  if (GratingOpen == 0) {PrintLineIndex(319); return;}

  TimePassed = 1;
  GratingOpen = 0;

  PrintLineIndex(14);

  prev_darkness = IsPlayerInDarkness();
  Room[ROOM_GRATING_ROOM].prop &= ~R_LIT; // no light spilling from grate opening
  if (IsPlayerInDarkness() != prev_darkness)
  {
    PrintNewLine();
    PrintPlayerRoomDesc(0);
  }
}



void DoMisc_ring_bell(void)
{
  TimePassed = 1;

  if (SpiritsBanished == 0 && Obj[OBJ_YOU].loc == ROOM_ENTRANCE_TO_HADES)
  {
    PrintLineIndex(320);
    ItObj = OBJ_HOT_BELL;

    Obj[OBJ_BELL].loc = 0;
    Obj[OBJ_HOT_BELL].loc = ROOM_ENTRANCE_TO_HADES;

    if (Obj[OBJ_CANDLES].loc == INSIDE + OBJ_YOU)
    {
      PrintLineIndex(321);

      Obj[OBJ_CANDLES].loc = ROOM_ENTRANCE_TO_HADES;
      Obj[OBJ_CANDLES].prop &= ~PROP_LIT;
    }

    BellRungCountdown = 6;
    BellHotCountdown = 20;
  }
  else
    PrintLineIndex(322);
}



int AreYouInForest(void)
{
  switch (Obj[OBJ_YOU].loc)
  {
    case ROOM_FOREST_1:
    case ROOM_FOREST_2:
    case ROOM_FOREST_3:
    case ROOM_PATH:
    case ROOM_UP_A_TREE:
      return 1;

    default:
      return 0;
  }
}



void DoMisc_wind_canary(void)
{
  TimePassed = 1;

  if (SongbirdSang == 0 && AreYouInForest())
  {
    SongbirdSang = 1;
    PrintLineIndex(323);

    if (Obj[OBJ_YOU].loc == ROOM_UP_A_TREE)
      Obj[OBJ_BAUBLE].loc = ROOM_PATH;
    else
      Obj[OBJ_BAUBLE].loc = Obj[OBJ_YOU].loc;
  }
  else
    PrintLineIndex(324);
}



void DoMisc_wind_broken_canary(void)
{
  TimePassed = 1;
  PrintLineIndex(325);
}



void DoMisc_wave_sceptre(void)
{
  TimePassed = 1;

  if (Obj[OBJ_YOU].loc == ROOM_ARAGAIN_FALLS ||
      Obj[OBJ_YOU].loc == ROOM_END_OF_RAINBOW)
  {
    if (RainbowSolid == 0)
    {
      RainbowSolid = 1;
      PrintLineIndex(326);

      if (Obj[OBJ_YOU].loc == ROOM_END_OF_RAINBOW &&
          (Obj[OBJ_POT_OF_GOLD].prop & PROP_INVISIBLE))
        PrintLineIndex(327);

      Obj[OBJ_POT_OF_GOLD].prop &= ~PROP_INVISIBLE;
    }
    else
    {
      RainbowSolid = 0;
      PrintLineIndex(328);
    }
  }
  else if (Obj[OBJ_YOU].loc == ROOM_ON_RAINBOW)
  {
    RainbowSolid = 0;
    PrintLineIndex(329);
    YoureDead(); // ##### RIP #####
  }
  else
    PrintLineIndex(330);
}



void DoMisc_raise_sceptre(void)
{
  if (Obj[OBJ_SCEPTRE].loc != INSIDE + OBJ_YOU)
    PrintLineIndex(135);
  else
    DoMisc_wave_sceptre();
}



void DoMisc_touch_mirror(void)
{
  int obj;

  if (MirrorBroken)
  {
    PrintTextIndex(331);
    switch (GetRandom(3))
    {
      case 0: PrintLineIndex(332);  break;
      case 1: PrintLineIndex(333); break;
      case 2: PrintLineIndex(334);         break;
    }
    return;
  }

  TimePassed = 1;
  PrintLineIndex(335);

  // note that this includes object 1: OBJ_YOU
  for (obj=1; obj<NUM_OBJECTS; obj++)
  {
         if (Obj[obj].loc == ROOM_MIRROR_ROOM_1) Obj[obj].loc = ROOM_MIRROR_ROOM_2;
    else if (Obj[obj].loc == ROOM_MIRROR_ROOM_2) Obj[obj].loc = ROOM_MIRROR_ROOM_1;
  }
}



void DoMisc_read_book(void)
{
  int obj = OBJ_BOOK;

  //if not holding it, try to take it
  if (Obj[obj].loc != INSIDE + OBJ_YOU)
    if (TakeRoutine(obj, "(taking it first)")) return;

  TimePassed = 1;

  if (Obj[OBJ_YOU].loc == ROOM_ENTRANCE_TO_HADES && CandlesLitCountdown > 0)
  {
    CandlesLitCountdown = 0;
    Obj[OBJ_GHOSTS].loc = 0;
    SpiritsBanished = 1;

    PrintLineIndex(336);
  }
  else
    PrintLineIndex(337);
}



void DoMisc_read_advertisement(void)
{
  int obj = OBJ_ADVERTISEMENT;

  //if not holding it, try to take it
  if (Obj[obj].loc != INSIDE + OBJ_YOU)
    if (TakeRoutine(obj, "(taking it first)")) return;

  TimePassed = 1;
  PrintLineIndex(338);
}



void DoMisc_read_match(void)
{
  int obj = OBJ_MATCH;

  //if not holding it, try to take it
  if (Obj[obj].loc != INSIDE + OBJ_YOU)
    if (TakeRoutine(obj, "(taking it first)")) return;

  TimePassed = 1;

  PrintLineIndex(339);
}



void DoMisc_read_map(void)
{
  int obj = OBJ_MAP;

  //if not holding it, try to take it
  if (Obj[obj].loc != INSIDE + OBJ_YOU)
    if (TakeRoutine(obj, "(taking it first)")) return;

  TimePassed = 1;
  PrintLineIndex(340);
}



void DoMisc_read_boat_label(void)
{
  int obj = OBJ_BOAT_LABEL;

  //if not holding it, try to take it
  if (Obj[obj].loc != INSIDE + OBJ_YOU)
    if (TakeRoutine(obj, "(taking it first)")) return;

  TimePassed = 1;
  PrintLineIndex(341);
}



void DoMisc_read_guide(void)
{
  int obj = OBJ_GUIDE;

  //if not holding it, try to take it
  if (Obj[obj].loc != INSIDE + OBJ_YOU)
    if (TakeRoutine(obj, "(taking it first)")) return;

  TimePassed = 1;

  PrintLineIndex(342);
  PrintLineIndex(343);
  PrintLineIndex(344);
}



void DoMisc_read_tube(void)
{
  int obj = OBJ_TUBE;

  //if not holding it, try to take it
  if (Obj[obj].loc != INSIDE + OBJ_YOU)
    if (TakeRoutine(obj, "(taking it first)")) return;

  TimePassed = 1;
  PrintLineIndex(345);
}



void DoMisc_read_owners_manual(void)
{
  int obj = OBJ_OWNERS_MANUAL;

  //if not holding it, try to take it
  if (Obj[obj].loc != INSIDE + OBJ_YOU)
    if (TakeRoutine(obj, "(taking it first)")) return;

  TimePassed = 1;
  PrintLineIndex(346);
}



void DoMisc_read_prayer(void)
{
  TimePassed = 1;
  PrintLineIndex(347);
}



void DoMisc_read_wooden_door(void)
{
  TimePassed = 1;
  PrintLineIndex(348);
}



void DoMisc_read_engravings(void)
{
  TimePassed = 1;
  PrintLineIndex(349);
}



void DoMisc_open_egg(void)
{
  int with;

  with = GetWith(); if (with < 0) return;

  if (Obj[OBJ_EGG].loc != INSIDE + OBJ_YOU)
    {PrintLineIndex(350); return;}

  if (Obj[OBJ_EGG].prop & PROP_OPEN)
    {PrintLineIndex(351); return;}

  if (with >= NUM_OBJECTS)
    {PrintLineIndex(204); return;}

  if (with == 0)
    {PrintLineIndex(352); return;}

  if (with == OBJ_YOU)
    {PrintLineIndex(353); return;}

  if ((Obj[with].prop & PROP_WEAPON) ||
      with == OBJ_PUMP || with == OBJ_SCREWDRIVER || with == OBJ_KEYS ||
      with == OBJ_SHOVEL || with == OBJ_PUTTY || with == OBJ_WRENCH)
  {
    PrintLineIndex(354);
    TimePassed = 1;

    Obj[OBJ_EGG].loc = 0;
    Obj[OBJ_BROKEN_EGG].loc = INSIDE + OBJ_YOU;
    Obj[OBJ_BROKEN_EGG].prop |= PROP_OPENABLE;
    Obj[OBJ_BROKEN_EGG].prop |= PROP_OPEN;
    return;
  }

  PrintLineIndex(355);
}



void DoMisc_climbthrough_kitchen_window(void)
{
  if (KitchenWindowOpen == 0)
  {
    PrintLineIndex(10);
    ItObj = FOBJ_KITCHEN_WINDOW;
  }
  else
  {
    if (Obj[OBJ_YOU].loc == ROOM_EAST_OF_HOUSE)
      GoToRoutine(ROOM_KITCHEN);
    else
      GoToRoutine(ROOM_EAST_OF_HOUSE);
  }
}



void DoMisc_climbthrough_trap_door(void)
{
  if (Obj[OBJ_YOU].loc == ROOM_LIVING_ROOM)
    GoFrom_LivingRoom_Down();
  else
    GoFrom_Cellar_Up();
}



void DoMisc_climbthrough_grate(void)
{
  if (Obj[OBJ_YOU].loc == ROOM_GRATING_CLEARING)
    GoFrom_GratingClearing_Down();
  else
    GoFrom_GratingRoom_Up();
}



void DoMisc_climbthrough_slide(void)
{
  if (Obj[OBJ_YOU].loc == ROOM_CELLAR)
    PrintBlockMsg(BLA);
  else
  {
    if (YouAreInBoat == 0)
      PrintLineIndex(356);
    GoToRoutine(ROOM_CELLAR);
  }
}



void DoMisc_climbthrough_chimney(void)
{
  if (Obj[OBJ_YOU].loc == ROOM_KITCHEN)
    GoFrom_Kitchen_Down();
  else
    GoFrom_Studio_Up();
}



void DoMisc_climbthrough_barrow_door(void)
{
  GoFrom_StoneBarrow_West();
}



void DoMisc_climbthrough_gate(void)
{
  if (SpiritsBanished == 0)
    PrintLineIndex(357);
  else
    GoToRoutine(ROOM_LAND_OF_LIVING_DEAD);
}



void DoMisc_climbthrough_crack(void)
{
  PrintLineIndex(358);
}



void DoMisc_enter_white_house(void)
{
  if (Obj[OBJ_YOU].loc != ROOM_EAST_OF_HOUSE)
    PrintLineIndex(359);
  else
    DoMisc_climbthrough_kitchen_window();
}



void DoMisc_slidedown_slide(void)
{
  if (Obj[OBJ_YOU].loc == ROOM_CELLAR)
    PrintLineIndex(360);
  else
  {
    if (YouAreInBoat == 0)
      PrintLineIndex(356);
    GoToRoutine(ROOM_CELLAR);
  }
}



void DoMisc_climbup_mountain_range(void)
{
  PrintLineIndex(361);
}



void DoMisc_climbup_white_cliff(void)
{
  PrintLineIndex(362);
}



void DoMisc_climbup_tree(void)
{
  if (Obj[OBJ_YOU].loc == ROOM_PATH)
    GoToRoutine(ROOM_UP_A_TREE);
  else
    PrintBlockMsg(BL9);
}



void DoMisc_climbdown_tree(void)
{
  if (Obj[OBJ_YOU].loc == ROOM_PATH)
    PrintBlockMsg(BL0);
  else
    GoToRoutine(ROOM_PATH);
}



void DoMisc_climbup_chimney(void)
{
  if (Obj[OBJ_YOU].loc == ROOM_STUDIO)
    GoFrom_Studio_Up();
  else
    PrintBlockMsg(BL0);
}



void DoMisc_climbdown_chimney(void)
{
  if (Obj[OBJ_YOU].loc == ROOM_KITCHEN)
    GoFrom_Kitchen_Down();
  else
    PrintBlockMsg(BL0);
}



void DoMisc_climbup_ladder(void)
{
  if (Obj[OBJ_YOU].loc == ROOM_LADDER_BOTTOM)
    GoToRoutine(ROOM_LADDER_TOP);
  else
    PrintBlockMsg(BL0);
}



void DoMisc_climbdown_ladder(void)
{
  if (Obj[OBJ_YOU].loc == ROOM_LADDER_TOP)
    GoToRoutine(ROOM_LADDER_BOTTOM);
  else
    PrintBlockMsg(BL0);
}



void DoMisc_climbup_slide(void)
{
  if (Obj[OBJ_YOU].loc == ROOM_CELLAR)
    PrintBlockMsg(BLA);
  else
    PrintBlockMsg(BL0);
}



void DoMisc_climbdown_slide(void)
{
  if (Obj[OBJ_YOU].loc == ROOM_SLIDE_ROOM)
  {
    if (YouAreInBoat == 0)
      PrintLineIndex(356);
    GoToRoutine(ROOM_CELLAR);
  }
  else
    PrintBlockMsg(BL0);
}



void DoMisc_climbup_climbable_cliff(void)
{
  switch (Obj[OBJ_YOU].loc)
  {
    default:                 PrintBlockMsg(BL0);             break;
    case ROOM_CLIFF_MIDDLE:  GoToRoutine(ROOM_CANYON_VIEW);  break;
    case ROOM_CANYON_BOTTOM: GoToRoutine(ROOM_CLIFF_MIDDLE); break;
  }
}



void DoMisc_climbdown_climbable_cliff(void)
{
  switch (Obj[OBJ_YOU].loc)
  {
    case ROOM_CANYON_VIEW:   GoToRoutine(ROOM_CLIFF_MIDDLE);  break;
    case ROOM_CLIFF_MIDDLE:  GoToRoutine(ROOM_CANYON_BOTTOM); break;
    default:                 PrintBlockMsg(BL0);              break;
  }
}



void DoMisc_climbup_stairs(void)
{
  switch (Obj[OBJ_YOU].loc)
  {
    case ROOM_CELLAR:          GoFrom_Cellar_Up();              break;
    case ROOM_CYCLOPS_ROOM:    GoFrom_CyclopsRoom_Up();         break;
    case ROOM_KITCHEN:         GoToRoutine(ROOM_ATTIC);         break;
    case ROOM_RESERVOIR_NORTH: GoToRoutine(ROOM_ATLANTIS_ROOM); break;
    case ROOM_ATLANTIS_ROOM:   GoToRoutine(ROOM_SMALL_CAVE);    break;
    case ROOM_LOUD_ROOM:       GoToRoutine(ROOM_DEEP_CANYON);   break;
    case ROOM_CHASM_ROOM:      GoToRoutine(ROOM_EW_PASSAGE);    break;
    case ROOM_EGYPT_ROOM:      GoToRoutine(ROOM_NORTH_TEMPLE);  break;
    case ROOM_GAS_ROOM:        GoToRoutine(ROOM_SMELLY_ROOM);   break;
    case ROOM_LADDER_TOP:      GoToRoutine(ROOM_MINE_4);        break;
    default:                   PrintBlockMsg(BL0);              break;
  }
}



void DoMisc_climbdown_stairs(void)
{
  switch (Obj[OBJ_YOU].loc)
  {
    case ROOM_LIVING_ROOM:   GoFrom_LivingRoom_Down();            break;
    case ROOM_ATTIC:         GoToRoutine(ROOM_KITCHEN);           break;
    case ROOM_TREASURE_ROOM: GoToRoutine(ROOM_CYCLOPS_ROOM);      break;
    case ROOM_SMALL_CAVE:    GoToRoutine(ROOM_ATLANTIS_ROOM);     break;
    case ROOM_TINY_CAVE:     GoToRoutine(ROOM_ENTRANCE_TO_HADES); break;
    case ROOM_EW_PASSAGE:    GoToRoutine(ROOM_CHASM_ROOM);        break;
    case ROOM_DEEP_CANYON:   GoToRoutine(ROOM_LOUD_ROOM);         break;
    case ROOM_TORCH_ROOM:    GoToRoutine(ROOM_NORTH_TEMPLE);      break;
    case ROOM_NORTH_TEMPLE:  GoToRoutine(ROOM_EGYPT_ROOM);        break;
    case ROOM_SMELLY_ROOM:   GoToRoutine(ROOM_GAS_ROOM);          break;
    default:                 PrintBlockMsg(BL0);                  break;
  }
}



void DoMisc_examine_sword(void)
{
  int glow = Obj[OBJ_SWORD].thiefvalue;

       if (glow == 1) PrintLineIndex(363);
  else if (glow == 2) PrintLineIndex(364);
  else                PrintLineIndex(365);
}



void DoMisc_examine_match(void)
{
  if (Obj[OBJ_MATCH].prop & PROP_LIT)
    PrintLineIndex(366);
  else
    PrintLineIndex(367);
}



void DoMisc_examine_candles(void)
{
  PrintTextIndex(368);
  if (Obj[OBJ_CANDLES].prop & PROP_LIT)
    PrintLineIndex(369);
  else
    PrintLineIndex(370);
}



void DoMisc_examine_torch(void)
{
  PrintLineIndex(371);
}



void DoMisc_examine_thief(void)
{
  PrintLineIndex(372);
}



void DoMisc_examine_tool_chest(void)
{
  PrintLineIndex(373);
}



void DoMisc_examine_board(void)
{
  PrintLineIndex(374);
}



void DoMisc_examine_chain(void)
{
  PrintLineIndex(375);
}



void DoMisc_open_tool_chest(void)
{
  PrintLineIndex(376);
}



void DoMisc_open_book(void)
{
  PrintLineIndex(377);
}



void DoMisc_close_book(void)
{
  PrintLineIndex(378);
}



void DoMisc_open_boarded_window(void)
{
  PrintLineIndex(379);
}



void DoMisc_break_boarded_window(void)
{
  PrintLineIndex(380);
}



void DoMisc_open_close_dam(void)
{
  PrintLineIndex(381);
}



void DoMisc_ring_hot_bell(void)
{
  PrintLineIndex(382);
}



void DoMisc_read_button(void)
{
  PrintLineIndex(383);
}



void DoMisc_raise_lower_granite_wall(void)
{
  PrintLineIndex(384);
}



void DoMisc_raise_lower_chain(void)
{
  PrintLineIndex(385);
}



void DoMisc_move_chain(void)
{
  PrintLineIndex(386);
}



void DoMisc_count_candles(void)
{
  PrintLineIndex(387);
}



void DoMisc_count_leaves(void)
{
  PrintLineIndex(388);
}



void DoMisc_examine_lamp(void)
{
  PrintTextIndex(389);

  if (LampTurnsLeft == 0)
    PrintLineIndex(390);
  else if (Obj[OBJ_LAMP].prop & PROP_LIT)
    PrintLineIndex(391);
  else
    PrintLineIndex(392);
}



void DoMisc_examine_troll(void)
{
  PrintDesc_Troll(1);
  PrintNewLine(); // above omits end newline
}



void DoMisc_examine_cyclops(void)
{
  if (CyclopsState == 3)
    PrintLineIndex(393);
  else
    PrintLineIndex(394);
}



void DoMisc_examine_white_house(void)
{
  PrintLineIndex(395);
}



void DoMisc_open_close_barrow_door(void)
{
  PrintLineIndex(396);
}



void DoMisc_open_close_studio_door(void)
{
  PrintLineIndex(397);
}



void DoMisc_open_close_bag_of_coins(void)
{
  PrintLineIndex(398);
}



void DoMisc_open_close_trunk(void)
{
  PrintLineIndex(399);
}



void DoMisc_open_close_large_bag(void)
{
  PrintLineIndex(400);
}



void DoMisc_open_front_door(void)
{
  PrintLineIndex(401);
}



void DoMisc_count_matches(void)
{
  PrintTextIndex(402);

  if (MatchesLeft == 0) PrintTextIndex(403);
  else PrintInteger(MatchesLeft);

  if (MatchesLeft == 1) PrintLineIndex(404);
  else                  PrintLineIndex(405);
}



void EatFood(int obj, const char *msg)
{
  if (Obj[obj].loc != INSIDE + OBJ_YOU)
    PrintLineIndex(406);
  else
  {
    PrintLine(msg);
    TimePassed = 1;
    Obj[obj].loc = 0;
  }
}



void DoMisc_eat_lunch(void)
{
  EatFood(OBJ_LUNCH, "Thank you very much. It really hit the spot.");
}



void DoMisc_eat_garlic(void)
{
  EatFood(OBJ_GARLIC, "What the heck! You won't make friends this way, but nobody around here is too friendly anyhow. Gulp!");
}



void DoMisc_drink_water(void)
{
  if (Room[Obj[OBJ_YOU].loc].prop & R_WATERHERE)
  {
    PrintLineIndex(407);
    TimePassed = 1;
  }
  else if (Obj[OBJ_BOTTLE].loc == Obj[OBJ_YOU].loc ||
           Obj[OBJ_BOTTLE].loc == INSIDE + OBJ_YOU)
  {
    if (Obj[OBJ_BOTTLE].loc != INSIDE + OBJ_YOU)
      PrintLineIndex(408);
    else if ((Obj[OBJ_BOTTLE].prop & PROP_OPEN) == 0)
      PrintLineIndex(233);
    else if (Obj[OBJ_WATER].loc != INSIDE + OBJ_BOTTLE)
      PrintLineIndex(409);
    else
    {
      PrintLineIndex(407);
      TimePassed = 1;
      Obj[OBJ_WATER].loc = 0;
    }
  }
  else
    PrintLineIndex(409);
}



void DoMisc_climbdown_rope(void)
{
  if (RopeTiedToRail && Obj[OBJ_YOU].loc == ROOM_DOME_ROOM)
    GoToRoutine(ROOM_TORCH_ROOM);
  else
    PrintLineIndex(410);
}



void DoMisc_break_mirror(void)
{
  if (MirrorBroken)
    PrintLineIndex(279);
  else
  {
    PrintLineIndex(280);
    TimePassed = 1;
    MirrorBroken = 1;
    NotLucky = 1;
  }
}



void DoMisc_lookin_mirror(void)
{
  if (MirrorBroken)
    PrintLineIndex(411);
  else
    PrintLineIndex(412);
}



void DoMisc_lookthrough_kitchen_window(void)
{
  PrintTextIndex(413);
  if (Obj[OBJ_YOU].loc == ROOM_KITCHEN)
    PrintLineIndex(414);
  else
    PrintLineIndex(415);
}



void DoMisc_lookunder_rug(void)
{
  if (RugMoved == 0 && TrapOpen == 0)
  {
    PrintLineIndex(416);
    TimePassed = 1;
  }
  else
    PrintLineIndex(417);
}



void DoMisc_lookunder_leaves(void)
{
  if (GratingRevealed == 0)
  {
    PrintLineIndex(418);
    TimePassed = 1;
    GratingRevealed = 1;
    Obj[OBJ_LEAVES].prop |= PROP_MOVEDDESC;
  }
  else
    PrintLineIndex(419);
}



void DoMisc_lookunder_rainbow(void)
{
  PrintLineIndex(420);
}



void DoMisc_lookin_chimney(void)
{
  PrintTextIndex(421);
  if (Obj[OBJ_YOU].loc == ROOM_KITCHEN) PrintTextIndex(422);
  else                                  PrintTextIndex(423);
  PrintLineIndex(424);
}



void DoMisc_examine_kitchen_window(void)
{
  if (KitchenWindowOpen == 0)
    PrintLineIndex(425);
  else
    PrintLineIndex(426);
}



void DoMisc_lookin_bag_of_coins(void)
{
  PrintLineIndex(427);
}



void DoMisc_lookin_trunk(void)
{
  PrintLineIndex(428);
}



void DoMisc_squeeze_tube(void)
{
  if (Obj[OBJ_TUBE].loc != INSIDE + OBJ_YOU)
    PrintLineIndex(429);
  else if ((Obj[OBJ_TUBE].prop & PROP_OPEN) == 0)
    PrintLineIndex(239);
  else if (Obj[OBJ_PUTTY].loc != INSIDE + OBJ_TUBE)
    PrintLineIndex(430);
  else
  {
    PrintLineIndex(431);
    TimePassed = 1;
    Obj[OBJ_PUTTY].loc = INSIDE + OBJ_YOU;
  }
}



void DoMisc_examine_raised_basket(void)
{
  PrintContents(OBJ_RAISED_BASKET, "It contains:", 1);
}



void DoMisc_examine_lowered_basket(void)
{
  PrintLineIndex(432);
}



void DoMisc_lookin_large_bag(void)
{
  if (ThiefDescType == 1) // unconscious
    PrintLineIndex(433);
  else
    PrintLineIndex(400);
}



void DoMisc_lookthrough_grate(void)
{
  if (Obj[OBJ_YOU].loc == ROOM_GRATING_CLEARING)
    PrintLineIndex(434);
  else
    PrintLineIndex(435);
}



void DoMisc_lookin_water(void)
{
  PrintLineIndex(436);
}



void DoMisc_whereis_granite_wall(void)
{
  switch (Obj[OBJ_YOU].loc)
  {
    case ROOM_NORTH_TEMPLE:  PrintLineIndex(437); break;
    case ROOM_TREASURE_ROOM: PrintLineIndex(438); break;
    case ROOM_SLIDE_ROOM:    PrintLineIndex(439);       break;
  }
}



void DoMisc_whereis_songbird(void)
{
  PrintLineIndex(440);
}



void DoMisc_whereis_white_house(void)
{
  switch (Obj[OBJ_YOU].loc)
  {
    case ROOM_KITCHEN:
    case ROOM_LIVING_ROOM:
    case ROOM_ATTIC:
      PrintLineIndex(441); // never printed because house is not in these locations
    break;

    case ROOM_EAST_OF_HOUSE:
    case ROOM_WEST_OF_HOUSE:
    case ROOM_NORTH_OF_HOUSE:
    case ROOM_SOUTH_OF_HOUSE:
      PrintLineIndex(442);
    break;

    case ROOM_CLEARING: PrintLineIndex(443);       break;
    default:            PrintLineIndex(444); break;
  }
}



void DoMisc_whereis_forest(void)
{
  PrintLineIndex(445);
}



void DoMisc_read_granite_wall(void)
{
  if (Obj[OBJ_YOU].loc == ROOM_SLIDE_ROOM)
    PrintLineIndex(439);
  else
    PrintLineIndex(446);
}



void DoMisc_examine_zorkmid(void)
{
  PrintLineIndex(447);
}



void DoMisc_examine_grue(void)
{
  PrintLineIndex(448);
}



void DoMisc_whereis_zorkmid(void)
{
  PrintLineIndex(449);
}



void DoMisc_whereis_grue(void)
{
  PrintLineIndex(450);
}



void DoMisc_listento_troll(void)
{
  PrintLineIndex(451);
}



void DoMisc_listento_thief(void)
{
  PrintLineIndex(452);
}



void DoMisc_listento_cyclops(void)
{
  PrintLineIndex(453);
}



void DoMisc_listento_forest(void)
{
  PrintLineIndex(454);
}



void DoMisc_listento_songbird(void)
{
  PrintLineIndex(455);
}



void DoMisc_cross_rainbow(void)
{
  if (Obj[OBJ_YOU].loc == ROOM_CANYON_VIEW)
    PrintLineIndex(456);
  else if (RainbowSolid == 0)
    PrintLineIndex(457);
  else
  {
    if (Obj[OBJ_YOU].loc == ROOM_ARAGAIN_FALLS)
      GoToRoutine(ROOM_END_OF_RAINBOW);
    else if (Obj[OBJ_YOU].loc == ROOM_END_OF_RAINBOW)
      GoToRoutine(ROOM_ARAGAIN_FALLS);
    else
      PrintLineIndex(458);
  }
}



void DoMisc_cross_lake(void)
{
  PrintLineIndex(459);
}



void DoMisc_cross_stream(void)
{
  PrintLineIndex(460);
}



void DoMisc_cross_chasm(void)
{
  PrintLineIndex(461);
}



void DoMisc_exorcise_ghosts(void)
{
  if (SpiritsBanished == 0 &&
      Obj[OBJ_BELL].loc == INSIDE + OBJ_YOU &&
      Obj[OBJ_BOOK].loc == INSIDE + OBJ_YOU &&
      Obj[OBJ_CANDLES].loc == INSIDE + OBJ_YOU)
    PrintLineIndex(462);
  else
    PrintLineIndex(463);
}



void DoMisc_raise_rug(void)
{
  PrintTextIndex(464);
  if (RugMoved)
    PrintLineIndex(56);
  else
    PrintLineIndex(465);
}



void DoMisc_raise_trap_door(void)
{
  DoMisc_open_trap_door();
}



void DoMisc_smell_gas(void)
{
  PrintLineIndex(466);
}



void DoMisc_smell_sandwich_bag(void)
{
  if (Obj[OBJ_LUNCH].loc == INSIDE + OBJ_SANDWICH_BAG)
    PrintLineIndex(467);
  else
    PrintLineIndex(468);
}



void CallDoMisc(int i)
{
  switch (i)
  {
    case   0: DoMisc_open_kitchen_window         (); return;
    case   1: DoMisc_close_kitchen_window        (); return;
    case   2: DoMisc_move_push_rug               (); return;
    case   3: DoMisc_move_push_rug               (); return;
    case   4: DoMisc_open_trap_door              (); return;
    case   5: DoMisc_close_trap_door             (); return;
    case   6: DoMisc_raise_basket                (); return;
    case   7: DoMisc_raise_basket                (); return;
    case   8: DoMisc_lower_basket                (); return;
    case   9: DoMisc_lower_basket                (); return;
    case  10: DoMisc_push_blue_button            (); return;
    case  11: DoMisc_push_red_button             (); return;
    case  12: DoMisc_push_brown_button           (); return;
    case  13: DoMisc_push_yellow_button          (); return;
    case  14: DoMisc_enter_inflated_boat         (); return;
    case  15: DoMisc_exit_inflated_boat          (); return;
    case  16: DoMisc_move_leaves                 (); return;
    case  17: DoMisc_open_grate                  (); return;
    case  18: DoMisc_close_grate                 (); return;
    case  19: DoMisc_ring_bell                   (); return;
    case  20: DoMisc_wind_canary                 (); return;
    case  21: DoMisc_wind_broken_canary          (); return;
    case  22: DoMisc_wave_sceptre                (); return;
    case  23: DoMisc_raise_sceptre               (); return;
    case  24: DoMisc_touch_mirror                (); return;
    case  25: DoMisc_touch_mirror                (); return;
    case  26: DoMisc_read_book                   (); return;
    case  27: DoMisc_read_advertisement          (); return;
    case  28: DoMisc_read_match                  (); return;
    case  29: DoMisc_read_map                    (); return;
    case  30: DoMisc_read_boat_label             (); return;
    case  31: DoMisc_read_guide                  (); return;
    case  32: DoMisc_read_tube                   (); return;
    case  33: DoMisc_read_owners_manual          (); return;
    case  34: DoMisc_read_prayer                 (); return;
    case  35: DoMisc_read_wooden_door            (); return;
    case  36: DoMisc_read_engravings             (); return;
    case  37: DoMisc_open_egg                    (); return;
    case  38: DoMisc_open_egg                    (); return;
    case  39: DoMisc_open_egg                    (); return;
    case  40: DoMisc_climbthrough_kitchen_window (); return;
    case  41: DoMisc_climbthrough_kitchen_window (); return;
    case  42: DoMisc_climbthrough_kitchen_window (); return;
    case  43: DoMisc_climbthrough_trap_door      (); return;
    case  44: DoMisc_climbthrough_trap_door      (); return;
    case  45: DoMisc_climbthrough_grate          (); return;
    case  46: DoMisc_climbthrough_grate          (); return;
    case  47: DoMisc_climbthrough_slide          (); return;
    case  48: DoMisc_climbthrough_slide          (); return;
    case  49: DoMisc_climbthrough_chimney        (); return;
    case  50: DoMisc_climbthrough_chimney        (); return;
    case  51: DoMisc_climbthrough_barrow_door    (); return;
    case  52: DoMisc_climbthrough_barrow_door    (); return;
    case  53: DoMisc_climbthrough_barrow_door    (); return;
    case  54: DoMisc_climbthrough_gate           (); return;
    case  55: DoMisc_climbthrough_gate           (); return;
    case  56: DoMisc_climbthrough_crack          (); return;
    case  57: DoMisc_climbthrough_crack          (); return;
    case  58: DoMisc_enter_white_house           (); return;
    case  59: DoMisc_slidedown_slide             (); return;
    case  60: DoMisc_climbup_mountain_range      (); return;
    case  61: DoMisc_climbup_mountain_range      (); return;
    case  62: DoMisc_climbup_white_cliff         (); return;
    case  63: DoMisc_climbup_white_cliff         (); return;
    case  64: DoMisc_climbup_tree                (); return;
    case  65: DoMisc_climbup_tree                (); return;
    case  66: DoMisc_climbdown_tree              (); return;
    case  67: DoMisc_climbup_chimney             (); return;
    case  68: DoMisc_climbup_chimney             (); return;
    case  69: DoMisc_climbdown_chimney           (); return;
    case  70: DoMisc_climbup_ladder              (); return;
    case  71: DoMisc_climbup_ladder              (); return;
    case  72: DoMisc_climbdown_ladder            (); return;
    case  73: DoMisc_climbup_slide               (); return;
    case  74: DoMisc_climbup_slide               (); return;
    case  75: DoMisc_climbdown_slide             (); return;
    case  76: DoMisc_climbup_climbable_cliff     (); return;
    case  77: DoMisc_climbup_climbable_cliff     (); return;
    case  78: DoMisc_climbdown_climbable_cliff   (); return;
    case  79: DoMisc_climbup_stairs              (); return;
    case  80: DoMisc_climbup_stairs              (); return;
    case  81: DoMisc_climbdown_stairs            (); return;
    case  82: DoMisc_examine_sword               (); return;
    case  83: DoMisc_examine_match               (); return;
    case  84: DoMisc_examine_candles             (); return;
    case  85: DoMisc_examine_torch               (); return;
    case  86: DoMisc_examine_thief               (); return;
    case  87: DoMisc_examine_tool_chest          (); return;
    case  88: DoMisc_examine_board               (); return;
    case  89: DoMisc_examine_chain               (); return;
    case  90: DoMisc_open_tool_chest             (); return;
    case  91: DoMisc_open_book                   (); return;
    case  92: DoMisc_close_book                  (); return;
    case  93: DoMisc_open_boarded_window         (); return;
    case  94: DoMisc_break_boarded_window        (); return;
    case  95: DoMisc_open_close_dam              (); return;
    case  96: DoMisc_open_close_dam              (); return;
    case  97: DoMisc_ring_hot_bell               (); return;
    case  98: DoMisc_read_button                 (); return;
    case  99: DoMisc_read_button                 (); return;
    case 100: DoMisc_read_button                 (); return;
    case 101: DoMisc_read_button                 (); return;
    case 102: DoMisc_raise_lower_granite_wall    (); return;
    case 103: DoMisc_raise_lower_granite_wall    (); return;
    case 104: DoMisc_raise_lower_chain           (); return;
    case 105: DoMisc_raise_lower_chain           (); return;
    case 106: DoMisc_move_chain                  (); return;
    case 107: DoMisc_count_candles               (); return;
    case 108: DoMisc_count_leaves                (); return;
    case 109: DoMisc_examine_lamp                (); return;
    case 110: DoMisc_examine_troll               (); return;
    case 111: DoMisc_examine_cyclops             (); return;
    case 112: DoMisc_examine_white_house         (); return;
    case 113: DoMisc_open_close_barrow_door      (); return;
    case 114: DoMisc_open_close_barrow_door      (); return;
    case 115: DoMisc_open_close_studio_door      (); return;
    case 116: DoMisc_open_close_studio_door      (); return;
    case 117: DoMisc_open_close_bag_of_coins     (); return;
    case 118: DoMisc_open_close_bag_of_coins     (); return;
    case 119: DoMisc_open_close_trunk            (); return;
    case 120: DoMisc_open_close_trunk            (); return;
    case 121: DoMisc_open_close_large_bag        (); return;
    case 122: DoMisc_open_close_large_bag        (); return;
    case 123: DoMisc_open_front_door             (); return;
    case 124: DoMisc_count_matches               (); return;
    case 125: DoMisc_count_matches               (); return;
    case 126: DoMisc_eat_lunch                   (); return;
    case 127: DoMisc_eat_garlic                  (); return;
    case 128: DoMisc_drink_water                 (); return;
    case 129: DoMisc_climbdown_rope              (); return;
    case 130: DoMisc_break_mirror                (); return;
    case 131: DoMisc_break_mirror                (); return;
    case 132: DoMisc_lookin_mirror               (); return;
    case 133: DoMisc_lookin_mirror               (); return;
    case 134: DoMisc_lookin_mirror               (); return;
    case 135: DoMisc_lookin_mirror               (); return;
    case 136: DoMisc_lookthrough_kitchen_window  (); return;
    case 137: DoMisc_lookthrough_kitchen_window  (); return;
    case 138: DoMisc_lookunder_rug               (); return;
    case 139: DoMisc_lookunder_leaves            (); return;
    case 140: DoMisc_lookunder_rainbow           (); return;
    case 141: DoMisc_lookin_chimney              (); return;
    case 142: DoMisc_lookin_chimney              (); return;
    case 143: DoMisc_examine_kitchen_window      (); return;
    case 144: DoMisc_lookin_bag_of_coins         (); return;
    case 145: DoMisc_lookin_bag_of_coins         (); return;
    case 146: DoMisc_lookin_trunk                (); return;
    case 147: DoMisc_lookin_trunk                (); return;
    case 148: DoMisc_squeeze_tube                (); return;
    case 149: DoMisc_examine_raised_basket       (); return;
    case 150: DoMisc_examine_lowered_basket      (); return;
    case 151: DoMisc_lookin_large_bag            (); return;
    case 152: DoMisc_lookin_large_bag            (); return;
    case 153: DoMisc_lookthrough_grate           (); return;
    case 154: DoMisc_lookthrough_grate           (); return;
    case 155: DoMisc_lookin_water                (); return;
    case 156: DoMisc_lookin_water                (); return;
    case 157: DoMisc_whereis_granite_wall        (); return;
    case 158: DoMisc_whereis_songbird            (); return;
    case 159: DoMisc_whereis_white_house         (); return;
    case 160: DoMisc_whereis_forest              (); return;
    case 161: DoMisc_read_granite_wall           (); return;
    case 162: DoMisc_examine_zorkmid             (); return;
    case 163: DoMisc_examine_grue                (); return;
    case 164: DoMisc_whereis_zorkmid             (); return;
    case 165: DoMisc_whereis_grue                (); return;
    case 166: DoMisc_listento_troll              (); return;
    case 167: DoMisc_listento_thief              (); return;
    case 168: DoMisc_listento_cyclops            (); return;
    case 169: DoMisc_listento_forest             (); return;
    case 170: DoMisc_listento_songbird           (); return;
    case 171: DoMisc_cross_rainbow               (); return;
    case 172: DoMisc_cross_lake                  (); return;
    case 173: DoMisc_cross_stream                (); return;
    case 174: DoMisc_cross_chasm                 (); return;
    case 175: DoMisc_exorcise_ghosts             (); return;
    case 176: DoMisc_raise_rug                   (); return;
    case 177: DoMisc_raise_trap_door             (); return;
    case 178: DoMisc_smell_gas                   (); return;
    case 179: DoMisc_smell_sandwich_bag          (); return;
  }
}
//*****************************************************************************



//*****************************************************************************
void PrintRandomFun(void)
{
  switch (GetRandom(4))
  {
    case 0: PrintLineIndex(469); break;
    case 1: PrintLineIndex(470);                     break;
    case 2: PrintLineIndex(471);                              break;
    case 3: PrintLineIndex(472);                   break;
  }
}



void PrintRandomJumpDeath(void)
{
  switch (GetRandom(3))
  {
    case 0: PrintLineIndex(473);                   break;
    case 1: PrintLineIndex(474); break;
    case 2: PrintLineIndex(475);                                                 break;
  }
}



void DoJump(void)
{
  int obj = 0;

  if (MatchCurWordIndex(WORD_ACROSS) || MatchCurWordIndex(WORD_FROM) || MatchCurWordIndex(WORD_IN) ||
      MatchCurWordIndex(WORD_INTO) || MatchCurWordIndex(WORD_OFF) || MatchCurWordIndex(WORD_OVER))
  {
    obj = GetAllObjFromInput(Obj[OBJ_YOU].loc); if (obj <= 0) return;

    if (obj == FOBJ_SCENERY_NOTVIS || obj == FOBJ_NOTVIS)
      {PrintLineIndex(476); return;}
    else if (obj == FOBJ_AMB)
      {PrintLineIndex(477); return;}
    else if (obj == OBJ_YOU)
      {PrintLineIndex(478); return;}
  }

  if (obj == 0 || obj >= NUM_OBJECTS)
  {
    switch (Obj[OBJ_YOU].loc)
    {
      case ROOM_KITCHEN:
      case ROOM_EAST_OF_CHASM:
      case ROOM_RESERVOIR:
      case ROOM_CHASM_ROOM:
      case ROOM_DOME_ROOM:
      case ROOM_SOUTH_TEMPLE:
      case ROOM_ARAGAIN_FALLS:
      case ROOM_SHAFT_ROOM:
        PrintLineIndex(479);
        PrintRandomJumpDeath();
        YoureDead(); // ##### RIP #####
      break;

      case ROOM_UP_A_TREE:
        PrintLineIndex(480);
        GoToRoutine(ROOM_PATH);
      break;

      default:
        PrintRandomFun();
      break;
    }
  }
  else if (Obj[obj].loc == Obj[OBJ_YOU].loc)
  {
    if (Obj[obj].prop & PROP_ACTOR)
      PrintLineIndex(481);
    else
      PrintRandomFun();
  }
  else
    PrintLineIndex(482);
}



void DoSleep(void)
{
  PrintLineIndex(483);
}



void DoDisembark(void)
{
  if (YouAreInBoat == 0)
    PrintLineIndex(484);
  else
    DoMisc_exit_inflated_boat();
}



void BoatGoToRoutine(int newroom)
{
  int prev_darkness;

  if ((Room[newroom].prop & R_BODYOFWATER) == 0)
    PrintLineIndex(485);

  Obj[OBJ_INFLATED_BOAT].loc = newroom;

  prev_darkness = IsPlayerInDarkness();

  Obj[OBJ_YOU].loc = newroom;
  TimePassed = 1;

  if (IsPlayerInDarkness())
  {
    if (prev_darkness)
    {
      //kill player that tried to go from dark to dark
      PrintLineIndex(7);
      YoureDead(); // ##### RIP #####
      return;
    }
    else PrintLineIndex(8);
  }

  PrintPlayerRoomDesc(0);
}



void DoLaunch(void)
{
  int i;
  int launch_from[8] = {ROOM_DAM_BASE, ROOM_WHITE_CLIFFS_NORTH, ROOM_WHITE_CLIFFS_SOUTH, ROOM_SHORE,
                        ROOM_SANDY_BEACH, ROOM_RESERVOIR_SOUTH, ROOM_RESERVOIR_NORTH, ROOM_STREAM_VIEW};
  int   launch_to[8] = {ROOM_RIVER_1, ROOM_RIVER_3, ROOM_RIVER_4, ROOM_RIVER_5, ROOM_RIVER_4, ROOM_RESERVOIR,
                        ROOM_RESERVOIR, ROOM_IN_STREAM};

  if (Room[Obj[OBJ_YOU].loc].prop & R_BODYOFWATER)
  {
    PrintTextIndex(486);
    if (Obj[OBJ_YOU].loc == ROOM_RESERVOIR)
      PrintTextIndex(487);
    else if (Obj[OBJ_YOU].loc == ROOM_IN_STREAM)
      PrintTextIndex(488);
    else
      PrintTextIndex(489);
    PrintLineIndex(490);
    return;
  }

  if (YouAreInBoat == 0) {PrintLineIndex(491); return;}

  for (i=0; i<8; i++)
    if (Obj[OBJ_YOU].loc == launch_from[i]) break;
  if (i == 8) {PrintLineIndex(492); return;}

  DownstreamCounter = -1; // start at -1 to account for this turn
  BoatGoToRoutine(launch_to[i]);
}



void DoLand(void)
{
  if ((Room[Obj[OBJ_YOU].loc].prop & R_BODYOFWATER) == 0)
    {PrintLineIndex(493); return;}

  switch (Obj[OBJ_YOU].loc)
  {
    case ROOM_RESERVOIR : PrintLineIndex(494); break;
    case ROOM_RIVER_2   : PrintLineIndex(495);            break;
    case ROOM_RIVER_4   : PrintLineIndex(496);   break;

    case ROOM_IN_STREAM : BoatGoToRoutine(ROOM_STREAM_VIEW       ); break;
    case ROOM_RIVER_1   : BoatGoToRoutine(ROOM_DAM_BASE          ); break;
    case ROOM_RIVER_3   : BoatGoToRoutine(ROOM_WHITE_CLIFFS_NORTH); break;
    case ROOM_RIVER_5   : BoatGoToRoutine(ROOM_SHORE             ); break;

    default: PrintLineIndex(493); break;
  }
}



void DoEcho(void)
{
  if (Obj[OBJ_YOU].loc == ROOM_LOUD_ROOM &&
      LoudRoomQuiet == 0 &&
      (GatesOpen || LowTide == 0))
  {
    LoudRoomQuiet = 1;
    Obj[OBJ_BAR].prop &= ~PROP_SACRED;
    PrintLineIndex(497);
    TimePassed = 1;
  }
  else
    PrintLineIndex(498);
}



void DoPray(void)
{
  TimePassed = 1;

  if (Obj[OBJ_YOU].loc == ROOM_SOUTH_TEMPLE)
  {
    if (YouAreDead)
    {
      PrintLineIndex(499);

      YouAreDead = 0;
      if (Obj[OBJ_TROLL].loc == ROOM_TROLL_ROOM)
        TrollAllowsPassage = 0;
      Obj[OBJ_LAMP].prop &= ~PROP_INVISIBLE;
      Obj[OBJ_YOU].prop &= ~PROP_LIT;
    }
    else
      YouAreInBoat = 0; // in case you're in it

    ExitFound = 1;
    GoToRoutine(ROOM_FOREST_1);
  }
  else
  {
    if (YouAreDead)
      PrintLineIndex(500);
    else
      PrintLineIndex(501);
  }
}



void DoVersion(void)
{
  PrintTextIndex(502);
  PrintText(__DATE__);
  PrintTextIndex(503);
  PrintLine(__TIME__);
}



void DoDiagnose(void)
{
  int wounds, count, death_dist = PlayerFightStrength(0) + PlayerStrength;

  if (EnableCureRoutine == 0) wounds = 0;
  else                        wounds = -PlayerStrength;

  if (wounds == 0)
    PrintLineIndex(504);
  else
  {
    PrintTextIndex(402);
         if (wounds == 1) PrintTextIndex(505);
    else if (wounds == 2) PrintTextIndex(506);
    else if (wounds == 3) PrintTextIndex(507);
    else                  PrintTextIndex(508);
    PrintTextIndex(509);
    count = CURE_WAIT * (wounds - 1) + EnableCureRoutine;
    PrintInteger(count);
    if (count == 1) PrintLineIndex(510);
    else            PrintLineIndex(511);
  }

  PrintTextIndex(512);
       if (death_dist == 0) PrintLineIndex(513);
  else if (death_dist == 1) PrintLineIndex(514);
  else if (death_dist == 2) PrintLineIndex(515);
  else if (death_dist == 3) PrintLineIndex(516);
  else                      PrintLineIndex(517);

  if (NumDeaths != 0)
  {
    PrintTextIndex(518);
    if (NumDeaths == 1) PrintLineIndex(519);
    else                PrintLineIndex(520);
  }
}



void DoOdysseus(void)
{
  if (Obj[OBJ_YOU].loc != ROOM_CYCLOPS_ROOM || Obj[OBJ_CYCLOPS].loc == 0)
    PrintLineIndex(521);
  else if (CyclopsState == 3)
    PrintLineIndex(522);
  else
  {
    CyclopsState = 4;
    Obj[OBJ_CYCLOPS].loc = 0;
    PrintLineIndex(523);
    TimePassed = 1;
    ExitFound = 1;
  }
}



void DoSwim(void)
{
  if (Room[Obj[OBJ_YOU].loc].prop & R_WATERHERE)
    PrintLineIndex(524);
  else
    PrintLineIndex(525);
}



void DoTempleTreasure(void)
{
  if (Obj[OBJ_YOU].loc == ROOM_NORTH_TEMPLE)
    GoToRoutine(ROOM_TREASURE_ROOM);
  else if (Obj[OBJ_YOU].loc == ROOM_TREASURE_ROOM)
    GoToRoutine(ROOM_NORTH_TEMPLE);
  else
    PrintLineIndex(526);
}



void DoIntro(void)
{
  PrintLineIndex(527);
}
//*****************************************************************************



//*****************************************************************************
void ActorResponse(int obj, int odysseus)
{
  switch (obj)
  {
    case OBJ_CYCLOPS:
      if (odysseus)
        DoOdysseus();
      else
        PrintLineIndex(528);
    break;

    case OBJ_GHOSTS:
      PrintLineIndex(529);
    break;

    case OBJ_BAT:
      PrintLineIndex(530);
    break;

    case OBJ_THIEF:
      PrintLineIndex(531);
    break;

    case OBJ_TROLL:
      PrintLineIndex(532);
    break;
  }
}



int VerifyActor(int obj)
{
  if (obj == FOBJ_SCENERY_NOTVIS || obj == FOBJ_NOTVIS)
    {PrintLineIndex(478); return 1;}
  else if (obj == FOBJ_AMB)
    {PrintLineIndex(533); return 1;}
  else if (obj == OBJ_YOU || obj >= NUM_OBJECTS)
    {PrintLineIndex(478); return 1;}
  else if ((Obj[obj].prop & PROP_ACTOR) == 0)
    {PrintLineIndex(478); return 1;}
  else if (Obj[obj].loc != Obj[OBJ_YOU].loc)
    {PrintLineIndex(534); return 1;}

  return 0;
}



// actor, *** until end of input

void DoCommandActor(int obj)
{
  int odysseus = 0;

  while (CurWord < NumStrWords)
  {
    if (MatchCurWordIndex(WORD_ODYSSEUS) || MatchCurWordIndex(WORD_ULYSSES))
      odysseus = 1;
    else
      CurWord++;
  }

  if (VerifyActor(obj) == 0)
    ActorResponse(obj, odysseus);
}



// talkto/ask/tell actor (about) (***)

void DoTalkTo(void)
{
  int obj, odysseus = 0;

  obj = GetAllObjFromInput(Obj[OBJ_YOU].loc); if (obj <= 0) return;
  if (VerifyActor(obj)) return;

  while (CurWord < NumStrWords)
  {
    if (MatchCurWordIndex(WORD_THEN))
      {CurWord--; break;} // end of this turn's command; back up so "then" can be matched later
    else if (MatchCurWordIndex(WORD_ODYSSEUS) || MatchCurWordIndex(WORD_ULYSSES))
      odysseus = 1;
    else
      CurWord++;
  }

  ActorResponse(obj, odysseus);
}



// greet/hello (,) actor

void DoGreet(void)
{
  int obj, odysseus = 0;

  MatchCurWordIndex(WORD_AND); // and = ,
  obj = GetAllObjFromInput(Obj[OBJ_YOU].loc); if (obj <= 0) return;
  if (VerifyActor(obj)) return;

  if (obj == OBJ_THIEF && ThiefDescType == 1) // unconscious
    PrintLineIndex(535);
  else if (obj == OBJ_TROLL && TrollDescType == 1) // unconscious
    PrintLineIndex(536);
  else
    ActorResponse(obj, odysseus);
}



// say *** (to actor)

void DoSay(void)
{
  int obj = 0, odysseus = 0, temp;

  while (CurWord < NumStrWords)
  {
    if (MatchCurWordIndex(WORD_TO))
      {CurWord--; break;} // back up so "to" can be matched below
    else if (MatchCurWordIndex(WORD_ODYSSEUS) || MatchCurWordIndex(WORD_ULYSSES))
      odysseus = 1;
    else
      CurWord++;
  }

  if (MatchCurWordIndex(WORD_TO))
  {
    obj = GetAllObjFromInput(Obj[OBJ_YOU].loc); if (obj <= 0) return;
  }

  if (obj == 0)
  {
    // look for exactly one actor in player's room who is described (thief can be invisible)
    for (temp=2; temp<NUM_OBJECTS; temp++)
      if ((Obj[temp].prop & PROP_ACTOR) &&
          (Obj[temp].prop & PROP_INVISIBLE) == 0 &&
          Obj[temp].loc == Obj[OBJ_YOU].loc)
        {if (obj == 0) obj = temp; else {obj = 0; break;}}
    if (obj == 0) // more than one or no actors
      {PrintLineIndex(537); return;}
  }

  if (VerifyActor(obj) == 0)
    ActorResponse(obj, odysseus);
}
//*****************************************************************************



//*****************************************************************************

// handle things like water and boats

int ActionDirectionRoutine(int newroom)
{
  if (Room[Obj[OBJ_YOU].loc].prop & R_BODYOFWATER)
  {
    //move from water to land or water

    if ((Room[newroom].prop & R_BODYOFWATER) == 0)
      PrintLineIndex(485);

    Obj[OBJ_INFLATED_BOAT].loc = newroom;

    DownstreamCounter = -1; // in case of moving to water; start at -1 to account for this turn
  }
  else
  {
    //move from land
    if (YouAreInBoat)
    {
      PrintLineIndex(6);
      return 1;
    }
  }
  return 0;
}
//*****************************************************************************



//*****************************************************************************

// returns 0 if action not intercepted

int InterceptActionWhenDead(int action)
{
  if (YouAreDead == 0)
    return 0;

  if (action == A_GO || (action >= A_NORTH && action <= A_OUT))
    return 0;

  switch (action)
  {
    case A_QUIT: case A_RESTART: case A_RESTORE: case A_SAVE:
    case A_BRIEF: case A_VERBOSE: case A_SUPERBRIEF: case A_VERSION:
    case A_PRAY:
      return 0;

    // A_BURN
    case A_OPEN: case A_CLOSE: case A_EAT: case A_DRINK: case A_INFLATE: case A_DEFLATE:
    case A_TURN: case A_TIE: case A_UNTIE: case A_TOUCH:
      PrintLineIndex(538);
    break;

    case A_SCORE:
      PrintLineIndex(539);
    break;

    case A_DIAGNOSE:
      PrintLineIndex(540);
    break;

    case A_WAIT:
      PrintLineIndex(541);
    break;

    case A_ACTIVATE:
      PrintLineIndex(542);
    break;

    case A_TAKE:
      PrintLineIndex(543);
    break;

    case A_BREAK:
      PrintLineIndex(544);
    break;

    // A_THROW
    case A_DROP: case A_INVENTORY:
      PrintLineIndex(545);
    break;

    case A_LOOK:
      PrintPlayerRoomDesc(1);
      PrintTextIndex(546);
      if (GetNumObjectsInLocation(Obj[OBJ_YOU].loc) == 0)
        PrintLineIndex(56);
      else
        PrintLineIndex(547);
      if ((Room[Obj[OBJ_YOU].loc].prop & R_LIT) == 0)
        PrintLineIndex(548);
    break;

    default:
      PrintLineIndex(549);
    break;
  }

  return 1;
}



void PrintEcho(int capital_flag)
{
  int i;
  unsigned int w;

  for (i=0,w=0; i<NumStrWords; i++)
  {
    w = StrWord[i];
    if (w != 0 && w != WORD_AND && w != WORD_THEN)
      break;
  }

  if (w == 0)
    PrintTextIndex(550);
  else
    PrintWord(w, capital_flag);
}



// returns 0 if action not intercepted

int InterceptActionInLoudRoom(int action)
{
  if (Obj[OBJ_YOU].loc != ROOM_LOUD_ROOM)
    return 0;

  if (LoudRoomQuiet || (GatesOpen == 0 && LowTide))
    return 0; // room not loud

  if ((action >= A_NORTH && action <= A_OUT) || action == A_GO ||
      action == A_SAVE || action == A_RESTORE || action == A_QUIT ||
      action == A_ECHO)
    return 0; // let these commands through

  PrintEcho(1);
  PrintChar(' ');
  PrintEcho(0);
  PrintLineIndex(550);

  return 1;
}



int InterceptAction(int action)
{
  if (InterceptActionWhenDead(action))   return 1;
  if (InterceptActionInLoudRoom(action)) return 1;

  return 0;
}
//*****************************************************************************



//*****************************************************************************

// returns 0 if take should go ahead

int InterceptTakeObj(int obj)
{
  switch (obj)
  {
    case OBJ_BAT:           PrintLineIndex(205); return 1;
    case OBJ_CYCLOPS:       PrintLineIndex(551); TimePassed = 1; return 1;
    case OBJ_THIEF:         PrintLineIndex(552); return 1;
    case OBJ_TROLL:         PrintLineIndex(553); TimePassed = 1; return 1;
    case OBJ_MACHINE:       PrintLineIndex(554); return 1;
    case OBJ_TROPHY_CASE:   PrintLineIndex(555); return 1;
    case OBJ_MAILBOX:       PrintLineIndex(556); return 1;
    case OBJ_KITCHEN_TABLE: PrintLineIndex(557); return 1;
    case OBJ_ATTIC_TABLE:   PrintLineIndex(557); return 1;
    case OBJ_HOT_BELL:      PrintLineIndex(558); return 1;

    case OBJ_WATER:
      if ((Room[Obj[OBJ_YOU].loc].prop & R_WATERHERE) == 0 &&
           !(IsObjVisible(OBJ_BOTTLE) &&
             (Obj[OBJ_BOTTLE].prop & PROP_OPEN) &&
             Obj[OBJ_WATER].loc == INSIDE + OBJ_BOTTLE))
        PrintLineIndex(200);
      else
        PrintLineIndex(559);
      return 1;

    case OBJ_TOOL_CHEST:
      PrintLineIndex(560);
      Obj[OBJ_TOOL_CHEST].loc = 0;
      return 1;

    case OBJ_ROPE:
      if (RopeTiedToRail)
        {PrintLineIndex(561); return 1;}
    break;

    case OBJ_RUSTY_KNIFE:
      if (Obj[OBJ_SWORD].loc == INSIDE + OBJ_YOU)
        PrintLineIndex(562);
    break;

    case OBJ_CHALICE:
      if (Obj[OBJ_CHALICE].loc == ROOM_TREASURE_ROOM &&
          Obj[OBJ_THIEF].loc == ROOM_TREASURE_ROOM &&
          (Obj[OBJ_THIEF].prop & PROP_INVISIBLE) == 0 &&
          VillainAttacking[VILLAIN_THIEF] &&
          ThiefDescType != 1) // not unconscious
        {PrintLineIndex(563); return 1;}
    break;

    case OBJ_LARGE_BAG:
      if (ThiefDescType == 1) // unconscious
        PrintLineIndex(564);
      else
        PrintLineIndex(565);
      return 1;
  }

  return 0;
}



// if player is inside vehicle, return vehicle obj; otherwise return 0

int GetPlayersVehicle(void)
{
  if (YouAreInBoat)
    return OBJ_INFLATED_BOAT;
  else
    return 0;
}



void MoveTreasuresToLandOfLivingDead(int loc)
{
  int obj;

  for (obj=2; obj<NUM_OBJECTS; obj++)
    if (Obj[obj].loc == loc &&
        (Obj[obj].prop & PROP_INVISIBLE) == 0 &&
        (Obj[obj].prop & PROP_SACRED) == 0 &&
        Obj[obj].thiefvalue > 0)
  {
    Obj[obj].loc = ROOM_LAND_OF_LIVING_DEAD;
    Obj[obj].prop |= PROP_MOVEDDESC;
  }
}



// returns 1 if intercepted

int InterceptTakeFixedObj(int obj)
{
  switch (obj)
  {
    case FOBJ_BOARD:        PrintLineIndex(374);                          return 1;
    case FOBJ_SONGBIRD:     PrintLineIndex(440);           return 1;
    case FOBJ_BODIES:       PrintLineIndex(566);                  return 1;
    case FOBJ_RUG:          PrintLineIndex(567);          return 1;
    case FOBJ_NAILS:        PrintLineIndex(568); return 1;
    case FOBJ_GRANITE_WALL: PrintLineIndex(384);                                        return 1;
    case FOBJ_CHAIN:        PrintLineIndex(386);                                       return 1;

    case FOBJ_BOLT:
    case FOBJ_BUBBLE:
      PrintLineIndex(569);
      return 1;

    case FOBJ_MIRROR2:
    case FOBJ_MIRROR1:
      PrintLineIndex(570);
      return 1;

    case FOBJ_BONES:
      PrintLineIndex(571);
      MoveTreasuresToLandOfLivingDead(Obj[OBJ_YOU].loc);
      MoveTreasuresToLandOfLivingDead(INSIDE + OBJ_YOU);
      return 1;
  }

  return 0;
}



int InterceptTakeOutOf(int container)
{
  switch (container)
  {
    case OBJ_LARGE_BAG:
    {
      PrintLineIndex(572);
      return 1;
    }
  }

  return 0;
}



// test flag:  1 if no changes should be made (yet)
// multi flag: 1 if obj name should be printed

// returns:
//   1:  intercepted, and obj MUST leave inventory, unless container is full
//  -1:  intercepted and calling function should immediately return

int InterceptDropPutObj(int obj, int container, int test, int multi)
{
  switch (container)
  {
    case OBJ_LOWERED_BASKET:
    {
      if (multi) {PrintObjectDesc(obj, 0); PrintTextIndex(573);}
      PrintLineIndex(432);
      return -1;
    }

    case OBJ_CHALICE:
    {
      if (multi) {PrintObjectDesc(obj, 0); PrintTextIndex(573);}
      PrintLineIndex(574);
      return -1;
    }

    case OBJ_LARGE_BAG:
    {
      if (multi) {PrintObjectDesc(obj, 0); PrintTextIndex(573);}
      PrintLineIndex(572);
      return -1;
    }

    case FOBJ_GRATE:
    {
      if (GetObjectSize(obj) > 20)
      {
        if (multi) {PrintObjectDesc(obj, 0); PrintTextIndex(573);}
        PrintLineIndex(575);
        return -1;
      }
      else if (Obj[OBJ_YOU].loc != ROOM_GRATING_CLEARING)
      {
        if (multi) {PrintObjectDesc(obj, 0); PrintTextIndex(573);}
        PrintLineIndex(576);
        return -1;
      }

      if (test == 0)
      {
        if (multi) {PrintObjectDesc(obj, 0); PrintTextIndex(573);}
        PrintLineIndex(577);

        Obj[obj].loc = ROOM_GRATING_ROOM;
        MoveObjOrderToLast(obj);
        TimePassed = 1;
      }

      return 1;
    }

    case FOBJ_SLIDE:
    {
      if (test == 0)
      {
        if (multi) {PrintObjectDesc(obj, 0); PrintTextIndex(573);}
        if (Obj[OBJ_YOU].loc == ROOM_SLIDE_ROOM)
          PrintLineIndex(578);
        else
          PrintLineIndex(579);

        Obj[obj].loc = ROOM_CELLAR;
        MoveObjOrderToLast(obj);
        TimePassed = 1;
      }

      return 1;
    }

    case FOBJ_RIVER:
    case OBJ_WATER:
    {
      if ((Room[Obj[OBJ_YOU].loc].prop & R_WATERHERE) == 0)
      {
        if (multi) {PrintObjectDesc(obj, 0); PrintTextIndex(573);}
        PrintLineIndex(580);
        return -1;
      }

      if (obj == OBJ_INFLATED_BOAT)
      {
        if (multi) {PrintObjectDesc(obj, 0); PrintTextIndex(573);}
        PrintLineIndex(581);
        return -1;
      }

      if (test == 0)
      {
        if (multi) {PrintObjectDesc(obj, 0); PrintTextIndex(573);}
        if (Obj[obj].prop & PROP_INFLAMMABLE)
          PrintLineIndex(582);
        else
          PrintLineIndex(583);

        Obj[obj].loc = 0;
        TimePassed = 1;
      }

      return 1;
    }
  }

  if (container >= NUM_OBJECTS)
  {
    PrintLineIndex(584);
    return -1;
  }

  return 0; // not intercepted
}

//*****************************************************************************



//*****************************************************************************
int IsActorInRoom(int room)
{
  int obj;

  for (obj=2; obj<NUM_OBJECTS; obj++)
    if (Obj[obj].loc == room &&
        (Obj[obj].prop & PROP_ACTOR) &&
        (Obj[obj].prop & PROP_INVISIBLE) == 0)
    return 1;

  return 0;
}



// thiefvalue for sword indicates glow level

void SwordRoutine(void)
{
  int glow, new_glow, i, room;

  if (Obj[OBJ_SWORD].loc != INSIDE + OBJ_YOU) return;

  glow = Obj[OBJ_SWORD].thiefvalue;
  new_glow = 0;

  if (IsActorInRoom(Obj[OBJ_YOU].loc))
    new_glow = 2;
  else
    for (i=0; i<10; i++)
  {
    room = GetRoomPassage(Obj[OBJ_YOU].loc, i);
    if (room > 0 && room < NUM_ROOMS && IsActorInRoom(room))
    {
      new_glow = 1;
      break;
    }
  }

  if (new_glow != glow)
  {
         if (new_glow == 0) PrintLineIndex(585);
    else if (new_glow == 1) PrintLineIndex(363);
    else                    PrintLineIndex(586);
    Obj[OBJ_SWORD].thiefvalue = new_glow;
  }
}



void LampDrainRoutine(void)
{
  if (Obj[OBJ_LAMP].loc == 0) return; // destroyed by machine or lost

  if ((Obj[OBJ_LAMP].prop & PROP_LIT) == 0) return;

  if (LampTurnsLeft > 0) LampTurnsLeft--;

  if (IsObjVisible(OBJ_LAMP))
    switch (LampTurnsLeft)
  {
    case 100: PrintLineIndex(587);     break;
    case  70: PrintLineIndex(588); break;
    case  15: PrintLineIndex(589);            break;
  }

  if (LampTurnsLeft == 0)
  {
    int prev_darkness;

    prev_darkness = IsPlayerInDarkness();
    Obj[OBJ_LAMP].prop &= ~PROP_LIT;
    if (IsPlayerInDarkness() != prev_darkness)
    {
      PrintNewLine();
      PrintPlayerRoomDesc(1);
    }
  }
}



// also handles:
//   candles in ceremony
//   candles put out by draft
//   items destroyed when candle is put inside them

void CandlesShrinkRoutine(void)
{
  int prev_darkness;

  if (Obj[OBJ_CANDLES].loc == 0) return; // destroyed by machine or lost

  if ((Obj[OBJ_CANDLES].prop & PROP_MOVEDDESC) == 0) return; // still sitting on altar

  if ((Obj[OBJ_CANDLES].prop & PROP_LIT) == 0) return; // not lit


  if (Obj[OBJ_CANDLES].loc == INSIDE + OBJ_YOU &&
      Obj[OBJ_YOU].loc == ROOM_ENTRANCE_TO_HADES &&
      BellRungCountdown > 0 &&
      CandlesLitCountdown == 0)
  {
    PrintLineIndex(590);
    BellRungCountdown = 0;
    CandlesLitCountdown = 3;
  }


  if (CandleTurnsLeft > 0) CandleTurnsLeft--;

  if (IsObjVisible(OBJ_CANDLES))
    switch (CandleTurnsLeft)
  {
    case 20: PrintLineIndex(591);                           break;
    case 10: PrintLineIndex(592);               break;
    case  5: PrintLineIndex(593);                    break;
    case  0: PrintLineIndex(594); break;
  }


  prev_darkness = IsPlayerInDarkness();


  if (CandleTurnsLeft == 0)
    Obj[OBJ_CANDLES].prop &= ~PROP_LIT;
  else if (Obj[OBJ_CANDLES].loc == INSIDE + OBJ_INFLATED_BOAT)
  {
    if (IsObjVisible(OBJ_INFLATED_BOAT))
      PrintLineIndex(595);
    Obj[OBJ_INFLATED_BOAT].loc = 0;
    Obj[OBJ_CANDLES].loc = 0;
  }
  else if (Obj[OBJ_CANDLES].loc == INSIDE + OBJ_NEST)
  {
    if (IsObjVisible(OBJ_NEST))
      PrintLineIndex(596);
    Obj[OBJ_NEST].loc = 0;
    Obj[OBJ_CANDLES].loc = 0;
  }
  else if (Obj[OBJ_YOU].loc == ROOM_TINY_CAVE && PercentChance(50, 80))
  {
    if (IsObjVisible(OBJ_CANDLES)) PrintLineIndex(597);
    Obj[OBJ_CANDLES].prop &= ~PROP_LIT;
  }


  if (IsPlayerInDarkness() != prev_darkness)
  {
    PrintNewLine();
    PrintPlayerRoomDesc(1);
  }
}



void ReservoirFillRoutine(void)
{
  if (ReservoirFillCountdown == 0) return;
      ReservoirFillCountdown--;
  if (ReservoirFillCountdown > 0) return;

  Room[ROOM_RESERVOIR  ].prop |= R_BODYOFWATER;
  Room[ROOM_DEEP_CANYON].prop &= ~R_DESCRIBED;
  Room[ROOM_LOUD_ROOM  ].prop &= ~R_DESCRIBED;

  LowTide = 0;

  switch (Obj[OBJ_YOU].loc)
  {
    case ROOM_RESERVOIR:
      if (YouAreInBoat)
        PrintLineIndex(598);
      else
      {
        PrintLineIndex(599);
        YoureDead(); // ##### RIP #####
      }
    break;

    case ROOM_DEEP_CANYON:
      PrintLineIndex(600);
    break;

    case ROOM_LOUD_ROOM:
      if (LoudRoomQuiet == 0)
      {
        int random_room[3] = {ROOM_DAMP_CAVE, ROOM_ROUND_ROOM, ROOM_DEEP_CANYON};

        PrintLineIndex(601);

        YouAreInBoat = 0; // in case you're in it
        GoToRoutine(random_room[GetRandom(3)]);
      }
    break;

    case ROOM_RESERVOIR_NORTH:
    case ROOM_RESERVOIR_SOUTH:
      PrintLineIndex(602);
    break;
  }
}



void ReservoirDrainRoutine(void)
{
  if (ReservoirDrainCountdown == 0) return;
      ReservoirDrainCountdown--;
  if (ReservoirDrainCountdown > 0) return;

  Room[ROOM_RESERVOIR  ].prop &= ~R_BODYOFWATER;
  Room[ROOM_DEEP_CANYON].prop &= ~R_DESCRIBED;
  Room[ROOM_LOUD_ROOM  ].prop &= ~R_DESCRIBED;

  LowTide = 1;

  switch (Obj[OBJ_YOU].loc)
  {
    case ROOM_RESERVOIR:
      if (YouAreInBoat)
        PrintLineIndex(603);
    break;

    case ROOM_DEEP_CANYON:
      PrintLineIndex(604);
    break;

    case ROOM_RESERVOIR_NORTH:
    case ROOM_RESERVOIR_SOUTH:
      PrintLineIndex(605);
    break;
  }
}



void SinkingObjectsRoutine(void)
{
  int obj, i;
  int check_room[7] = {ROOM_RESERVOIR, ROOM_IN_STREAM, ROOM_RIVER_1, ROOM_RIVER_2, ROOM_RIVER_3,
                       ROOM_RIVER_4, ROOM_RIVER_5};

  for (obj=2; obj<NUM_OBJECTS; obj++)
    if (obj != OBJ_INFLATED_BOAT && obj != OBJ_BUOY && obj != OBJ_THIEF)
      for (i=0; i<7; i++)
        if (Obj[obj].loc == check_room[i])
  {
    if ((Room[check_room[i]].prop & R_BODYOFWATER) && (Obj[obj].prop & PROP_INVISIBLE) == 0)
    {
      // if room is filled with water and object hasn't sunk, sink object
      Obj[obj].prop |= PROP_INVISIBLE;
    }
    else if ((Room[check_room[i]].prop & R_BODYOFWATER) == 0 && (Obj[obj].prop & PROP_INVISIBLE))
    {
      // if room is not filled with water and object has sunk, unsink object
      Obj[obj].prop &= ~PROP_INVISIBLE;
    }
  }
}



void LoudRoomRoutine(void)
{
  if (Obj[OBJ_YOU].loc == ROOM_LOUD_ROOM && LoudRoomQuiet == 0 && GatesOpen && LowTide == 0)
  {
    int random_room[3] = {ROOM_DAMP_CAVE, ROOM_ROUND_ROOM, ROOM_DEEP_CANYON};

    PrintLineIndex(606);

    YouAreInBoat = 0; // in case you're in it

    GoToRoutine(random_room[GetRandom(3)]);
  }
}



void MaintenanceLeakRoutine(void)
{
  if (MaintenanceWaterLevel <= 0 || MaintenanceWaterLevel > 16) return;

  if (Obj[OBJ_YOU].loc == ROOM_MAINTENANCE_ROOM)
  {
    PrintTextIndex(607);

    switch (MaintenanceWaterLevel / 2)
    {
      case 0: PrintLineIndex(608);  break;
      case 1: PrintLineIndex(609);    break;
      case 2: PrintLineIndex(610);   break;
      case 3: PrintLineIndex(611);    break;
      case 4: PrintLineIndex(612);   break;
      case 5: PrintLineIndex(613);   break;
      case 6: PrintLineIndex(614);    break;
      case 7: PrintLineIndex(615);     break;
      case 8: PrintLineIndex(616); break;
    }
  }

  MaintenanceWaterLevel++;
  if (MaintenanceWaterLevel > 16 && Obj[OBJ_YOU].loc == ROOM_MAINTENANCE_ROOM)
  {
    PrintLineIndex(617);
    if (YouAreInBoat)
      switch (Obj[OBJ_YOU].loc)
    {
      case ROOM_MAINTENANCE_ROOM:
      case ROOM_DAM_ROOM:
      case ROOM_DAM_LOBBY:
        PrintLineIndex(618);
      break;
    }
    YoureDead(); // ##### RIP #####
  }
}



void BoatPuncturedRoutine(void)
{
  int i, flag;
  int pointy_obj[6] = {OBJ_SCEPTRE, OBJ_KNIFE, OBJ_SWORD, OBJ_RUSTY_KNIFE, OBJ_AXE, OBJ_STILETTO};

  flag = 0;
  for (i=0; i<6; i++)
    if (Obj[pointy_obj[i]].loc == INSIDE + OBJ_INFLATED_BOAT)
  {
    flag = 1;
    Obj[pointy_obj[i]].loc = Obj[OBJ_INFLATED_BOAT].loc;
  }
  if (flag == 0) return;

  PrintLineIndex(619);

  Obj[OBJ_PUNCTURED_BOAT].loc = Obj[OBJ_INFLATED_BOAT].loc;
  Obj[OBJ_INFLATED_BOAT].loc = 0;

  if (YouAreInBoat) YouAreInBoat = 0;

  if (Room[Obj[OBJ_YOU].loc].prop & R_BODYOFWATER)
  {
    if (Obj[OBJ_YOU].loc == ROOM_RESERVOIR || Obj[OBJ_YOU].loc == ROOM_IN_STREAM)
      PrintLineIndex(620);
    else
      PrintLineIndex(621);
    YoureDead(); // ##### RIP #####
  }
}



void BuoyRoutine(void)
{
  if (BuoyFlag == 0 && Obj[OBJ_BUOY].loc == INSIDE + OBJ_YOU)
  {
    BuoyFlag = 1;
    PrintLineIndex(622);
  }
}



void DownstreamRoutine(void)
{
  int i;
  int  float_from[5] = {ROOM_RIVER_1, ROOM_RIVER_2, ROOM_RIVER_3, ROOM_RIVER_4, ROOM_RIVER_5};
  int    float_to[5] = {ROOM_RIVER_2, ROOM_RIVER_3, ROOM_RIVER_4, ROOM_RIVER_5, 0};
  int float_speed[5] = {4, 4, 3, 2, 1};

  for (i=0; i<5; i++)
    if (Obj[OBJ_YOU].loc == float_from[i]) break;
  if (i == 5) return;

  DownstreamCounter++;
  if (DownstreamCounter < float_speed[i]) return;

  if (float_to[i] == 0)
  {
    PrintLineIndex(623);
    YoureDead(); // ##### RIP #####
    return;
  }

  PrintLineIndex(624);

  DownstreamCounter = 0;
  BoatGoToRoutine(float_to[i]);
}



void BatRoomRoutine(void)
{
  if (Obj[OBJ_YOU].loc == ROOM_BAT_ROOM && IsObjVisible(OBJ_GARLIC) == 0)
  {
    int random_room[8] = {ROOM_MINE_1, ROOM_MINE_2, ROOM_MINE_3, ROOM_MINE_4, ROOM_LADDER_TOP,
                          ROOM_LADDER_BOTTOM, ROOM_SQUEEKY_ROOM, ROOM_MINE_ENTRANCE};

    PrintLineIndex(625);

    GoToRoutine(random_room[GetRandom(8)]);
  }
}



void LeavesTakenRoutine(void)
{
  if (Obj[OBJ_YOU].loc == ROOM_GRATING_CLEARING &&
      Obj[OBJ_LEAVES].loc != ROOM_GRATING_CLEARING &&
      GratingRevealed == 0)
  {
    GratingRevealed = 1;
    PrintLineIndex(626);
  }

  // also reveal grating just by being in grating room
  if (Obj[OBJ_YOU].loc == ROOM_GRATING_ROOM)
    GratingRevealed = 1;
}



// must call before match routine

void GasRoomRoutine(void)
{
  if (Obj[OBJ_YOU].loc == ROOM_GAS_ROOM)
  {
    int match   = (Obj[OBJ_MATCH  ].loc == INSIDE + OBJ_YOU && (Obj[OBJ_MATCH  ].prop & PROP_LIT));
    int candles = (Obj[OBJ_CANDLES].loc == INSIDE + OBJ_YOU && (Obj[OBJ_CANDLES].prop & PROP_LIT));
    int torch   = (Obj[OBJ_TORCH  ].loc == INSIDE + OBJ_YOU && (Obj[OBJ_TORCH  ].prop & PROP_LIT));
    int type = 0; // 1: lighted  2: carried

    if (match && MatchTurnsLeft == 2)
      type = 1;
    else if (match || candles || torch)
      type = 2;

    if (type)
    {
      if (type == 1)
        PrintLineIndex(627);
      else
        PrintLineIndex(628);

      PrintLineIndex(629);

      YoureDead(); // ##### RIP #####
    }
  }
}



void MatchRoutine(void)
{
  if (MatchTurnsLeft == 0) return;

  MatchTurnsLeft--;
  if (MatchTurnsLeft == 0)
  {
    int prev_darkness;

    if (IsObjVisible(OBJ_MATCH))
      PrintLineIndex(630);

    prev_darkness = IsPlayerInDarkness();
    Obj[OBJ_MATCH].prop &= ~PROP_LIT;
    if (IsPlayerInDarkness() != prev_darkness)
    {
      PrintNewLine();
      PrintPlayerRoomDesc(1);
    }
  }
}



void CeremonyBroken(void)
{
  if (Obj[OBJ_YOU].loc == ROOM_ENTRANCE_TO_HADES)
    PrintLineIndex(631);
}



void BellRungRoutine(void)
{
  if (BellRungCountdown == 0) return;
      BellRungCountdown--;
  if (BellRungCountdown == 0)
    CeremonyBroken();
}



void CandlesLitRoutine(void)
{
  if (CandlesLitCountdown == 0) return;
      CandlesLitCountdown--;
  if (CandlesLitCountdown == 0)
    CeremonyBroken();
}



void BellHotRoutine(void)
{
  if (BellHotCountdown == 0) return;
      BellHotCountdown--;
  if (BellHotCountdown == 0)
  {
    if (Obj[OBJ_YOU].loc == ROOM_ENTRANCE_TO_HADES)
      PrintLineIndex(632);

    Obj[OBJ_BELL].loc = ROOM_ENTRANCE_TO_HADES;
    Obj[OBJ_HOT_BELL].loc = 0;
  }
}



void HoldingGunkRoutine(void)
{
  if (Obj[OBJ_GUNK].loc == INSIDE + OBJ_YOU)
  {
    Obj[OBJ_GUNK].loc = 0;
    PrintLineIndex(633);
  }
}



void InRoomOnRainbowRoutine(void)
{
  if (Obj[OBJ_YOU].loc == ROOM_ON_RAINBOW)
    ExitFound = 1;
}



void DomeRoomRoutine(void)
{
  if (Obj[OBJ_YOU].loc == ROOM_DOME_ROOM && YouAreDead)
  {
    PrintLineIndex(634);

    GoToRoutine(ROOM_TORCH_ROOM);
  }
}



void UpATreeRoutine(void)
{
  int obj, other_fell = 0, count = 0;

  for (obj=2; obj<NUM_OBJECTS; obj++)
    if (Obj[obj].loc == ROOM_UP_A_TREE)
  {
    if (obj == OBJ_NEST)
    {
      if (Obj[obj].prop & PROP_MOVEDDESC)
      {
        count++;
        Obj[obj].loc = ROOM_PATH;
        if (Obj[OBJ_EGG].loc == INSIDE + OBJ_NEST)
        {
          other_fell = 1;
          Obj[OBJ_EGG].loc = 0;
          Obj[OBJ_BROKEN_EGG].loc = ROOM_PATH;
        }
      }
    }
    else if (obj == OBJ_EGG)
    {
      other_fell = 2;
      count++;
      Obj[OBJ_EGG].loc = 0;
      Obj[OBJ_BROKEN_EGG].loc = ROOM_PATH;
      Obj[OBJ_BROKEN_EGG].prop |= PROP_OPENABLE;
      Obj[OBJ_BROKEN_EGG].prop |= PROP_OPEN;
    }
    else
    {
      count++;
      Obj[obj].loc = ROOM_PATH;
    }
  }

  if (count == 1 && other_fell == 0)
    PrintLineIndex(635);
  else if (count > 1)
    PrintLineIndex(636);

  if (other_fell == 1)
    PrintLineIndex(637);
  else if (other_fell == 2)
    PrintLineIndex(638);
}



void SongbirdRoutine(void)
{
  if (AreYouInForest() && PercentChance(15, -1))
    PrintLineIndex(639);
}



void WaterSpilledRoutine(void)
{
  if (Obj[OBJ_WATER].loc == Obj[OBJ_YOU].loc)
  {
    Obj[OBJ_WATER].loc = 0;
    PrintLineIndex(640);
  }
}



void CyclopsRoomRoutine(void)
{
  if (Obj[OBJ_YOU].loc != ROOM_CYCLOPS_ROOM)
    {CyclopsCounter = 0; return;}

  if (CyclopsState >= 3 ||                  // asleep or fled
      VillainAttacking[VILLAIN_CYCLOPS] ||  // attacking
      Obj[OBJ_CYCLOPS].loc == 0)            // dead
    return;

  CyclopsCounter++;

  if (CyclopsState >= 1) // hungry or thirsty
  {
    switch (CyclopsCounter - 1)
    {
      case 0: PrintLineIndex(641); break;
      case 1: PrintLineIndex(642); break;
      case 2: PrintLineIndex(643); break;
      case 3: PrintLineIndex(644); break;
      case 4: PrintLineIndex(645); break;
      case 5: PrintLineIndex(646); break;
      case 6: PrintLineIndex(647); break;
    }

    if (CyclopsCounter == 7)
      YoureDead(); // ##### RIP #####
  }
  else if (CyclopsCounter == 5)
  {
    CyclopsCounter = 0;
    CyclopsState = 1; // hungry
  }
}

//-----------------------------------------------------------------------------

void ScoreObj(int obj, unsigned char takeval, unsigned char caseval)
{
  if (Obj[obj].loc == INSIDE + OBJ_YOU && (Obj[obj].prop & PROP_SCOREDTAKE) == 0)
  {
    Obj[obj].prop |= PROP_SCOREDTAKE;
    Score += takeval;
  }
  else if (Obj[obj].loc == INSIDE + OBJ_TROPHY_CASE && (Obj[obj].prop & PROP_SCOREDCASE) == 0)
  {
    Obj[obj].prop |= PROP_SCOREDCASE;
    Score += caseval;
  }
}



void ScoreRoom(int room, unsigned char visitval)
{
  if (Obj[OBJ_YOU].loc == room && (Room[room].prop & R_SCOREDVISIT) == 0)
  {
    Room[room].prop |= R_SCOREDVISIT;
    Score += visitval;
  }
}



void ScoreUpdateRoutine(void)
{
  int old_score = Score;

  ScoreObj(OBJ_EGG          ,  5,  5);
  ScoreObj(OBJ_CANARY       ,  6,  4);
  ScoreObj(OBJ_PAINTING     ,  4,  6);
  ScoreObj(OBJ_BAR          , 10,  5);
  ScoreObj(OBJ_TORCH        , 14,  6);
  ScoreObj(OBJ_COFFIN       , 10, 15);
  ScoreObj(OBJ_SCEPTRE      ,  4,  6);
  ScoreObj(OBJ_TRUNK        , 15,  5);
  ScoreObj(OBJ_TRIDENT      ,  4, 11);
  ScoreObj(OBJ_JADE         ,  5,  5);
  ScoreObj(OBJ_BRACELET     ,  5,  5);
  ScoreObj(OBJ_DIAMOND      , 10, 10);
  ScoreObj(OBJ_BAG_OF_COINS , 10,  5);
  ScoreObj(OBJ_SKULL        , 10, 10);
  ScoreObj(OBJ_SCARAB       ,  5,  5);
  ScoreObj(OBJ_EMERALD      ,  5, 10);
  ScoreObj(OBJ_CHALICE      , 10,  5);
  ScoreObj(OBJ_POT_OF_GOLD  , 10, 10);
  ScoreObj(OBJ_BAUBLE       ,  1,  1);

  ScoreRoom(ROOM_KITCHEN       , 10);
  ScoreRoom(ROOM_CELLAR        , 25);
  ScoreRoom(ROOM_EW_PASSAGE    ,  5);
  ScoreRoom(ROOM_LOWER_SHAFT   , 13);
  ScoreRoom(ROOM_TREASURE_ROOM , 25);

  if (Score - old_score > 0)
  {
    // PrintTextIndex(648);
    // PrintInteger(Score - old_score);
    // if (Score - old_score == 1) PrintLineIndex(649);
    // else                        PrintLineIndex(650);
  }

  if (Score == SCORE_MAX && WonGame == 0)
  {
    WonGame = 1;
    Obj[OBJ_MAP].prop &= ~PROP_INVISIBLE;
    Room[ROOM_WEST_OF_HOUSE].prop &= ~R_DESCRIBED;
    PrintLineIndex(651);
  }
}

//-----------------------------------------------------------------------------

//run event routines after each action that set time-passed flag
void RunEventRoutines(void)
{
  SwordRoutine();
  LampDrainRoutine();
  CandlesShrinkRoutine();
  ReservoirFillRoutine();
  ReservoirDrainRoutine();
  SinkingObjectsRoutine(); // must be called after reservoir fill/drain routines
  LoudRoomRoutine();
  MaintenanceLeakRoutine();
  BoatPuncturedRoutine();
  BuoyRoutine(); // should be called before downstream routine because of message order
  DownstreamRoutine();
  BatRoomRoutine();
  LeavesTakenRoutine();
  GasRoomRoutine(); // must be called before match routine
  MatchRoutine();
  BellRungRoutine();
  CandlesLitRoutine();
  BellHotRoutine();
  HoldingGunkRoutine();
  InRoomOnRainbowRoutine();
  DomeRoomRoutine();
  UpATreeRoutine();
  SongbirdRoutine();
  WaterSpilledRoutine();
  CyclopsRoomRoutine();
  ScoreUpdateRoutine();

  VillainsRoutine();
}
//*****************************************************************************



//*****************************************************************************
int GetScore(void)
{
  return Score;
}



int GetMaxScore(void)
{
  return SCORE_MAX;
}



void PrintRankName(void)
{
       if (Score == 350) PrintLineIndex(652);
  else if (Score >  330) PrintLineIndex(653);
  else if (Score >  300) PrintLineIndex(654);
  else if (Score >  200) PrintLineIndex(655);
  else if (Score >  100) PrintLineIndex(656);
  else if (Score >   50) PrintLineIndex(657);
  else if (Score >   25) PrintLineIndex(658);
  else                   PrintLineIndex(659);
}

//*****************************************************************************



//*****************************************************************************

void ZeroMem(void *p, int n)
{
  memset(p, 0, n);
}



void InitGameState(void)
{
  int i;

  ZeroMem(&GameFlags, sizeof(GameFlags));
  ZeroMem(&GameVars , sizeof(GameVars ));

  LampTurnsLeft   = 200;
  CandleTurnsLeft = 40;
  MatchesLeft     = 6;
  LoadAllowed     = 100;

  VillainStrength[VILLAIN_TROLL  ] = 2;
  VillainStrength[VILLAIN_THIEF  ] = 5;
  VillainStrength[VILLAIN_CYCLOPS] = 10000;

  InitRoomProperties();

  InitObjectLocations();
  InitObjectProperties();

  for (i=0; i<NUM_OBJECTS; i++)
  {
    Obj[i].order = i;
    Obj[i].thiefvalue = 0;
  }

  Obj[OBJ_SKULL          ].thiefvalue = 10;
  Obj[OBJ_SCEPTRE        ].thiefvalue =  6;
  Obj[OBJ_CHALICE        ].thiefvalue =  5;
  Obj[OBJ_TRIDENT        ].thiefvalue = 11;
  Obj[OBJ_COFFIN         ].thiefvalue = 15;
  Obj[OBJ_DIAMOND        ].thiefvalue = 10;
  Obj[OBJ_JADE           ].thiefvalue =  5;
  Obj[OBJ_BAG_OF_COINS   ].thiefvalue =  5;
  Obj[OBJ_EMERALD        ].thiefvalue = 10;
  Obj[OBJ_PAINTING       ].thiefvalue =  6;
  Obj[OBJ_BAR            ].thiefvalue =  5;
  Obj[OBJ_POT_OF_GOLD    ].thiefvalue = 10;
  Obj[OBJ_BRACELET       ].thiefvalue =  5;
  Obj[OBJ_SCARAB         ].thiefvalue =  5;
  Obj[OBJ_TORCH          ].thiefvalue =  6;
  Obj[OBJ_TRUNK          ].thiefvalue =  5;
  Obj[OBJ_EGG            ].thiefvalue =  5;
  Obj[OBJ_BROKEN_EGG     ].thiefvalue =  2;
  Obj[OBJ_BAUBLE         ].thiefvalue =  1;
  Obj[OBJ_CANARY         ].thiefvalue =  4;
  Obj[OBJ_BROKEN_CANARY  ].thiefvalue =  1;

  ItObj = OBJ_MAILBOX;
}
//*****************************************************************************
