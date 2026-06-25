// Zork I: The Great Underground Empire
// (c) 1980 by INFOCOM, Inc.
// C port and parser (c) 2021 by Donnie Russell II

// This source code is provided for personal, educational use only.
// You are welcome to use this source code to develop your own works,
// but the story-related content belongs to the original authors of Zork.



#include "def.h"
#include "_enum.h"



int CurWord;

int NumStrWords;
unsigned int StrWord[80];

int NumOopsWords;
unsigned int OopsWord[80];

int NumAgainWords;
unsigned int AgainWord[80];

unsigned char AgainMode;

char *UnknownWordP;
int UnknownWordSize;

enum
{
  V_BRIEF,
  V_VERBOSE,
  V_SUPERBRIEF
};

int Verbosity = V_BRIEF;

int ItObj, PrevItObj; //the obj "it" refers to

unsigned char TimePassed; //flag: time passed during action
unsigned char GameOver; //flag, but with special value 2: restart was requested



extern struct GAMEVARS GameVars;
extern struct ROOM_STRUCT Room[NUM_ROOMS];
extern struct OBJ_STRUCT Obj[NUM_OBJECTS];

extern int NumPrintedLines;



//*****************************************************************************

void PrintBlockMsg(int newroom)
{
  if (newroom >= BL0 && newroom <= 255)
    PrintBlockMsgIndex(newroom - BL0, 0);
}



void PrintObjectDesc(int obj, int desc_flag)
{
  int16_t segment = 0;

  if (OverrideObjectDesc(obj, desc_flag))
    {}
  else
  {
    if (desc_flag)
    {
      if (Obj[obj].prop & PROP_MOVEDDESC)
        segment = 2;
      else
        segment = 1;
    }

    PrintObjectDescIndex(obj, segment);
  }
}



void PrintScore(void)
{
  if (GameOver)
    PrintTextIndex(660);
  else
    PrintTextIndex(661);

  PrintInteger(GetScore());
  PrintTextIndex(662);
  PrintInteger(GetMaxScore());
  PrintTextIndex(663);
  PrintInteger(NumMoves);

  if (NumMoves == 1)
    PrintLineIndex(510);
  else
    PrintLineIndex(511);

  if (GameOver)
  {
    PrintTextIndex(664);
    PrintRankName();
  }
}

//*****************************************************************************



//*****************************************************************************

// convert "and"s to "then"s if they precede a verb that is not also a noun
// for example, "pump" is both so if preceded by "and", don't convert



int IsVerbPhrase(int i)
{
  int j, w1, w2;

  for (j=0; ; j++)
  {
    if (GetVerbToAction_Action(j) == 0) break;
    GetVerbToAction_Words(j, &w1, &w2);
    if (StrWord[i] == w1)
      if (w2 == 0 || (i+1 < NumStrWords && StrWord[i+1] == w2))
        return 1;
  }

  return 0;
}



int IsNounPhrase(int i)
{
  int j, w1, w2, w3;

  for (j=0; ; j++)
  {
    if (GetNounPhraseToObj_Obj(j) == 0) break;
    GetNounPhraseToObj_Words(j, &w1, &w2, &w3);
    if (StrWord[i] == w1)
      if (w2 == 0 || (i+1 < NumStrWords && StrWord[i+1] == w2))
        if (w3 == 0 || (i+2 < NumStrWords && StrWord[i+2] == w3))
          return 1;
  }

  for (j=0; ; j++)
  {
    if (GetNounPhraseToFixedObj_FObj(j) == 0) break;
    GetNounPhraseToFixedObj_Words(j, &w1, &w2);
    if (StrWord[i] == w1)
      if (w2 == 0 || (i+1 < NumStrWords && StrWord[i+1] == w2))
        return 1;
  }

  return 0;
}



void ConvertAndsToThens(void)
{
  int i;

  for (i=0; i<NumStrWords; i++)
    if (StrWord[i] == WORD_AND)
  {
    if (i+1 == NumStrWords || StrWord[i+1] == WORD_THEN ||
        (IsVerbPhrase(i+1) && IsNounPhrase(i+1) == 0))
      StrWord[i] = WORD_THEN;
  }
}

//*****************************************************************************



//*****************************************************************************

#define MAX_DIGITS  4  // maximum number of digits extracted from numeric word



// get input from player
// assumes str buffer size is 80

void GetWordsRoutine(void)
{
  char *str, *p, *wordstart;
  int c, digits;
  unsigned int w;

#ifndef NO_STATUS_LINE
  PrintScoreAndMovesToStatusLine();
  PrintStatusLine();
#endif

  str = GetString();

  // convert upper case chars to lower case, replace whitespace chars with null char, replace ! and ? with .
  for (p=str; p<str+80; p++)
  {
    c = *p;

    if (c >= 'A' && c <= 'Z') *p = c - 'A' + 'a';
    else if (c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r') *p = 0;
    else if (c == '!' || c == '?') *p = '.';
  }

  NumStrWords = 0;
  CurWord = 0;

  // assign number to each word in input
  for (p=str; p<str+80; )
  {
    if (*p == 0) p++;
    else if (*p == ',') {p++; StrWord[NumStrWords++] = WORD_AND;}
    else if (*p == '.') {p++; StrWord[NumStrWords++] = WORD_THEN;}
    else if (*p >= '0' && *p <= '9')
    {
      w = 0;
      digits = 0;

      while (p < str+80 && *p != 0 && *p != ',' && *p != '.')
      {
        if (*p >= '0' && *p <= '9')
        {
          if (digits < MAX_DIGITS)
          {
            w = w * 10 + (*p - '0');
            digits++;
          }
        }
        else
          digits = MAX_DIGITS;
        p++;
      }
      StrWord[NumStrWords++] = NUMERIC + w;
    }
    else
    {
      wordstart = p;
      w = 0;

      ReadWordList(0); // go to start of word list
      for (;;)
      {
        if (p == wordstart && ReadWordList(1) == 0)
        {
          StrWord[NumStrWords++] = 0;
          UnknownWordP = p;
          UnknownWordSize = 0;
          while (p < str+80 && *p != 0 && *p != ',' && *p != '.') {p++; UnknownWordSize++;}
          break;
        }

        if (p >= str+80 || *p == ',' || *p == '.') c = 0;
        else c = *p;

        if (c != ReadWordList(1)) // char didn't match
        {
          while (ReadWordList(1)) ReadWordList(2); // skip remaining chars of word in wordlist
          ReadWordList(2); // skip null char
          p = wordstart;
          w++;
        }
        else if (c == 0) {StrWord[NumStrWords++] = w; break;} // word matched; don't increment p here
        else {p++; ReadWordList(2);}
      }
    }
  }
}



void GetWords(unsigned char prompt)
{
  char *p;
  int i, j, n;
  unsigned int w;

  for (;;)
  {
    if (prompt)
    {
      if (Verbosity != V_SUPERBRIEF) PrintNewLine();
      PrintChar('>');
    }

    GetWordsRoutine();

    if (prompt == 0) break;

    ConvertAndsToThens();

    for (i=0; i<NumStrWords; i++)
      if (StrWord[i] != WORD_AND && StrWord[i] != WORD_THEN) break;
    if (NumStrWords == 0 || i == NumStrWords)
    {
      NumOopsWords = 0;
      PrintLineIndex(665);
      continue;
    }

    n = 0;
    for (i=0; i<NumStrWords; i++)
      if (StrWord[i] == 0) n++;
    if (n > 0)
    {
      if (n > 1) PrintLineIndex(666);
      else
      {
        PrintTextIndex(667);
        for (p=UnknownWordP; p<UnknownWordP+UnknownWordSize; p++) PrintChar(*p);
        PrintLineIndex(668);
      }
      NumOopsWords = NumStrWords;
      for (i=0; i<NumOopsWords; i++) OopsWord[i] = StrWord[i];
      continue;
    }

    w = 0;
    if (NumStrWords > 0 && StrWord[0] == WORD_OOPS)
    {
      if (NumStrWords == 1) {PrintLineIndex(669); continue;}
      if (NumStrWords > 2) {PrintLineIndex(670); continue;}
      w = StrWord[1];
    }

    if (w)
    {
      NumStrWords = NumOopsWords;
      for (i=0; i<NumStrWords; i++) StrWord[i] = OopsWord[i];
      NumOopsWords = 0;
      if (NumStrWords > 0 && StrWord[0] == WORD_OOPS) {PrintLineIndex(671); continue;}

      n = 0;
      for (i=0; i<NumStrWords; i++)
        if (StrWord[i] == 0) {j = i; n++;}
      if (n == 0) {PrintLineIndex(672); continue;}
      if (n > 1) {PrintLineIndex(673); continue;}

      StrWord[j] = w;
    }

    break;
  }
}



unsigned char MatchCurWordIndex(int i)
{
  if (CurWord < NumStrWords && i == (AgainMode ? AgainWord[CurWord] : StrWord[CurWord]))
  {
    CurWord++;
    return 1;
  }
  return 0;
}



void PrintWord(unsigned int w, int capital_flag)
{
  if (w >= NUMERIC)
  {
    PrintInteger(w - NUMERIC);
    return;
  }

  ReadWordList(0); // go to start of word list
  while (w)
  {
    if (ReadWordList(1) == 0) w--;
    ReadWordList(2);
  }

  for (;;)
  {
    unsigned char c = ReadWordList(1);

    if (c == 0) break;
    if (capital_flag && c >= 'a' && c <= 'z')
    {
      c = c - 'a' + 'A';
      capital_flag = 0;
    }
    PrintChar(c);
    ReadWordList(2);
  }
}

//*****************************************************************************



//*****************************************************************************

// function can call itself

int IsObjVisible(int obj)
{
  if (obj < 2 || obj >= NUM_OBJECTS) return 0;

  if (Obj[obj].prop & PROP_EVERYWHERE) return 1; // presence must be checked by calling function

  if (Obj[obj].prop & PROP_INVISIBLE) return 0;

  if (Obj[obj].loc == INSIDE + OBJ_YOU) return 1; // obj is in your inventory
  if (Obj[obj].loc == Obj[OBJ_YOU].loc) return 1; // obj is in same room as you

  if (Obj[obj].loc >= INSIDE) // obj is inside container
  {
    int container = Obj[obj].loc - INSIDE;

    if (Obj[container].prop & PROP_OPEN) // container is open
      if (IsObjVisible(container)) return 1; // yikes it's recursive
  }

  return 0;
}



// assumes player is not an actor

int IsObjCarriedByActor(int obj)
{
  int container = Obj[obj].loc - INSIDE; // get obj's container, if any

  if (container > 1 && container < NUM_OBJECTS)
    if (Obj[container].prop & PROP_ACTOR)
      return 1;

  return 0;
}



// a lighted object carried by an actor will light the room if the actor is "open";
// therefore exclude such objects

int IsPlayerInDarkness(void)
{
  int i, obj;

  if (Room[Obj[OBJ_YOU].loc].prop & R_LIT) return 0; // room is lit

  if (Obj[OBJ_YOU].prop & PROP_LIT) return 0; // you are lit

  for (i=2; i<NUM_OBJECTS; i++)
  {
    obj = Obj[i].order;

    if (IsObjVisible(obj) &&              
        IsObjCarriedByActor(obj) == 0 &&  
        (Obj[obj].prop & PROP_LIT))       
      return 0; // obj is visible, not carried by actor, and lit
  }

  return 1;
}



//move order of "obj" to last in printing order
void MoveObjOrderToLast(int obj)
{
  int i, j;

  for (i=2; i<NUM_OBJECTS; i++)
    if (obj == Obj[i].order)
  {
    for (j=i; j<NUM_OBJECTS-1; j++)
      Obj[j].order = Obj[j+1].order;
    Obj[j].order = obj;
    break;
  }
}



//returns number of objects in location
//location can be a room, player's inventory, or inside object
int GetNumObjectsInLocation(int loc)
{
  int count, i, obj;

  count = 0;
  for (i=2; i<NUM_OBJECTS; i++)
  {
    obj = Obj[i].order;
    if (Obj[obj].loc == loc) count++;
  }
  return count;
}



//returns total size of objects in location
//location can be a room, player's inventory, or inside object
int GetTotalSizeInLocation(int loc)
{
  int size, i, obj;

  size = 0;
  for (i=2; i<NUM_OBJECTS; i++)
  {
    obj = Obj[i].order;
    if (Obj[obj].loc == loc) size += GetObjectSize(obj);
  }
  return size;
}



// only display objects inside something else one level deep

void PrintContents(int obj, const char *heading, int print_empty)
{
  int flag = 0, i;

  for (i=2; i<NUM_OBJECTS; i++)
  {
    int obj_inside = Obj[i].order;

    if ( Obj[obj_inside].loc == INSIDE + obj &&
         (Obj[obj_inside].prop & PROP_NODESC) == 0 &&
         (Obj[obj_inside].prop & PROP_INVISIBLE) == 0 &&
         !( (Obj[obj_inside].prop & PROP_INSIDEDESC) &&
            (Obj[obj_inside].prop & PROP_MOVEDDESC) == 0 ))
    {
      if (flag == 0) {PrintLine(heading); flag = 1;}
      PrintChar(' '); PrintChar(' ');
      PrintObjectDesc(obj_inside, 0);
      PrintNewLine();
    }
  }

  for (i=2; i<NUM_OBJECTS; i++)
  {
    int obj_inside = Obj[i].order;

    if ( Obj[obj_inside].loc == INSIDE + obj &&
         (Obj[obj_inside].prop & PROP_NODESC) == 0 &&
         (Obj[obj_inside].prop & PROP_INVISIBLE) == 0 &&
         ( (Obj[obj_inside].prop & PROP_INSIDEDESC) &&
           (Obj[obj_inside].prop & PROP_MOVEDDESC) == 0 ))
    {
      flag = 1;
      PrintObjectDesc(obj_inside, 1);
    }
  }

  if (print_empty && flag == 0)
    PrintLineIndex(674);
}



// print objects at "location"; location can be player's inventory

void PrintPresentObjects(int location, const char *heading, int list_flag)
{
  int flag, i, obj;

  flag = 0;
  for (i=2; i<NUM_OBJECTS; i++)
  {
    obj = Obj[i].order;

    if (Obj[obj].loc == location &&
        (Obj[obj].prop & PROP_NODESC) == 0 &&
        (Obj[obj].prop & PROP_INVISIBLE) == 0)
    {
      if (flag == 0)
      {
        flag = 1;

        if (location == INSIDE + OBJ_YOU)
          PrintLineIndex(675);
        else if (heading != 0)
          PrintLine(heading);
      }

      if (location == INSIDE + OBJ_YOU || list_flag)
      {
        PrintObjectDesc(obj, 0);
        PrintNewLine();
      }
      else
        PrintObjectDesc(obj, 1);

      if ((Obj[obj].prop & PROP_OPEN) &&
          (Obj[obj].prop & PROP_ACTOR) == 0)
        PrintContents(obj, "  (which contains)", 0); // 0: don't print "It's empty."
    }
  }

  if (location == INSIDE + OBJ_YOU && flag == 0) PrintLineIndex(676);
}
//*****************************************************************************



//*****************************************************************************
void PrintRoomDesc(int room, int force_description)
{
#ifndef NO_STATUS_LINE
  PrintRoomDescIndexToStatusLine(room);
#endif

  PrintRoomDescIndex(room, 0);

  if (force_description || Verbosity != V_SUPERBRIEF)
  {
    if ((Room[room].prop & R_ALWAYSDESC) || force_description || Verbosity == V_VERBOSE)
      Room[room].prop &= ~R_DESCRIBED;

    if (OverrideRoomDesc(room))
      {}
    else if ((Room[room].prop & R_DESCRIBED) == 0)
      PrintRoomDescIndex(room, 1);
  }

  // game logic depends on this being set even if player never actually sees full description
  Room[room].prop |= R_DESCRIBED;
}



void PrintPlayerRoomDesc(int force_description)
{
  if (IsPlayerInDarkness())
    PrintLineIndex(677);
  else
  {
    PrintRoomDesc(Obj[OBJ_YOU].loc, force_description);
    if (force_description || Verbosity != V_SUPERBRIEF) PrintPresentObjects(Obj[OBJ_YOU].loc, 0, 0);
  }
}
//*****************************************************************************



//*****************************************************************************

int ReadWriteSaveSlot(char *filename, int mode, int slot);

#ifdef __AVR_ATmega2560__

#define LAST_SAVE_SLOT  9

int CheckSaveSignature(int slot);

#else

#define LAST_SAVE_SLOT  99

#endif



// mode: 0 load  1 save
// returns 1 if okay to load/save
int GetLoadSaveFilename(char *filename, int mode, int *slot)
{
  int exists = 0;

  if (mode == 0)
    PrintTextIndex(678);
  else
    PrintTextIndex(679);
  PrintTextIndex(680);
  PrintInteger(LAST_SAVE_SLOT);
  PrintLineIndex(681);

  GetWords(0);
  if (NumStrWords == 0)
    *slot = 0;
  else
  {
    *slot = StrWord[0] - NUMERIC;
    if (*slot < 0 || *slot > LAST_SAVE_SLOT)
    {
      if (mode == 0)
        PrintLineIndex(682);
      else
        PrintLineIndex(683);
      return 0;
    }
  }

#ifdef __AVR_ATmega2560__
  exists = (CheckSaveSignature(*slot) != 0);
#elif defined(COMPILE_SATURN)
  (void)filename;
  exists = 0; // no file system for saves on Saturn
#else
  {
    int len = 0;
    FILE *f;

    memcpy(filename+len, "game", 4); len += 4;
    len += PrintIntToBuffer(*slot, filename+len);
    memcpy(filename+len, ".sav", 4); len += 4;
    filename[len] = 0;

    f = fopen(filename, "rb");
    if (f) {fclose(f); exists = 1;}
  }
#endif

  if (exists)
  {
    if (mode == 1)
      for (;;)
    {
      PrintLineIndex(684);
      GetWords(0);
      if (MatchCurWordIndex(WORD_Y) || MatchCurWordIndex(WORD_YES))
        break;
      if (MatchCurWordIndex(WORD_N) || MatchCurWordIndex(WORD_NO))
      {
        PrintLineIndex(683);
        return 0;
      }
      PrintLineIndex(685);
    }
  }
  else if (mode == 0)
  {
    PrintLineIndex(686);
    return 0;
  }

  return 1;
}



// returns 1 if successful
int DoSave(void)
{
#ifdef COMPILE_SATURN
  PrintLineIndex(687);
  return 0;
#else
  char filename[80];
  int slot, error = 1;

  if (GetLoadSaveFilename(filename, 1, &slot) == 0)
    return 0;
  error = ReadWriteSaveSlot(filename, 1, slot);

  if (error)
    PrintLineIndex(688);
  else
    PrintLineIndex(689);

  return (error == 0);
#endif
}



// returns 1 if successful
int DoRestore(void)
{
#ifdef COMPILE_SATURN
  PrintLineIndex(690);
  return 0;
#else
  char filename[80];
  int slot, error = 1;

  if (GetLoadSaveFilename(filename, 0, &slot) == 0)
    return 0;
  error = ReadWriteSaveSlot(filename, 0, slot);

  if (error)
    PrintLineIndex(691);
  else
  {
    PrintLineIndex(692);
    if (Verbosity != V_SUPERBRIEF)
      PrintNewLine();
    NumStrWords = 0;
    NumOopsWords = 0;
    NumAgainWords = 0;
    ItObj = 0;
    GameOver = 0;
    PrintPlayerRoomDesc(1); // force description
  }

  return (error == 0);
#endif
}

//*****************************************************************************



//*****************************************************************************

// Parse Routines



// matches a maximum of two words

int GetActionFromInput(void)
{
  int i, action, w1, w2, temp;

  for (i=0; ; i++)
  {
    action = GetVerbToAction_Action(i);
    if (action == 0) break; // reached end of list without finding anything

    GetVerbToAction_Words(i, &w1, &w2);

    temp = CurWord;
    if (MatchCurWordIndex(w1))
      if (w2 && MatchCurWordIndex(w2) == 0)
        CurWord = temp;
    if (CurWord > temp) break; // found action
  }
  return action;
}



// matches a maximum of three words; longest matched phrase size needed
// if there are multiple visible match objects, returns -1: player must be more specific

int GetObjFromInput(void)
{
  int i, obj, temp, size, w1, w2, w3, fobj = 0, fsize = 0;
  int vobj[3] = {0, 0, 0};

  // look through noun phrase table
  for (i=0; ; i++)
  {
    obj = GetNounPhraseToObj_Obj(i);
    if (obj == 0) break; // reached end

    temp = CurWord;
    size = 1;
    GetNounPhraseToObj_Words(i, &w1, &w2, &w3);
    MatchCurWordIndex(w1);
    if (w2) 
    {
      size++;
      MatchCurWordIndex(w2);
      if (w3) 
      {
        size++;
        MatchCurWordIndex(w3);
      }
    }
    if (CurWord - temp != size) size = 0;
    CurWord = temp;

    if (size)
    {
      if (size > fsize)
      {
        fobj = obj;
        fsize = size;
      }

      if (IsObjVisible(obj))
      {
        if (vobj[size-1] == 0)
          vobj[size-1] = obj;
        else if (vobj[size-1] != obj)
          vobj[size-1] = -1;
      }
    }
  }

  for (size=3; size>=1; size--)
  {
    if (vobj[size-1] == -1)
      return -1;
    if (vobj[size-1])
    {
      CurWord += size;
      return vobj[size-1];
    }
  }
  CurWord += fsize;
  return fobj; // can return 0
}



int GetFixedObjFromInput(int room)
{
  int temp, i, fobj, w1, w2, fobj_inroom = 0, cw_inroom, fobj_notinroom = 0, cw_notinroom;

  temp = CurWord; // keep track of word position because we will be matching multiple noun phrases

  for (i=0; ; i++)
  {
    fobj = GetNounPhraseToFixedObj_FObj(i);
    if (fobj == 0) break; // reached end of list

    CurWord = temp;
    GetNounPhraseToFixedObj_Words(i, &w1, &w2);
    if (MatchCurWordIndex(w1))
      if (w2 && MatchCurWordIndex(w2) == 0)
        CurWord = temp;
    if (CurWord > temp) // found noun phrase
    {
      if (GetNounPhraseToFixedObj_Room(i) == room) // found a fixed obj in room
      {
        if (fobj_inroom == 0) // only use first one found, though there should be only one anyway
        {
          fobj_inroom = fobj;
          cw_inroom = CurWord; // keep track of word position past this match
        }
      }
      else // found a fixed obj not in room
      {
        if (fobj_notinroom == 0) // only use first one found, though there should be only one anyway
        {
          fobj_notinroom = fobj;
          cw_notinroom = CurWord; // keep track of word position past this match
        }
      }
    }
  }

  // fixed object in room; return this first
  if (fobj_inroom)
  {
    CurWord = cw_inroom;
    return fobj_inroom;
  }

  // fixed object not in room
  if (fobj_notinroom)
  {
    CurWord = cw_notinroom;
    return FOBJ_NOTVIS;
  }

  return 0;
}



// gets obj, fixed obj, or scenery obj, whichever comes first

// returns -1 if player needs to be more specific
//         -2 if player used "it" but it wasn't clear what itobj is

// itobj starts at 0
// if itobj already refers to an object, any additional object invalidates itobj (sets to -1)

int GetAllObjFromInput(int room)
{
  int obj;

  if (MatchCurWordIndex(WORD_IT) || MatchCurWordIndex(WORD_THEM))
  {
    // itobj from previous sentence
    if (PrevItObj <= 0)
    {
      PrintLineIndex(693);
      return -2;
    }
    ItObj = PrevItObj;
    return ItObj;
  }

  // skip article (if any) immediately before object
  if (MatchCurWordIndex(WORD_THE) || MatchCurWordIndex(WORD_A) || MatchCurWordIndex(WORD_AN)) {}

  // convert noun phrase to obj
  obj = GetObjFromInput(); // can return -1 if player needs to be more specific
  if (obj == 0) obj = GetFixedObjFromInput(room);

  if (obj == 0)
  {
    ItObj = -1;
    PrintLineIndex(694);
  }
  else if (obj == -1)
  {
    ItObj = -1;
    PrintLineIndex(695);
  }
  else
  {
    if (ItObj == 0) ItObj = obj; // first object encountered this sentence; set itobj
    else ItObj = -1; // another obj encountered; invalidate itobj
  }

  return obj;
}



// same as GetAllObjFromInput() above but just skips noun phrase without doing or printing anything
void SkipObjFromInput(int room)
{
  int obj;

  if (MatchCurWordIndex(WORD_IT) || MatchCurWordIndex(WORD_THEM)) return;

  if (MatchCurWordIndex(WORD_THE) || MatchCurWordIndex(WORD_A) || MatchCurWordIndex(WORD_AN)) {}

  obj = GetObjFromInput();
  if (obj == 0) obj = GetFixedObjFromInput(room);
}
//*****************************************************************************



//*****************************************************************************
//set TimePassed to 1 when an action completes successfully



void ParseActionDirection(int action)
{
  int newroom, prev_darkness;

  newroom = GetRoomPassage(Obj[OBJ_YOU].loc, action - A_NORTH);
  if (newroom >= BL0)
  {
    PrintBlockMsg(newroom);
    return;
  }

  // handle things like water and boats in game-specific code
  if (ActionDirectionRoutine(newroom)) return; 

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
      return;
    }
    else PrintLineIndex(8);
  }

  PrintPlayerRoomDesc(0);
}



//returns 1 to signal to calling routine that command was processed

int TakeOutOfRoutine(int *container)
{
  MatchCurWordIndex(WORD_OF);

  *container = GetAllObjFromInput(Obj[OBJ_YOU].loc); if ((*container) <= 0) return 1;

  if ((*container) == FOBJ_SCENERY_NOTVIS || (*container) == FOBJ_NOTVIS)
  {
    PrintLineIndex(138);
    return 1;
  }

  if ((*container) == OBJ_YOU || (*container) >= NUM_OBJECTS)
  {
    PrintLineIndex(696);
    return 1;
  }

  if (IsObjVisible(*container) == 0)
  {
    PrintObjectDesc(*container, 0);
    PrintLineIndex(697);
    return 1;
  }

  if (InterceptTakeOutOf(*container)) return 1;

  if ((Obj[*container].prop & PROP_OPEN) == 0 ||
      (Obj[*container].prop & PROP_ACTOR))
  {
    PrintObjectDesc(*container, 0);
    PrintChar(':'); PrintChar(' ');

    if (Obj[*container].prop & PROP_OPENABLE)
    {
      PrintLineIndex(698);
      ItObj = *container;
    }
    else
      PrintLineIndex(699);

    return 1;
  }

  return 0;
}



//returns 1 to signal to calling routine that command was processed
//container may be 0 (none specified)

int VerifyTakeableObj(int obj, int container, int num_takes)
{
  if (obj == FOBJ_SCENERY_NOTVIS || obj == FOBJ_NOTVIS)
  {
    if (num_takes > 1)
      PrintLineIndex(138);
    else
      PrintLineIndex(293);

    return 1;
  }

  if (obj == FOBJ_AMB)
  {
    if (num_takes > 1)
      PrintLineIndex(700);
    else
      PrintLineIndex(477);

    return 1;
  }

  if (InterceptTakeFixedObj(obj)) return 1;

  if (obj == OBJ_YOU || obj >= NUM_OBJECTS)
  {
    if (num_takes > 1)
      PrintLineIndex(701);
    else
      PrintLineIndex(702);

    return 1;
  }

  if (GetPlayersVehicle() == obj)
  {
    PrintObjectDesc(obj, 0);
    PrintLineIndex(703);
    return 1;
  }

  if (container != 0 &&
      Obj[obj].loc != INSIDE + container &&
      container != GetPlayersVehicle())
  {
    PrintObjectDesc(obj, 0);

    if (Obj[container].prop & PROP_SURFACE)
      PrintLineIndex(704);
    else
      PrintLineIndex(705);

    return 1;
  }

  return 0;
}



// returns 1 if take intercepted or failed

int TakeRoutine(int obj, const char *msg)
{
  int num, chance;

  if (InterceptTakeObj(obj)) return 1;

  if (Obj[obj].prop & PROP_NOTTAKEABLE)
    {PrintLineIndex(706); return 1;}


  if (GetTotalSizeInLocation(INSIDE + OBJ_YOU) + GetObjectSize(obj) > LoadAllowed)
  {
    PrintTextIndex(707);
    if (LoadAllowed < LOAD_MAX)
      PrintLineIndex(708);
    else
      PrintLineIndex(56);
    return 1;
  }

  num = GetNumObjectsInLocation(INSIDE + OBJ_YOU);
  chance = INV_LIMIT_CHANCE * num; if (chance > 100) chance = 100;
  if (num > MAX_INVENTORY_ITEMS && PercentChance(chance, -1))
  {
    PrintLineIndex(709);
    return 1;
  }


  TimePassed = 1;
  PrintLine(msg);

  Obj[obj].loc = INSIDE + OBJ_YOU;
  Obj[obj].prop |= PROP_MOVEDDESC;

  MoveObjOrderToLast(obj);

  return 0;
}



int VerifyTakeAllExceptObj(int obj)
{
  if (obj == FOBJ_SCENERY_NOTVIS || obj == FOBJ_NOTVIS)
  {
    PrintLineIndex(138);
    return 1;
  }
  else if (obj == FOBJ_AMB)
  {
    PrintLineIndex(700);
    return 1;
  }
  else if (obj == OBJ_YOU || obj >= NUM_OBJECTS)
  {
    PrintLineIndex(701);
    return 1;
  }
  else if (Obj[obj].loc == INSIDE + OBJ_YOU)
  {
    PrintLineIndex(710);
    return 1;
  }
  else if (IsObjVisible(obj) == 0)
  {
    PrintLineIndex(138);
    return 1;
  }

  return 0;
}



void TakeAllRoutine(void)
{
  int num_exceptions, num_takes, obj, container, i, j;
  unsigned short exception[80]; //stores objects not to be taken by "take all"
  unsigned short take[80];      //stores objects to be taken by "take all"

  num_exceptions = 0;
  container = 0;

  if (MatchCurWordIndex(WORD_BUT) || MatchCurWordIndex(WORD_EXCEPT)) // take all except * (and ...) (from *)
  {
    MatchCurWordIndex(WORD_FOR); // skip "for" if it exists
    for (;;)
    {
      obj = GetAllObjFromInput(Obj[OBJ_YOU].loc); if (obj <= 0) return;
      if (VerifyTakeAllExceptObj(obj)) return;
      exception[num_exceptions++] = obj; if (num_exceptions == 80 || CurWord == NumStrWords) break;
  
      if (MatchCurWordIndex(WORD_FROM) || MatchCurWordIndex(WORD_OUT) || MatchCurWordIndex(WORD_OFF))
      {
        if (TakeOutOfRoutine(&container)) return;
        break;
      }
  
      if (MatchCurWordIndex(WORD_THEN)) {CurWord--; break;} //end of this turn's command; back up so "then" can be matched later
      if (MatchCurWordIndex(WORD_AND) == 0)
        {PrintLineIndex(711); return;}
      for (;;) if (MatchCurWordIndex(WORD_AND) == 0) break; // handle repeated "and"s like "take one, two, and three"
    }
  }
  else if (MatchCurWordIndex(WORD_FROM) || MatchCurWordIndex(WORD_OUT) || MatchCurWordIndex(WORD_OFF)) // take all from * except * (and ...)
  {
    if (TakeOutOfRoutine(&container)) return;

    if (MatchCurWordIndex(WORD_BUT) || MatchCurWordIndex(WORD_EXCEPT))
    {
      MatchCurWordIndex(WORD_FOR); // skip "for" if it exists
      for (;;)
      {
        obj = GetAllObjFromInput(Obj[OBJ_YOU].loc); if (obj <= 0) return;
        if (VerifyTakeAllExceptObj(obj)) return;
        exception[num_exceptions++] = obj; if (num_exceptions == 80 || CurWord == NumStrWords) break;
    
        if (MatchCurWordIndex(WORD_THEN)) {CurWord--; break;} //end of this turn's command; back up so "then" can be matched later
        if (MatchCurWordIndex(WORD_AND) == 0)
          {PrintLineIndex(711); return;}
        for (;;) if (MatchCurWordIndex(WORD_AND) == 0) break; // handle repeated "and"s like "take one, two, and three"
      }
    }
  }

  if (container == 0 && GetPlayersVehicle() != 0)
    container = GetPlayersVehicle();

  num_takes = 0;

  for (i=2; i<NUM_OBJECTS; i++)
  {
    obj = Obj[i].order;

    if (Obj[obj].loc == (container ? INSIDE + container : Obj[OBJ_YOU].loc) &&
        (Obj[obj].prop & PROP_NOTTAKEABLE) == 0)
    {
      for (j=0; j<num_exceptions; j++)
        if (obj == exception[j]) break;
      if (j == num_exceptions)
      {
        take[num_takes++] = obj;
        if (num_takes == 80) break;
      }
    }
  }

  for (i=0; i<num_takes; i++)
  {
    obj = take[i];

    PrintObjectDesc(obj, 0);
    PrintChar(':'); PrintChar(' ');

    TakeRoutine(obj, "Okay.");
  }

  if (num_takes == 0)
    PrintLineIndex(712);
}



void ParseActionTake(void)
{
  int num_takes, obj, container, i;
  unsigned short take[80]; //stores objects to be taken

  if (MatchCurWordIndex(WORD_ALL) || MatchCurWordIndex(WORD_EVERYTHING))
    {TakeAllRoutine(); return;}

  num_takes = 0;
  container = 0;

  for (;;)
  {
    obj = GetAllObjFromInput(Obj[OBJ_YOU].loc); if (obj <= 0) return;

    take[num_takes++] = obj;
    if (num_takes == 80 || CurWord == NumStrWords) break;

    if (MatchCurWordIndex(WORD_FROM) || MatchCurWordIndex(WORD_OUT) || MatchCurWordIndex(WORD_OFF))
    {
      if (TakeOutOfRoutine(&container)) return;
      break;
    }

    if (MatchCurWordIndex(WORD_THEN))
    {
      CurWord--; //end of this turn's command; back up so "then" can be matched later
      break;
    }

    if (MatchCurWordIndex(WORD_AND) == 0)
      {PrintLineIndex(711); return;}

    for (;;) if (MatchCurWordIndex(WORD_AND) == 0) break; // handle repeated "and"s like "take one, two, and three"
  }

  if (container == 0 && GetPlayersVehicle() != 0)
    container = GetPlayersVehicle();

  for (i=0; i<num_takes; i++)
    if (VerifyTakeableObj(take[i], container, num_takes)) return;

  for (i=0; i<num_takes; i++)
  {
    obj = take[i];

    if (num_takes > 1)
    {
      PrintObjectDesc(obj, 0);
      PrintChar(':'); PrintChar(' ');
    }

    if (Obj[obj].loc == INSIDE + OBJ_YOU)
      PrintLineIndex(713);
    else if (IsObjVisible(obj) == 0)
      PrintLineIndex(714);
    else
      TakeRoutine(obj, "Okay.");
  }
}



int DropPutVerifyContainer(int container)
{
  if (container == FOBJ_SCENERY_NOTVIS || container == FOBJ_NOTVIS)
  {
    PrintLineIndex(138);
    return 1;
  }

  if (container == OBJ_YOU)
  {
    PrintLineIndex(478);
    return 1;
  }

  if (container < NUM_OBJECTS)
  {
    if (IsObjVisible(container) == 0)
    {
      PrintObjectDesc(container, 0);
      PrintLineIndex(697);
      return 1;
    }

    if ((Obj[container].prop & PROP_OPEN) == 0 ||
        (Obj[container].prop & PROP_ACTOR))
    {
      PrintObjectDesc(container, 0);
      PrintChar(':'); PrintChar(' ');

      if (Obj[container].prop & PROP_OPENABLE)
      {
        PrintLineIndex(698);
        ItObj = container;
      }
      else
        PrintLineIndex(584);

      return 1;
    }
  }

  return 0;
}



void DropPutAllRoutine(int put_flag)
{
  int i, j, obj, num_exceptions, nothing_dropped, into_flag, outside_flag, container, prev_darkness;
  unsigned short exception[80]; //stores objects not to be dropped/put by "drop/put all"

  num_exceptions = 0;
  container = 0;
  into_flag = 0;
  outside_flag = 0;

  if (MatchCurWordIndex(WORD_BUT) || MatchCurWordIndex(WORD_EXCEPT))
  {
    MatchCurWordIndex(WORD_FOR); // skip "for" if it exists
    for (;;)
    {
      obj = GetAllObjFromInput(Obj[OBJ_YOU].loc); if (obj <= 0) return;
      if (obj == OBJ_YOU || obj >= NUM_OBJECTS)
        {PrintLineIndex(715); return;}
      if (Obj[obj].loc != INSIDE + OBJ_YOU)
        {PrintLineIndex(715); return;}
      exception[num_exceptions++] = obj; if (num_exceptions == 80 || CurWord == NumStrWords) break;

      if (MatchCurWordIndex(WORD_IN) || MatchCurWordIndex(WORD_INTO) || MatchCurWordIndex(WORD_INSIDE) ||
          MatchCurWordIndex(WORD_THROUGH) || MatchCurWordIndex(WORD_ON) || MatchCurWordIndex(WORD_ONTO))
        into_flag = 1;
      else if (MatchCurWordIndex(WORD_OUTSIDE))
        outside_flag = 1;
      if (into_flag || outside_flag)
      {
        put_flag = 1; // change "drop" to "put" (if not already)
        container = GetAllObjFromInput(Obj[OBJ_YOU].loc); if (container <= 0) return;
        if (DropPutVerifyContainer(container)) return;
        break;
      }

      if (MatchCurWordIndex(WORD_THEN)) {CurWord--; break;} //end of this turn's command; back up so "then" can be matched later
      if (MatchCurWordIndex(WORD_AND) == 0)
        {PrintLineIndex(711); return;}
      for (;;) if (MatchCurWordIndex(WORD_AND) == 0) break; // handle repeated "and"s like "take one, two, and three"
    }
  }
  else
  {
    if (MatchCurWordIndex(WORD_IN) || MatchCurWordIndex(WORD_INTO) || MatchCurWordIndex(WORD_INSIDE) ||
        MatchCurWordIndex(WORD_THROUGH) || MatchCurWordIndex(WORD_ON) || MatchCurWordIndex(WORD_ONTO))
      into_flag = 1;
    else if (MatchCurWordIndex(WORD_OUTSIDE))
      outside_flag = 1;
    if (into_flag || outside_flag)
    {
      put_flag = 1; // change "drop" to "put" (if not already)
      container = GetAllObjFromInput(Obj[OBJ_YOU].loc); if (container <= 0) return;
      if (DropPutVerifyContainer(container)) return;

      if (MatchCurWordIndex(WORD_BUT) || MatchCurWordIndex(WORD_EXCEPT))
      {
        MatchCurWordIndex(WORD_FOR); // skip "for" if it exists
        for (;;)
        {
          obj = GetAllObjFromInput(Obj[OBJ_YOU].loc); if (obj <= 0) return;
          if (obj == OBJ_YOU || obj >= NUM_OBJECTS)
            {PrintLineIndex(715); return;}
          if (Obj[obj].loc != INSIDE + OBJ_YOU)
            {PrintLineIndex(715); return;}
          exception[num_exceptions++] = obj; if (num_exceptions == 80 || CurWord == NumStrWords) break;
    
          if (MatchCurWordIndex(WORD_THEN)) {CurWord--; break;} //end of this turn's command; back up so "then" can be matched later
          if (MatchCurWordIndex(WORD_AND) == 0)
            {PrintLineIndex(711); return;}
          for (;;) if (MatchCurWordIndex(WORD_AND) == 0) break; // handle repeated "and"s like "take one, two, and three"
        }
      }
    }
  }

  if (put_flag)
  {
    if (into_flag == 0 && outside_flag == 0)
      {PrintLineIndex(716); return;}

    if (outside_flag)
    {
      if (container != GetPlayersVehicle())
        {PrintLineIndex(717); return;}

      put_flag = 0;
      container = 0;
    }
  }

  if (outside_flag == 0 && container == 0 && GetPlayersVehicle() != 0)
  {
    put_flag = 1;
    container = GetPlayersVehicle();
  }

  for (i=2; i<NUM_OBJECTS; i++)
  {
    obj = Obj[i].order;

    if (Obj[obj].loc == INSIDE + OBJ_YOU && obj != container)
    {
      for (j=0; j<num_exceptions; j++)  // look for obj in exception list
        if (obj == exception[j]) break; // if obj is in exception list, break early
      if (j == num_exceptions)          // if obj is not in exception list
      {
        if (InterceptDropPutObj(obj, container, 1, 1) == -1) // first 1: test only
          return;
      }
    }
  }

  prev_darkness = IsPlayerInDarkness();

  nothing_dropped = 1;

  for (;;)
  {
    for (i=2; i<NUM_OBJECTS; i++)
    {
      obj = Obj[i].order;

      if (Obj[obj].loc == INSIDE + OBJ_YOU && obj != container)
      {
        for (j=0; j<num_exceptions; j++)  // look for obj in exception list
          if (obj == exception[j]) break; // if obj is in exception list, break early
        if (j == num_exceptions)          // if obj is not in exception list
        {
          // caution: if obj doesn't leave inventory here, or function return, outer loop will never end

          nothing_dropped = 0;

          if (InterceptDropPutObj(obj, container, 0, 1) == 0) // first 0: not a test
          {
            PrintObjectDesc(obj, 0);
            PrintChar(':'); PrintChar(' ');
  
            if (put_flag &&
                GetTotalSizeInLocation(INSIDE + container) + GetObjectSize(obj) > GetObjectCapacity(container))
              {PrintLineIndex(718); return;}
  
            Obj[obj].loc = put_flag ? INSIDE + container : Obj[OBJ_YOU].loc;
            MoveObjOrderToLast(obj);
            PrintLineIndex(296);
            TimePassed = 1;
          }

          break; // break inner loop early to cause outer loop to repeat for any remaining objects
        }
      }
    }
    if (i == NUM_OBJECTS) break; // break outer loop if inner loop was not broken above
  }

  if (nothing_dropped)
  {
    if (put_flag)
    {
      if (Obj[container].prop & PROP_SURFACE)
        PrintLineIndex(719);
      else
        PrintLineIndex(720);
    }
    else
      PrintLineIndex(721);
  }

  if (IsPlayerInDarkness() != prev_darkness)
  {
    PrintNewLine();
    PrintPlayerRoomDesc(1);
  }
}



void ParseActionDropPut(int put_flag)
{
  int obj, i, num_drops, container, into_flag, outside_flag, prev_darkness;
  unsigned short drop[80]; //stores objects to be dropped

  if (MatchCurWordIndex(WORD_ALL) || MatchCurWordIndex(WORD_EVERYTHING))
    {DropPutAllRoutine(put_flag); return;}

  container = 0;
  into_flag = 0;
  outside_flag = 0;
  num_drops = 0;

  for (;;)
  {
    obj = GetAllObjFromInput(Obj[OBJ_YOU].loc); if (obj <= 0) return;
    drop[num_drops++] = obj;
    if (num_drops == 80 || CurWord == NumStrWords) break;

    if (MatchCurWordIndex(WORD_IN) || MatchCurWordIndex(WORD_INTO) || MatchCurWordIndex(WORD_INSIDE) ||
        MatchCurWordIndex(WORD_THROUGH) || MatchCurWordIndex(WORD_ON) || MatchCurWordIndex(WORD_ONTO))
    {
      put_flag = 1; // change "drop" to "put" (if not already)
      into_flag = 1;

      container = GetAllObjFromInput(Obj[OBJ_YOU].loc); if (container <= 0) return;
      if (DropPutVerifyContainer(container)) return;

      break;
    }

    if (MatchCurWordIndex(WORD_OUTSIDE))
    {
      put_flag = 1; // change "drop" to "put" (if not already)
      outside_flag = 1;

      container = GetAllObjFromInput(Obj[OBJ_YOU].loc); if (container <= 0) return;
      if (DropPutVerifyContainer(container)) return;

      break;
    }

    if (MatchCurWordIndex(WORD_THEN)) {CurWord--; break;} // end of this turn's command; back up so "then" can be matched later
    if (MatchCurWordIndex(WORD_AND) == 0)
      {PrintLineIndex(711); return;}
    for (;;) if (MatchCurWordIndex(WORD_AND) == 0) break; // handle repeated "and"s like "take one, two, and three"
  }

  if (put_flag)
  {
    if (into_flag == 0 && outside_flag == 0)
      {PrintLineIndex(716); return;}

    if (outside_flag)
    {
      if (container != GetPlayersVehicle())
        {PrintLineIndex(717); return;}

      put_flag = 0;
      container = 0;
    }
  }

  if (outside_flag == 0 && container == 0 && GetPlayersVehicle() != 0)
  {
    put_flag = 1;
    container = GetPlayersVehicle();
  }

  for (i=0; i<num_drops; i++)
  {
    obj = drop[i];

    if (obj == OBJ_YOU || obj >= NUM_OBJECTS)
    {
      if (num_drops > 1)
        PrintLineIndex(715);
      else
        PrintLineIndex(722);
      return;
    }

    if (InterceptDropPutObj(obj, container, 1, (num_drops > 1)) == -1) // first 1: test only
      return;
  }

  prev_darkness = IsPlayerInDarkness();

  for (i=0; i<num_drops; i++)
  {
    obj = drop[i];

    if (Obj[obj].loc != INSIDE + OBJ_YOU)
    {
      if (num_drops > 1)
        {PrintObjectDesc(obj, 0); PrintChar(':'); PrintChar(' ');}
      PrintLineIndex(722);
    }
    else if (obj == container)
    {
      if (num_drops > 1)
        {PrintObjectDesc(obj, 0); PrintChar(':'); PrintChar(' ');}
      PrintLineIndex(723);
    }
    else if (InterceptDropPutObj(obj, container, 0, (num_drops > 1)) == 0) // first 0: not a test
    {
      if (num_drops > 1)
        {PrintObjectDesc(obj, 0); PrintChar(':'); PrintChar(' ');}
      if (put_flag && GetTotalSizeInLocation(INSIDE + container) + GetObjectSize(obj) > GetObjectCapacity(container))
        PrintLineIndex(718);
      else
      {
        Obj[obj].loc = put_flag ? INSIDE + container : Obj[OBJ_YOU].loc;
        MoveObjOrderToLast(obj);
        PrintLineIndex(296);
        TimePassed = 1;
      }
    }
  }

  if (IsPlayerInDarkness() != prev_darkness)
  {
    PrintNewLine();
    PrintPlayerRoomDesc(1);
  }
}



// returns 1 if obj is not visible or not specific enough

int ValidateObject(int obj)
{
  if (obj == FOBJ_SCENERY_NOTVIS || obj == FOBJ_NOTVIS)
  {
    PrintLineIndex(138);
    return 1;
  }

  if (obj == FOBJ_AMB)
  {
    PrintLineIndex(700);
    return 1;
  }

  if (obj > 1 && obj < NUM_OBJECTS && IsObjVisible(obj) == 0)
  {
    PrintLineIndex(138);
    return 1;
  }

  return 0;
}



// CAUTION: called function will have to check if with_to >= NUM_OBJECTS

void ParseActionWithTo(int action, int match_hack, const char *cant)
{
  int obj, with_to, i;

  obj = GetAllObjFromInput(Obj[OBJ_YOU].loc); if (obj <= 0) return;
  if (ValidateObject(obj)) return;

  if (match_hack != 0) MatchCurWordIndex(match_hack); //skip specified word

  with_to = 0;
  if (MatchCurWordIndex(WORD_WITH) || MatchCurWordIndex(WORD_USING) || MatchCurWordIndex(WORD_TO) ||
      MatchCurWordIndex(WORD_FROM) || MatchCurWordIndex(WORD_ON) || MatchCurWordIndex(WORD_ONTO))
  {
    with_to = GetAllObjFromInput(Obj[OBJ_YOU].loc); if (with_to <= 0) return;
  }
  if (ValidateObject(with_to)) return;

  for (i=0; GetDoMiscWithTo_Action(i) != 0; i++)
    if (GetDoMiscWithTo_Action(i) == action && GetDoMiscWithTo_Obj(i) == obj)
  {
    CallDoMiscWithTo(i, with_to);
    return;
  }

  if (obj == OBJ_YOU) {PrintLineIndex(478); return;}

  PrintTextIndex(724);
  PrintLine(cant);
}



// for use outside parser
// returns -1 if failure message was printed

int GetWith(void)
{
  int with = 0;

  if (MatchCurWordIndex(WORD_WITH) || MatchCurWordIndex(WORD_USING))
  {
    with = GetAllObjFromInput(Obj[OBJ_YOU].loc); if (with <= 0) return -1;
  }

  if (ValidateObject(with)) return -1;

  if (with >= NUM_OBJECTS)
    {PrintLineIndex(204); return -1;}

  return with;
}



void ParseActionExamine(void)
{
  int obj, i, flag;

  obj = GetAllObjFromInput(Obj[OBJ_YOU].loc);
  if (obj <= 0) return;

  if (obj == FOBJ_SCENERY_NOTVIS || obj == FOBJ_NOTVIS)
    {PrintLineIndex(476); return;}

  if (obj == FOBJ_AMB)
    {PrintLineIndex(477); return;}

  //fixed objects only
  for (i=0; GetDoMisc_Action(i) != 0; i++)
    if (GetDoMisc_Action(i) == A_EXAMINE && GetDoMisc_Obj(i) >= NUM_OBJECTS && GetDoMisc_Obj(i) == obj)
  {
    CallDoMisc(i);
    return;
  }

  if (obj == OBJ_YOU)
    {PrintLineIndex(725); return;}

  if (obj >= NUM_OBJECTS)
    {PrintLineIndex(365); return;}

  if (IsObjVisible(obj) == 0)
  {
    TimePassed = 0;
    PrintLineIndex(714);
    return;
  }

  //non-fixed objects only
  for (i=0; GetDoMisc_Action(i) != 0; i++)
    if (GetDoMisc_Action(i) == A_EXAMINE && GetDoMisc_Obj(i) < NUM_OBJECTS && GetDoMisc_Obj(i) == obj)
  {
    CallDoMisc(i);
    return;
  }


  flag = 0;

  if (Obj[obj].prop & PROP_LIT)
  {
    flag = 1;
    PrintLineIndex(726);
  }

  if (Obj[obj].prop & PROP_OPENABLE)
  {
    flag = 1;
    if (Obj[obj].prop & PROP_OPEN)
      PrintLineIndex(426);
    else
      PrintLineIndex(727);
  }

  if ((Obj[obj].prop & PROP_OPEN) &&
      (Obj[obj].prop & PROP_ACTOR) == 0)
  {
    flag = 1;
    PrintContents(obj, "It contains:", 1); // 1: allow printing "It's empty."
  }

  if (flag == 0)
    PrintLineIndex(365);
}



// called function must check if player is holding obj

void ParseActionGive(void)
{
  int obj, to, flag, swap;

  obj = GetAllObjFromInput(Obj[OBJ_YOU].loc); if (obj <= 0) return;
  flag = MatchCurWordIndex(WORD_TO);
  to = GetAllObjFromInput(Obj[OBJ_YOU].loc); if (to <= 0) return;

  if (flag == 0)
  {
    //if "to" omitted, swap obj and to, as in "give plant water"
    swap = obj;
    obj = to;
    to = swap;
  }

  if (obj == FOBJ_SCENERY_NOTVIS || obj == FOBJ_NOTVIS)
    {PrintLineIndex(138); return;}

  if (obj == FOBJ_AMB)
    {PrintLineIndex(700); return;}

  if (obj == OBJ_YOU)
    {PrintLineIndex(478); return;}

  if (obj >= NUM_OBJECTS)
    {PrintLineIndex(204); return;}


  if (to == FOBJ_SCENERY_NOTVIS || to == FOBJ_NOTVIS)
    {PrintLineIndex(138); return;}

  if (to == FOBJ_AMB)
    {PrintLineIndex(700); return;}

  if (to == OBJ_YOU)
    {PrintLineIndex(478); return;}

  if (to > 0 && to < NUM_OBJECTS && IsObjVisible(to) == 0)
    {PrintLineIndex(138); return;}


  if (CallDoMiscGiveTo(to, obj)) return;

  PrintLineIndex(728);
}



void ParseActionRestartOrQuit(int action)
{
  for (;;)
  {
    if (action == A_RESTART)
      PrintLineIndex(729);
    else
      PrintLineIndex(730);

    GetWords(0);

    if (MatchCurWordIndex(WORD_Y) || MatchCurWordIndex(WORD_YES))
    {
      GameOver = (action == A_RESTART) ? 2 : 1;
      return;
    }

    if (MatchCurWordIndex(WORD_N) || MatchCurWordIndex(WORD_NO)) return;

    PrintLineIndex(685);
  }
}



void OpenObj(int obj)
{
  int prev_darkness;

  if ((Obj[obj].prop & PROP_OPENABLE) == 0 ||
      (Obj[obj].prop & PROP_ACTOR))
    {PrintLineIndex(731); return;}

  if (Obj[obj].prop & PROP_OPEN)
    {PrintLineIndex(286); return;}

  prev_darkness = IsPlayerInDarkness();

  Obj[obj].prop |= PROP_OPEN; //open object
  PrintLineIndex(296);
  TimePassed = 1;

  if (IsPlayerInDarkness() == 0)
    PrintContents(obj, "It contains:", 0); // 0: don't print "It's empty."

  if (IsPlayerInDarkness() != prev_darkness)
  {
    PrintNewLine();
    PrintPlayerRoomDesc(1);
  }
}



void CloseObj(int obj)
{
  int prev_darkness;

  if ((Obj[obj].prop & PROP_OPENABLE) == 0 ||
      (Obj[obj].prop & PROP_ACTOR))
    {PrintLineIndex(732); return;}

  if ((Obj[obj].prop & PROP_OPEN) == 0)
    {PrintLineIndex(288); return;}

  prev_darkness = IsPlayerInDarkness();

  Obj[obj].prop &= ~PROP_OPEN; //close object
  PrintLineIndex(296);
  TimePassed = 1;

  if (IsPlayerInDarkness() != prev_darkness)
  {
    PrintNewLine();
    PrintPlayerRoomDesc(1);
  }
}



void LookInObj(int obj)
{
  if ((Obj[obj].prop & PROP_OPEN) &&
      (Obj[obj].prop & PROP_ACTOR) == 0)
    PrintContents(obj, "It contains:", 1); // 1: allow printing "It's empty."
  else
  {
    if ((Obj[obj].prop & PROP_OPENABLE) == 0)
      PrintLineIndex(733);
    else
      PrintLineIndex(727);
  }
}



void EmptyObj(int obj)
{
  int flag = 0, i;

  if ((Obj[obj].prop & PROP_OPENABLE) == 0)
    {PrintLineIndex(734); return;}

  if ((Obj[obj].prop & PROP_OPEN) == 0)
    {PrintLineIndex(727); return;}

  for (i=2; i<NUM_OBJECTS; i++)
  {
    int obj_inside = Obj[i].order;

    if (Obj[obj_inside].loc == INSIDE + obj)
    {
      flag = 1;
      TimePassed = 1;

      Obj[obj_inside].loc = Obj[OBJ_YOU].loc;
      Obj[obj_inside].prop |= PROP_MOVEDDESC;

      PrintObjectDesc(obj_inside, 0);
      PrintLineIndex(735);
    }
  }

  if (flag == 0)
    PrintLineIndex(736);
}



void ParseActionWhereIs(void)
{
  int obj, i;

  obj = GetAllObjFromInput(Obj[OBJ_YOU].loc);

  if (obj <= 0) return;
  else if (obj == FOBJ_SCENERY_NOTVIS || obj == FOBJ_NOTVIS)
    {PrintLineIndex(737); return;}
  else if (obj == FOBJ_AMB)
    {PrintLineIndex(477); return;}

  for (i=0; GetDoMisc_Action(i) != 0; i++)
    if (GetDoMisc_Action(i) == A_WHEREIS && GetDoMisc_Obj(i) == obj)
  {
    CallDoMisc(i);
    return;
  }

  if (obj == OBJ_YOU)
    PrintLineIndex(738);
  else if (obj >= NUM_OBJECTS)
    PrintLineIndex(739);
  else if (IsObjVisible(obj))
  {
    if (Obj[obj].loc == INSIDE + OBJ_YOU)
      PrintLineIndex(740);
    else
      PrintLineIndex(739);
  }
  else
    PrintLineIndex(741);
}



void ParseActionThrow(void)
{
  int obj, to, i;
  unsigned char prep[13] =
  { WORD_ACROSS, WORD_AT, WORD_FROM, WORD_IN, WORD_INSIDE, WORD_INTO, WORD_OFF,
    WORD_OUT, WORD_OUTSIDE, WORD_OVER, WORD_TO, WORD_TOWARD, WORD_THROUGH
  };

  obj = GetAllObjFromInput(Obj[OBJ_YOU].loc); if (obj <= 0) return;
  to = 0;

  for (i=0; i<13; i++)
    if (MatchCurWordIndex(prep[i])) break;
  if (i < 13)
  {
    MatchCurWordIndex(WORD_OF); // as in "throw ball out of park"
    to = GetAllObjFromInput(Obj[OBJ_YOU].loc); if (to <= 0) return;
  }

  if (obj == FOBJ_SCENERY_NOTVIS || obj == FOBJ_NOTVIS)
    {PrintLineIndex(138); return;}
  else if (obj == FOBJ_AMB)
    {PrintLineIndex(700); return;}
  else if (obj == OBJ_YOU)
    {PrintLineIndex(478); return;}
  else if (obj >= NUM_OBJECTS)
    {PrintLineIndex(204); return;}
  else if (Obj[obj].loc != INSIDE + OBJ_YOU)
    {PrintLineIndex(204); return;}

  if (to == FOBJ_SCENERY_NOTVIS || to == FOBJ_NOTVIS)
    {PrintLineIndex(138); return;}
  else if (to == FOBJ_AMB)
    {PrintLineIndex(700); return;}
  else if (to == OBJ_YOU)
    {PrintLineIndex(478); return;}
  else if (to > 0 && to < NUM_OBJECTS && IsObjVisible(to) == 0)
    {PrintLineIndex(138); return;}

  if (CallDoMiscThrowTo(to, obj)) return;

  ThrowObjRoutine(obj, to);
}



void PrintCantAction(int action)
{
  switch (action)
  {
    case A_OPEN:         PrintLineIndex(731);                 break;
    case A_CLOSE:        PrintLineIndex(732);                break;
    case A_LOOKIN:       PrintLineIndex(742);          break;
    case A_LOOKTHROUGH:  PrintLineIndex(743);         break;
    case A_MOUNT:        PrintLineIndex(744);               break;
    case A_DISMOUNT:     PrintLineIndex(745);                  break;
    case A_EAT:          PrintLineIndex(746); break;
    case A_DRINK:        PrintLineIndex(747);                break;
    case A_WEAR:         PrintLineIndex(748);                 break;
    case A_REMOVE:       PrintLineIndex(749);             break;
    case A_PLAY:         PrintLineIndex(750);                 break;
    case A_SLEEPON:      PrintLineIndex(751);             break;
    case A_RAISE:        PrintLineIndex(752);                break;
    case A_LOWER:        PrintLineIndex(753);                break;
    case A_ENTER:        PrintLineIndex(754);            break;
    case A_EXIT:         PrintLineIndex(755);                  break;
    case A_READ:         PrintLineIndex(446);             break;
    case A_RING:         PrintLineIndex(756);                 break;
    case A_WIND:         PrintLineIndex(757);                 break;
    case A_CLIMB:        PrintLineIndex(758);                break;
    case A_CLIMBUP:      PrintLineIndex(759);             break;
    case A_CLIMBDOWN:    PrintLineIndex(760);           break;
    case A_SLIDEUP:      PrintLineIndex(761);             break;
    case A_SLIDEDOWN:    PrintLineIndex(762);           break;
    case A_CLIMBTHROUGH: PrintLineIndex(763);        break;
    case A_LISTENTO:     PrintLineIndex(764);                   break;
    case A_CROSS:        PrintLineIndex(765);                break;
    case A_EXORCISE:     PrintLineIndex(766);              break;
    case A_SMELL:        PrintLineIndex(468);       break;

    default:             PrintLineIndex(240);                break;
  }
}



void DoActionOnObject(int action)
{
  int must_hold = 0, obj, i;

  // eat and drink must-hold checks must be handled by called function
  switch (action)
  {
    case A_EMPTY:
    case A_WEAR:
    case A_REMOVE:
    case A_PLAY:
    case A_RING:
    case A_WIND:
    case A_WAVE:
      must_hold = 1;
    break;
  }

  switch (action)
  {
    case A_ENTER:
    case A_EXIT:
      MatchCurWordIndex(WORD_OF); // "go inside of"  "get out of"
    break;
  }

  obj = GetAllObjFromInput(Obj[OBJ_YOU].loc); if (obj <= 0) return;

  switch (action)
  {
    case A_WEAR:   MatchCurWordIndex(WORD_ON);  break; // "put obj on"
    case A_REMOVE: MatchCurWordIndex(WORD_OFF); break; // "take obj off"
  }

  if (obj == FOBJ_SCENERY_NOTVIS || obj == FOBJ_NOTVIS)
    {PrintLineIndex(476); return;}

  if (obj == FOBJ_AMB)
    {PrintLineIndex(477); return;}

  if (must_hold && obj == OBJ_YOU)
    {PrintLineIndex(478); return;}

  if (obj > 1 && obj < NUM_OBJECTS)
  {
    if (IsObjVisible(obj) == 0)
      {PrintLineIndex(714); return;}

    if (must_hold && Obj[obj].loc != INSIDE + OBJ_YOU)
      {PrintLineIndex(767); return;}
  }

  for (i=0; GetDoMisc_Action(i) != 0; i++)
    if (GetDoMisc_Action(i) == action && GetDoMisc_Obj(i) == obj)
  {
    CallDoMisc(i);
    return;
  }

  if (obj == OBJ_YOU)
    {PrintLineIndex(478); return;}

  if (obj > 1 && obj < NUM_OBJECTS)
    switch (action)
  {
    case A_OPEN:   OpenObj(obj);   return;
    case A_CLOSE:  CloseObj(obj);  return;
    case A_LOOKIN: LookInObj(obj); return;
    case A_EMPTY:  EmptyObj(obj);  return;
  }

  PrintCantAction(action);
}
//*****************************************************************************



//*****************************************************************************
void Parse(void)
{
  int obj, action, i, temp, temp2;


  if (CurWord == NumStrWords) return;


  // "actor, command1 (then command2 ...)"
  // match noun phrase instead of verb phrase here to see if player is talking to actor
  // NOTES:
  //   comma is represented by "and"
  //   all following actions in input will be directed at actor

  temp = CurWord;

  obj = GetObjFromInput(); // note that this can return -1 (noun phrase not specific enough)

  if (MatchCurWordIndex(WORD_AND) && obj > 1 && (Obj[obj].prop & PROP_ACTOR)) // and = ,
    {DoCommandActor(obj); return;}

  CurWord = temp;


  action = GetActionFromInput();

  if (InterceptAction(action)) return;

  if (action == 0)
  {
    PrintLineIndex(768);
    return;
  }


  //hacks to allow "turn obj on/off" "take obj off" "put obj on" to be
  //caught by "activate/deactivate" "remove" and "wear"
  //NOTE: this will allow strange commands like "spin obj on" "remove obj off" "wear obj on"

  temp = CurWord; SkipObjFromInput(Obj[OBJ_YOU].loc);

  if (action == A_TURN && MatchCurWordIndex(WORD_ON))
    action = A_ACTIVATE; // "turn obj on"

  if (action == A_TURN && MatchCurWordIndex(WORD_OFF))
    action = A_DEACTIVATE; // "turn obj off""

  if (action == A_TAKE && MatchCurWordIndex(WORD_OFF))
  {
    MatchCurWordIndex(WORD_OF);
    // if no obj is after "off (of)", change action to remove
    temp2 = CurWord; SkipObjFromInput(Obj[OBJ_YOU].loc);
    if (temp2 == CurWord) action = A_REMOVE; // "take obj off"
  }

  if (action == A_PUT && MatchCurWordIndex(WORD_ON))
  {
    // if no obj is after "on", change action to wear
    temp2 = CurWord; SkipObjFromInput(Obj[OBJ_YOU].loc);
    if (temp2 == CurWord) action = A_WEAR; // "put obj on"
  }

  CurWord = temp;


  //replace "go dir" with "dir"
  if (action == A_GO)
    for (i=0; ; i++)
  {
    int w1, w2;

    action = GetVerbToAction_Action(i);
    if (action == 0) //reached end of list without finding anything
    {
      PrintLineIndex(769);
      return;
    }

    GetVerbToAction_Words(i, &w1, &w2);
    if (action >= A_NORTH && action <= A_OUT && MatchCurWordIndex(w1)) break;
  }

  //special movements; executed function can fall through if it returns 0
  //A_IN and A_OUT must be handled here
  for (i=0; ; i++)
  {
    if (GetGoFrom_Action(i) == 0) break;

    if (GetGoFrom_Room(i) == Obj[OBJ_YOU].loc && GetGoFrom_Action(i) == action)
    {
      SkipObjFromInput(Obj[OBJ_YOU].loc); // hack; in case player types something like "go in house"

      if (CallGoFrom(i)) return;
    }
  }

  if (action == A_IN || action == A_OUT) //if these actions are not handled above, print blocked message
  {
    PrintBlockMsg(BL0);
    return;
  }

  if (action >= A_NORTH && action <= A_DOWN)
  {
    SkipObjFromInput(Obj[OBJ_YOU].loc); // hack; in case player types something like "go down chute"

    ParseActionDirection(action);
    return;
  }


  switch (action)
  {
    case A_SAVE:     DoSave();     return;
    case A_RESTORE:  DoRestore();  return;
    case A_SCORE:    PrintScore(); return;
    case A_VERSION:  DoVersion();  return;
    case A_DIAGNOSE: DoDiagnose(); return;

    case A_RESTART:
    case A_QUIT:
      ParseActionRestartOrQuit(action);
      return;

    case A_BRIEF:      Verbosity = V_BRIEF;      PrintLineIndex(770); return;
    case A_VERBOSE:    Verbosity = V_VERBOSE;    PrintLineIndex(771); PrintPlayerRoomDesc(1); return;
    case A_SUPERBRIEF: Verbosity = V_SUPERBRIEF; PrintLineIndex(772); return;

    case A_ECHO:     DoEcho();     return;
    case A_PRAY:     DoPray();     return;
    case A_ODYSSEUS: DoOdysseus(); return;

    case A_LOOK: PrintPlayerRoomDesc(1);            return;
    case A_WAIT: PrintLineIndex(773); TimePassed = 1; return;

    case A_ACTIVATE:   ParseActionWithTo(action, WORD_ON, "light that!");   return;
    case A_DEACTIVATE: ParseActionWithTo(action, WORD_OFF, "extinguish that!"); return;

    // NOTE: when in darkness, player is still allowed to open/close because light might be inside container
    case A_OPEN:
    case A_CLOSE:
      DoActionOnObject(action);
      return;
  }

  // actions above this line work when in darkness
  if (IsPlayerInDarkness()) {PrintLineIndex(774); return;}
  // actions below this line are not possible when in darkness

  switch (action)
  {
    case A_INVENTORY: PrintPresentObjects(INSIDE + OBJ_YOU, 0, 0); return;

    case A_TAKE:    ParseActionTake();     return;
    case A_DROP:    ParseActionDropPut(0); return;
    case A_PUT:     ParseActionDropPut(1); return;
    case A_EXAMINE: ParseActionExamine();  return;
    case A_GIVE:    ParseActionGive();     return;
    case A_WHEREIS: ParseActionWhereIs();  return;
    case A_THROW:   ParseActionThrow();    return;

    case A_DIG:     ParseActionWithTo(action, 0, "dig that!");     return;
    case A_LOCK:    ParseActionWithTo(action, 0, "lock that!");    return;
    case A_UNLOCK:  ParseActionWithTo(action, 0, "unlock that!");  return;
    case A_TURN:    ParseActionWithTo(action, 0, "turn that!");    return;
    case A_OIL:     ParseActionWithTo(action, 0, "oil that!");     return;
    case A_TIE:     ParseActionWithTo(action, 0, "tie that!");     return;
    case A_UNTIE:   ParseActionWithTo(action, 0, "untie that!");   return;
    case A_FIX:     ParseActionWithTo(action, 0, "fix that!");     return;
    case A_INFLATE: ParseActionWithTo(action, 0, "inflate that!"); return;
    case A_DEFLATE: ParseActionWithTo(action, 0, "deflate that!"); return;
    case A_FILL:    ParseActionWithTo(action, 0, "fill that!");    return;
    case A_ATTACK:  ParseActionWithTo(action, 0, "attack that!");  return;
    case A_POUR:    ParseActionWithTo(action, 0, "pour that!");    return;
    case A_BRUSH:   ParseActionWithTo(action, 0, "brush that!");   return;

    case A_JUMP:           DoJump();           return;
    case A_SLEEP:          DoSleep();          return;
    case A_DISEMBARK:      DoDisembark();      return;
    case A_LAUNCH:         DoLaunch();         return;
    case A_LAND:           DoLand();           return;
    case A_SWIM:           DoSwim();           return;
    case A_TALKTO:         DoTalkTo();         return;
    case A_GREET:          DoGreet();          return;
    case A_SAY:            DoSay();            return;
    case A_TEMPLETREASURE: DoTempleTreasure(); return;

    default:
      DoActionOnObject(action);
      return;
  }

  //not reached
}
//*****************************************************************************



//*****************************************************************************

extern int ConsoleH, CursorRow;



void RestartGame(void)
{
  int i;

  InitGameState(); // sets ItObj

  NumStrWords = 0;
  NumOopsWords = 0;
  NumAgainWords = 0;

  GameOver = 0;

  // print newlines until all existing text is scrolled off screen
  i = ConsoleH*2-CursorRow;
  while (i-- > 0)
  {
    PrintNewLine();
    NumPrintedLines = 0;
  }

  DoIntro();

  PrintPlayerRoomDesc(0);
}



// returning from this function is equivalent to resetting arduino or closing program
void GameLoop(void)
{
  unsigned char allow_blank_line = 0;

  RestartGame();

  for (;;)
  {
    int temp, old_num, old_cur, i;

    while (NumStrWords == 0)
    {
      GetWords(1);
      allow_blank_line = 0;
    }

    if (MatchCurWordIndex(WORD_THEN)) continue;

    if (CurWord == NumStrWords) {NumStrWords = 0; continue;}

    PrevItObj = ItObj;
    ItObj = 0;

    TimePassed = 0;


    if (MatchCurWordIndex(WORD_G) || MatchCurWordIndex(WORD_AGAIN))
    {
      if (NumAgainWords == 0) {PrintLineIndex(775); NumStrWords = 0; continue;}

      old_num = NumStrWords; NumStrWords = NumAgainWords;
      old_cur = CurWord;     CurWord = 0;
      AgainMode = 1;
    }

    if (allow_blank_line && Verbosity != V_SUPERBRIEF)
      PrintNewLine();
    allow_blank_line = 1;

    temp = CurWord;

    Parse();

    if (AgainMode)
    {
      NumStrWords = old_num;
      CurWord = old_cur;
      AgainMode = 0;
    }
    else
    {
      NumAgainWords = CurWord - temp;
      for (i=0; i<NumAgainWords; i++)
        AgainWord[i] = StrWord[temp + i];
    }


    if (MatchCurWordIndex(WORD_THEN) == 0) NumStrWords = 0;

    if (TimePassed)
    {
      NumMoves++;
      if (GameOver == 0) RunEventRoutines();
    }
    else NumStrWords = 0;

    if (GameOver)
    {
      if (GameOver == 2) {RestartGame(); continue;}

      PrintScore();
      for (;;)
      {
        PrintLineIndex(776);
        GetWords(0);

             if (MatchCurWordIndex(WORD_AUTOPLAY)) {RestartGame(); ActivateAutoPlay(); break;} // secret option
        else if (MatchCurWordIndex(WORD_RESTART )) {RestartGame(); break;}
        else if (MatchCurWordIndex(WORD_RESTORE )) {if (DoRestore()) break;}
        else if (MatchCurWordIndex(WORD_EXIT    )) return;
      }
      continue;
    }
  }
}

//*****************************************************************************
