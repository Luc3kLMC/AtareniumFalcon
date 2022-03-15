#include <ace/managers/viewport/simplebuffer.h>
#include <ace/managers/joy.h>
#include <ace/managers/key.h>
#include <ace/managers/system.h>
#include <ace/managers/game.h>
#include <ace/utils/palette.h>
#include <stdio.h>
#include <ace/managers/state.h>
#include <ace/managers/blit.h>
#include <ace/utils/font.h>
#include <ace/managers/ptplayer.h>

#define CREDITS_LINE_COUNT 23

enum StatesCredits
{
  STATE_LMC_FADE_IN,
  STATE_LMC_WAIT,
  STATE_LMC_FADE_OUT,
  STATE_ACE_FADE_IN,
  STATE_ACE_WAIT,
  STATE_ACE_FADE_OUT,
  STATE_CREDITS,
};

static enum StatesCredits s_eState;

static tView *s_pView;
static tVPort *s_pVp;
static tSimpleBufferManager *s_pVpManager;

static tBitMap *s_pLMC;
static tBitMap *s_pACE;

static tPtplayerSfx *s_pACEsfx;
static tPtplayerSfx *s_pLMCsfx;

extern tState g_sStateMenu;
extern tStateManager *g_pStateMachineGame;
extern tState g_sStateIntro;

static tFont *s_pFont;
static tTextBitMap *s_pBmText;

static UWORD s_pPaletteLMC_ACE[32];
static UWORD s_pPaletteAtariBasic[32];

extern UBYTE creditsControl;

UBYTE printOnce = 0;

const char *s_pCreditsLines[CREDITS_LINE_COUNT] = {
    "ATARENIUM FALCON  v0.975 (RC)",
    "First release at Decrunch 2020",
    "gamedev compo (2nd place).",
    "",
    "Luc3k: code & MSX",
    "Softiron: GFX",
    "KaiN, Proxy & PCH: AFK",
    "",
    "Special credit goes to Saberman - ",
    "The Great Atariman of the Galaxy.",
    "",
    "Navigate your ship using",
    "WSAD, arrows or Joy.",
    "M - toggle music/sfx/mute in play.",
    "",
    "Look for 3-letter cheatcodes here ",
    "and there and try typing them in menu.", 
    "There is one for unlimited coal,", 
    "one for great-looking ship, and one",
    "for skipping levels by N in play.",
    "",
    "(Fire or Return to continue)"};

void stateCreditsCreate(void)
{

  s_pView = viewCreate(0,
                       TAG_VIEW_COPLIST_MODE, COPPER_MODE_BLOCK,
                       TAG_VIEW_GLOBAL_CLUT, 1,
                       TAG_END);

  s_pVp = vPortCreate(0,
                      TAG_VPORT_VIEW, s_pView,
                      TAG_VPORT_BPP, 5,
                      TAG_END);

  s_pVpManager = simpleBufferCreate(0,
                                    TAG_SIMPLEBUFFER_VPORT, s_pVp,
                                    TAG_SIMPLEBUFFER_BITMAP_FLAGS, BMF_CLEAR | BMF_INTERLEAVED,
                                    TAG_SIMPLEBUFFER_IS_DBLBUF, 0,
                                    TAG_END);

  systemUnuse();
  joyOpen();
  keyCreate();
  viewLoad(s_pView);
  ptplayerCreate(1);

  printOnce = 0;


  s_pFont = fontCreate("data/topaz.fnt");
  s_pBmText = fontCreateTextBitMap(300, s_pFont->uwHeight);
  paletteLoad("data/falkon.plt", s_pPaletteAtariBasic, 32);
 
}

void stateCreditsLoop(void)
{
  joyProcess();
  keyProcess();

    if (printOnce == 0)
    {
      ++printOnce;
      blitRect(s_pVpManager->pBack, 0, 0, 320, 128, 21);
      blitRect(s_pVpManager->pBack, 0, 128, 320, 128, 21);
      paletteDim(s_pPaletteAtariBasic, s_pVp->pPalette, 32, 15); // 0 - czarno, 15 - pe�na paleta
      viewUpdateCLUT(s_pView);
      
    

    for (BYTE i = 0; i < CREDITS_LINE_COUNT; ++i)
    {
      fontFillTextBitMap(s_pFont, s_pBmText, s_pCreditsLines[i]);
      fontDrawTextBitMap(s_pVpManager->pBack, s_pBmText, 10, (i * 9) + 10, 23, FONT_COOKIE);
    }
    
    }
  

  if (joyUse(JOY1_FIRE) || keyUse(KEY_RETURN)){
    //if (creditsControl == 0)
    //{
      //stateChange(g_pStateMachineGame, &g_sStateIntro);
      //return;
    //}
    //else if (creditsControl == 1)
    //{
      stateChange(g_pStateMachineGame, &g_sStateMenu);
      return;
    //}
  }

  viewProcessManagers(s_pView);
  copProcessBlocks();
  vPortWaitForEnd(s_pVp);
}

void stateCreditsDestroy(void)
{
  systemUse();

  fontDestroy(s_pFont);
  fontDestroyTextBitMap(s_pBmText);
  joyClose();
  keyDestroy();
  viewDestroy(s_pView);
  ptplayerDestroy();
}

tState g_sStateCredits = {
    .cbCreate = stateCreditsCreate,
    .cbLoop = stateCreditsLoop,
    .cbDestroy = stateCreditsDestroy,
    .cbSuspend = 0,
    .cbResume = 0,
    .pPrev = 0};
