#include <ace/managers/viewport/simplebuffer.h>
#include <ace/managers/joy.h>
#include <ace/managers/key.h>
#include <ace/managers/system.h>
#include <ace/managers/game.h>
#include <ace/utils/palette.h>
#include <ace/utils/font.h>
#include <stdio.h>
#include <ace/managers/rand.h>
#include <ace/managers/state.h>
#include <ace/utils/custom.h>
#include <ace/utils/ptplayer.h>
#include <ace/utils/file.h>

//------------------------------------------------------- gdzie� przed funkcjami
// zmienne trzymaj�ce adresy do viewa, viewporta, simple buffer managera
static tView *s_pView;
static tVPort *s_pVp;
static tSimpleBufferManager *s_pVpManager;
static tBitMap *s_pTiles;
static tBitMap *s_pTilesMask;
static tBitMap *s_pBg;
static tBitMap *s_pBgWithTile;
static tBitMap *s_pBgPortalGlow;
static tBitMap *s_pHUD;
static tBitMap *s_pFalconBg;
static tBitMap *s_pAnimBg;
static tBitMap *s_pRobbo;

static tPtplayerMod *s_pMod;

static tFont *s_pFont;
static tTextBitMap *s_pBmText;

static UWORD s_pPalette[32];

extern tStateManager *g_pStateMachineGame;
extern tState g_sStateMenu;
extern tState g_sStateGameOver;
extern tState g_sStateScore;
extern tState g_sStateGuruMastah;
extern tState g_sStateScoreAmi;

#define MAP_TILE_HEIGHT 7
#define MAP_TILE_WIDTH 10
#define FALCON_HEIGHT 32
#define ANIM_FRAME_COUNT 8

#define LAST_LEVEL_NUMBER 28

char szMsg[50];  // do wyswietlania wegla na HUD
char szMsg2[50]; // do wyswietlania kondkow na HUD
char szMsg3[50];
char szMsg4[50];
char levelFilePath[20];
char szLvl[50];

char szRobboMsg[80];
char *szRobbo1stLine = "ROBBO says:";

BYTE youWin = 0;

BYTE musicPlay = 1;

BYTE ubStoneImg = 0;

BYTE kamyki[10][7];
BYTE collectiblesAnim[10][7];

// coordsy do rysowania falkona i kontrolowania zeby sie nie wypierdolil za ekran
BYTE falkonx = 0;
BYTE falkony = 0;
BYTE krawedzx = 0;
BYTE krawedzy = 0;
BYTE kierunek = 0;
BYTE falkonFace = 0; // kierunek dziobem

UWORD pAnim[] = {0, 32, 64, 96, 128, 160, 192, 224};

UWORD uwPosX = 0;
UWORD uwPosY = 0;
UWORD uwPreviousX = 0;
UWORD uwPreviousY = 0;

BYTE stoneHit = 0;
BYTE frameHit = 0;

CONST BYTE startingCoal = 10;

BYTE falkonIdle = 0;
BYTE falkonIdleTempo = 8;
BYTE falkonIdleControl = 1;
BYTE redCapacitorsAnimTick = 0;
BYTE tickTempo = 8;
BYTE redCapacitorsAnimTileCheck = 0;
BYTE blueCapacitorsAnimTick = 0;
BYTE blueCapacitorsAnimTileCheck = 0;

BYTE portalGlowTick = 0;
BYTE portalTickTempo = 4;
BYTE portalGlowFrame = 0;

BYTE hudScrollingControl = 0;
BYTE hudScrollingTick = 0;

BYTE portalAnimControl = 0;
BYTE portalAnimTick = 0;
BYTE portalFrame = 0;
BYTE portalGlowX = 0;
BYTE portalGlowY = 0;
BYTE portalGlowDB  = 0;

BYTE stonehitAnimControl = 0;
BYTE stonehitAnimTick = 0;
BYTE stonehitAnimFrame = 0;
BYTE oneFrameDirection = 0;

BYTE flyingAnimControl = 0;
BYTE flyingTick = 0;
BYTE flyingFrame = 0;
UWORD newPosX = 0;
UWORD newPosY = 0;

BYTE coal = startingCoal;
BYTE capacitors = 0;
BYTE excesscoal = 0;
BYTE level = 16;

BYTE robboMsgNr = 0;
BYTE robboMsgCtrl = 0;
BYTE robboMsgCount = 0;
BYTE HUDfontColor = 23;

UBYTE doubleBufferFrameControl = 2;
UBYTE idleFrame = 0;
BYTE amigaMode = 0;
BYTE kierunekHold = 0;

UBYTE tempX = 0;
UBYTE tempY = 0;

extern UBYTE cheatmodeEnablerWhenEqual3;
extern UBYTE secondCheatEnablerWhenEqual3;

UBYTE audioFadeIn = 0;

void waitFrames(tVPort *pVPort, UBYTE ubHowMany, UWORD uwPosY)
{
  for (UBYTE i = 0; i < ubHowMany; ++i)
  {
    viewProcessManagers(pVPort->pView);
    copProcessBlocks();
    vPortWaitForEnd(pVPort);
  }
}

void clean();

void portalCloseAnim(void){\
  blitCopy(s_pBg, uwPosX, uwPosY, s_pFalconBg, 0, 0, 32, 32, MINTERM_COOKIE);
  blitCopy(s_pFalconBg, 0, 0, s_pVpManager->pBack, uwPosX, uwPosY, 32, 32, MINTERM_COOKIE);
  blitCopy(s_pFalconBg, 0, 0, s_pVpManager->pFront, uwPosX, uwPosY, 32, 32, MINTERM_COOKIE);
  for (UBYTE i = 0; i < 8; ++i) // zamykanie portala
  {
    blitCopy(s_pFalconBg, 0, 0, s_pVpManager->pBack, uwPosX, uwPosY, 32, 32, MINTERM_COOKIE);
    blitCopy(s_pFalconBg, 0, 0, s_pVpManager->pFront, uwPosX, uwPosY, 32, 32, MINTERM_COOKIE);
    blitCopyMask(s_pTiles, 224 - (32 * i), 320, s_pVpManager->pBack, falkonx * 32, falkony * 32, 32, 32, (UWORD *)s_pTilesMask->Planes[0]);
    blitCopyMask(s_pTiles, 224 - (32 * i), 320, s_pVpManager->pFront, falkonx * 32, falkony * 32, 32, 32, (UWORD *)s_pTilesMask->Planes[0]);
    //blitCopyMask(s_pTiles, 32, 192 + falkonFace, s_pVpManager->pBack, falkonx * 32, falkony * 32, 32, 32, (UWORD *)s_pTilesMask->Planes[0]);
    //blitCopyMask(s_pTiles, 32, 192 + falkonFace, s_pVpManager->pFront, falkonx * 32, falkony * 32, 32, 32, (UWORD *)s_pTilesMask->Planes[0]);

    waitFrames(s_pVp, falkonIdleTempo, uwPosY + FALCON_HEIGHT);
  }
}

void endLevelFadeOut(void)
{
  UBYTE bRatioGame = 15;

  for (UBYTE i = 0; i < 16; ++i)
  {
    if (musicPlay == 1)
    {
      ptplayerSetMasterVolume(bRatioGame * 4);
    }
    paletteDim(s_pPalette, s_pVp->pPalette, 32, bRatioGame); // 0 - czarno, 15 - pe?na paleta
    viewUpdateCLUT(s_pView);                                 // we? palet? z viewporta i wrzu? j? na ekran
    --bRatioGame;
    waitFrames(s_pVp, 10, uwPosY + FALCON_HEIGHT);
  }
}

void amiHUDprintOnFrontOnceAfterLoad(void)
{

  blitCopy(s_pHUD, 32, 0, s_pVpManager->pFront, 32, 224, 32, 32, MINTERM_COOKIE);
  sprintf(szMsg, "%d", coal);
  fontFillTextBitMap(s_pFont, s_pBmText, szMsg);
  fontDrawTextBitMap(s_pVpManager->pFront, s_pBmText, 42, 232, HUDfontColor, FONT_COOKIE);
  blitCopy(s_pHUD, 188, 0, s_pVpManager->pFront, 188, 224, 32, 32, MINTERM_COOKIE);
  sprintf(szMsg2, "%d", capacitors);
  fontFillTextBitMap(s_pFont, s_pBmText, szMsg2);
  fontDrawTextBitMap(s_pVpManager->pFront, s_pBmText, 190, 236, HUDfontColor, FONT_COOKIE);
  blitCopy(s_pHUD, 128, 0, s_pVpManager->pFront, 128, 224, 32, 32, MINTERM_COOKIE);
  sprintf(szMsg3, "%d", excesscoal);
  fontFillTextBitMap(s_pFont, s_pBmText, szMsg3);
  fontDrawTextBitMap(s_pVpManager->pFront, s_pBmText, 130, 232, HUDfontColor, FONT_COOKIE);
  blitCopy(s_pHUD, 248, 0, s_pVpManager->pFront, 248, 224, 32, 32, MINTERM_COOKIE);
  sprintf(szMsg4, "%d", robboMsgCount);
  fontFillTextBitMap(s_pFont, s_pBmText, szMsg4);
  fontDrawTextBitMap(s_pVpManager->pFront, s_pBmText, 250, 236, HUDfontColor, FONT_COOKIE);
  blitCopy(s_pHUD, 288, 0, s_pVpManager->pFront, 288, 224, 32, 32, MINTERM_COOKIE);
  sprintf(szLvl, "%d", level);
  fontFillTextBitMap(s_pFont, s_pBmText, szLvl);
  fontDrawTextBitMap(s_pVpManager->pFront, s_pBmText, 290, 236, HUDfontColor, FONT_COOKIE);
}

void printOnHUD(void)
{

  blitCopy(s_pHUD, 32, 0, s_pVpManager->pBack, 32, 224, 32, 32, MINTERM_COOKIE);
  sprintf(szMsg, "%d", coal);
  fontFillTextBitMap(s_pFont, s_pBmText, szMsg);
  fontDrawTextBitMap(s_pVpManager->pBack, s_pBmText, 42, 232, HUDfontColor, FONT_COOKIE);
  blitCopy(s_pHUD, 188, 0, s_pVpManager->pBack, 188, 224, 32, 32, MINTERM_COOKIE);
  sprintf(szMsg2, "%d", capacitors);
  fontFillTextBitMap(s_pFont, s_pBmText, szMsg2);
  fontDrawTextBitMap(s_pVpManager->pBack, s_pBmText, 190, 236, HUDfontColor, FONT_COOKIE);
  blitCopy(s_pHUD, 128, 0, s_pVpManager->pBack, 128, 224, 32, 32, MINTERM_COOKIE);
  sprintf(szMsg3, "%d", excesscoal);
  fontFillTextBitMap(s_pFont, s_pBmText, szMsg3);
  fontDrawTextBitMap(s_pVpManager->pBack, s_pBmText, 130, 232, HUDfontColor, FONT_COOKIE);
  blitCopy(s_pHUD, 248, 0, s_pVpManager->pBack, 248, 224, 32, 32, MINTERM_COOKIE);
  sprintf(szMsg4, "%d", robboMsgCount);
  fontFillTextBitMap(s_pFont, s_pBmText, szMsg4);
  fontDrawTextBitMap(s_pVpManager->pBack, s_pBmText, 250, 236, HUDfontColor, FONT_COOKIE);
  blitCopy(s_pHUD, 288, 0, s_pVpManager->pBack, 288, 224, 32, 32, MINTERM_COOKIE);
  sprintf(szLvl, "%d", level);
  fontFillTextBitMap(s_pFont, s_pBmText, szLvl);
  fontDrawTextBitMap(s_pVpManager->pBack, s_pBmText, 290, 236, HUDfontColor, FONT_COOKIE);
}

void gameOnResume(void)
{
  viewLoad(s_pView);
}

void drawTiles(void)
{

  sprintf(levelFilePath, "data/%d.txt", level);
  systemUse();
  tFile *levelFile = fileOpen(levelFilePath, "r");
  BYTE ubZmienna = 0;
  BYTE x = 0;
  BYTE y = 0;

  falkonx = 0;
  falkony = 0;
  krawedzx = 0;
  krawedzy = 0;
  kierunek = 0;

  for (BYTE i = 0; i < 82; ++i)
  {
    fileRead(levelFile, &ubZmienna, 1);

    if (ubZmienna == 0x30)
    {
      kamyki[x][y] = 0;
      blitCopy(s_pBg, x * 32, y * 32, s_pVpManager->pBack, x * 32, y * 32, 32, 32, MINTERM_COPY);
      blitCopy(s_pBg, x * 32, y * 32, s_pVpManager->pFront, x * 32, y * 32, 32, 32, MINTERM_COPY);
    }

    else if (ubZmienna == 0x33)
    {
      kamyki[x][y] = 3;
      ubStoneImg = ulRandMinMax(0, 2);
      blitCopy(s_pBg, x * 32, y * 32, s_pVpManager->pBack, x * 32, y * 32, 32, 32, MINTERM_COPY);
      blitCopy(s_pBg, x * 32, y * 32, s_pVpManager->pFront, x * 32, y * 32, 32, 32, MINTERM_COPY);
      blitCopyMask(s_pTiles, ubStoneImg * 32, 0, s_pVpManager->pBack, x * 32, y * 32, 32, 32, (UWORD *)s_pTilesMask->Planes[0]);
      blitCopyMask(s_pTiles, ubStoneImg * 32, 0, s_pVpManager->pFront, x * 32, y * 32, 32, 32, (UWORD *)s_pTilesMask->Planes[0]);
    }
    else if (ubZmienna == 0x34)
    {
      kamyki[x][y] = 4;
      blitCopyMask(s_pTiles, 96, 0, s_pBgWithTile, x * 32, y * 32, 32, 32, (UWORD *)s_pTilesMask->Planes[0]);
      blitCopyMask(s_pTiles, 96, 0, s_pBgWithTile, x * 32, y * 32, 32, 32, (UWORD *)s_pTilesMask->Planes[0]);

      blitCopy(s_pBgWithTile, x * 32, y * 32, s_pVpManager->pBack, x * 32, y * 32, 32, 32, MINTERM_COPY);
      blitCopy(s_pBgWithTile, x * 32, y * 32, s_pVpManager->pFront, x * 32, y * 32, 32, 32, MINTERM_COPY);
    }

    else if (ubZmienna == 0x35)
    {
      kamyki[x][y] = 5;
      blitCopyMask(s_pTiles, 128, 0, s_pBgWithTile, x * 32, y * 32, 32, 32, (UWORD *)s_pTilesMask->Planes[0]);
      blitCopyMask(s_pTiles, 128, 0, s_pBgWithTile, x * 32, y * 32, 32, 32, (UWORD *)s_pTilesMask->Planes[0]);

      blitCopy(s_pBgWithTile, x * 32, y * 32, s_pVpManager->pBack, x * 32, y * 32, 32, 32, MINTERM_COPY);
      blitCopy(s_pBgWithTile, x * 32, y * 32, s_pVpManager->pFront, x * 32, y * 32, 32, 32, MINTERM_COPY);
    }

    else if (ubZmienna == 0x36)
    {
      kamyki[x][y] = 6;
      blitCopyMask(s_pTiles, 160, 0, s_pBgWithTile, x * 32, y * 32, 32, 32, (UWORD *)s_pTilesMask->Planes[0]);
      blitCopyMask(s_pTiles, 160, 0, s_pBgWithTile, x * 32, y * 32, 32, 32, (UWORD *)s_pTilesMask->Planes[0]);

      blitCopy(s_pBgWithTile, x * 32, y * 32, s_pVpManager->pBack, x * 32, y * 32, 32, 32, MINTERM_COPY);
      blitCopy(s_pBgWithTile, x * 32, y * 32, s_pVpManager->pFront, x * 32, y * 32, 32, 32, MINTERM_COPY);
    }
    else if (ubZmienna == 0x37)
    {
      kamyki[x][y] = 7;
      blitCopyMask(s_pTiles, 192, 0, s_pBgWithTile, x * 32, y * 32, 32, 32, (UWORD *)s_pTilesMask->Planes[0]);
      blitCopyMask(s_pTiles, 192, 0, s_pBgWithTile, x * 32, y * 32, 32, 32, (UWORD *)s_pTilesMask->Planes[0]);

      blitCopy(s_pBgWithTile, x * 32, y * 32, s_pVpManager->pBack, x * 32, y * 32, 32, 32, MINTERM_COPY);
      blitCopy(s_pBgWithTile, x * 32, y * 32, s_pVpManager->pFront, x * 32, y * 32, 32, 32, MINTERM_COPY);
    }
    else if (ubZmienna == 0x38)
    {
      kamyki[x][y] = 8;
      collectiblesAnim[x][y] = 8;
      blitCopyMask(s_pTiles, 0, 256, s_pBgWithTile, x * 32, y * 32, 32, 32, (UWORD *)s_pTilesMask->Planes[0]);
      blitCopyMask(s_pTiles, 0, 256, s_pBgWithTile, x * 32, y * 32, 32, 32, (UWORD *)s_pTilesMask->Planes[0]);

      blitCopy(s_pBgWithTile, x * 32, y * 32, s_pVpManager->pBack, x * 32, y * 32, 32, 32, MINTERM_COPY);
      blitCopy(s_pBgWithTile, x * 32, y * 32, s_pVpManager->pFront, x * 32, y * 32, 32, 32, MINTERM_COPY);
    }
    else if (ubZmienna == 0x39)
    {
      kamyki[x][y] = 9;
      collectiblesAnim[x][y] = 9;
      blitCopyMask(s_pTiles, 0, 288, s_pBgWithTile, x * 32, y * 32, 32, 32, (UWORD *)s_pTilesMask->Planes[0]);
      blitCopyMask(s_pTiles, 0, 288, s_pBgWithTile, x * 32, y * 32, 32, 32, (UWORD *)s_pTilesMask->Planes[0]);
      blitCopy(s_pBgWithTile, x * 32, y * 32, s_pVpManager->pBack, x * 32, y * 32, 32, 32, MINTERM_COPY);
      blitCopy(s_pBgWithTile, x * 32, y * 32, s_pVpManager->pFront, x * 32, y * 32, 32, 32, MINTERM_COPY);
    }
    else if (ubZmienna == 0x45)
    {
      kamyki[x][y] = 10;
      portalGlowX = x;
      portalGlowY = y;
      blitCopyMask(s_pTiles, 64, 32, s_pBgWithTile, x * 32, y * 32, 32, 32, (UWORD *)s_pTilesMask->Planes[0]);
      blitCopyMask(s_pTiles, 64, 32, s_pBgWithTile, x * 32, y * 32, 32, 32, (UWORD *)s_pTilesMask->Planes[0]);

      blitCopy(s_pBgWithTile, x * 32, y * 32, s_pVpManager->pBack, x * 32, y * 32, 32, 32, MINTERM_COPY);
      blitCopy(s_pBgWithTile, x * 32, y * 32, s_pVpManager->pFront, x * 32, y * 32, 32, 32, MINTERM_COPY);
    }
    else if (ubZmienna == 0x52)
    {
      kamyki[x][y] = 11;
      blitCopyMask(s_pTiles, 96, 32, s_pBgWithTile, x * 32, y * 32, 32, 32, (UWORD *)s_pTilesMask->Planes[0]);
      blitCopyMask(s_pTiles, 96, 32, s_pBgWithTile, x * 32, y * 32, 32, 32, (UWORD *)s_pTilesMask->Planes[0]);

      blitCopy(s_pBgWithTile, x * 32, y * 32, s_pVpManager->pBack, x * 32, y * 32, 32, 32, MINTERM_COPY);
      blitCopy(s_pBgWithTile, x * 32, y * 32, s_pVpManager->pFront, x * 32, y * 32, 32, 32, MINTERM_COPY);
    }
    else if (ubZmienna == 0x42)
    {
      kamyki[x][y] = 12;
      blitCopyMask(s_pTiles, 128, 32, s_pBgWithTile, x * 32, y * 32, 32, 32, (UWORD *)s_pTilesMask->Planes[0]);
      blitCopyMask(s_pTiles, 128, 32, s_pBgWithTile, x * 32, y * 32, 32, 32, (UWORD *)s_pTilesMask->Planes[0]);

      blitCopy(s_pBgWithTile, x * 32, y * 32, s_pVpManager->pBack, x * 32, y * 32, 32, 32, MINTERM_COPY);
      blitCopy(s_pBgWithTile, x * 32, y * 32, s_pVpManager->pFront, x * 32, y * 32, 32, 32, MINTERM_COPY);
    }
    else if (ubZmienna == 0x31)
    {
      kamyki[x][y] = 1;
      falkonx = x;
      falkony = y;
      krawedzx = x;
      krawedzy = y;
      uwPosX = falkonx * 32;
      uwPosY = falkony * 32;
      tempX = falkonx;
      tempY = falkony;
      //if (falkonFace == 0)
      //{
      //  blitCopyMask(s_pTiles, 128, 32, s_pVpManager->pBack, x * 32, y * 32, 32, 32, (UWORD *)s_pTilesMask->Planes[0]);
      //}
      //else if (falkonFace == 32)
      //{
      //  blitCopyMask(s_pTiles, 160, 32, s_pVpManager->pBack, x * 32, y * 32, 32, 32, (UWORD *)s_pTilesMask->Planes[0]);
      //}
    }

    ++x;
    if (ubZmienna == 0x0a)
    {
      x = 0;
      ++y;
    }
  }
  fileClose(levelFile);
  systemUnuse();
  paletteDim(s_pPalette, s_pVp->pPalette, 32, 15);
  viewUpdateCLUT(s_pView);
}

void clearTiles(void)
{
  blitCopy(s_pBg, 0, 0, s_pBgWithTile, 0, 0, 320, 128, MINTERM_COPY);
  blitCopy(s_pBg, 0, 128, s_pBgWithTile, 0, 128, 320, 96, MINTERM_COPY);
  for (UBYTE y = 0; y < MAP_TILE_HEIGHT; ++y)
  {
    for (UBYTE x = 0; x < MAP_TILE_WIDTH; ++x)
    {
      kamyki[x][y] = 0;
      collectiblesAnim[x][y] = 0;
    }
  }
}

void levelScore(void)
{
  blitCopy(s_pBg, uwPosX, uwPosY, s_pFalconBg, 0, 0, 32, 32, MINTERM_COOKIE);
  blitCopy(s_pFalconBg, 0, 0, s_pVpManager->pBack, uwPosX, uwPosY, 32, 32, MINTERM_COOKIE);
  blitCopy(s_pFalconBg, 0, 0, s_pVpManager->pFront, uwPosX, uwPosY, 32, 32, MINTERM_COOKIE);
  blitCopyMask(s_pTiles, 32, 192 + falkonFace, s_pVpManager->pBack, falkonx * 32, falkony * 32, 32, 32, (UWORD *)s_pTilesMask->Planes[0]);
  blitCopyMask(s_pTiles, 32, 192 + falkonFace, s_pVpManager->pFront, falkonx * 32, falkony * 32, 32, 32, (UWORD *)s_pTilesMask->Planes[0]);

  BYTE thisLevelExcessCoal = coal - 1;
  for (UBYTE i = 0; i < thisLevelExcessCoal; ++i)
  {
    --coal;
    ++excesscoal;
    blitCopy(s_pHUD, 32, 0, s_pVpManager->pBack, 32, 224, 32, 32, MINTERM_COOKIE);
    blitCopy(s_pHUD, 32, 0, s_pVpManager->pFront, 32, 224, 32, 32, MINTERM_COOKIE);
    sprintf(szMsg, "%d", coal);
    fontFillTextBitMap(s_pFont, s_pBmText, szMsg);
    fontDrawTextBitMap(s_pVpManager->pBack, s_pBmText, 42, 231, HUDfontColor, FONT_COOKIE);
    fontDrawTextBitMap(s_pVpManager->pFront, s_pBmText, 42, 231, HUDfontColor, FONT_COOKIE);
    blitCopy(s_pHUD, 128, 0, s_pVpManager->pBack, 128, 224, 32, 32, MINTERM_COOKIE);
    blitCopy(s_pHUD, 128, 0, s_pVpManager->pFront, 128, 224, 32, 32, MINTERM_COOKIE);
    sprintf(szMsg3, "%d", excesscoal);
    fontFillTextBitMap(s_pFont, s_pBmText, szMsg3);
    fontDrawTextBitMap(s_pVpManager->pBack, s_pBmText, 130, 232, HUDfontColor, FONT_COOKIE);
    fontDrawTextBitMap(s_pVpManager->pFront, s_pBmText, 130, 232, HUDfontColor, FONT_COOKIE);
    waitFrames(s_pVp, 20, uwPosY + FALCON_HEIGHT);
  }

  for (UBYTE i = 0; i < 8; ++i) // otwieranie portala
  {
    blitCopy(s_pFalconBg, 0, 0, s_pVpManager->pBack, uwPosX, uwPosY, 32, 32, MINTERM_COOKIE);
    blitCopy(s_pFalconBg, 0, 0, s_pVpManager->pFront, uwPosX, uwPosY, 32, 32, MINTERM_COOKIE);
    blitCopyMask(s_pTiles, 32 * i, 320, s_pVpManager->pBack, falkonx * 32, falkony * 32, 32, 32, (UWORD *)s_pTilesMask->Planes[0]);
    blitCopyMask(s_pTiles, 32 * i, 320, s_pVpManager->pFront, falkonx * 32, falkony * 32, 32, 32, (UWORD *)s_pTilesMask->Planes[0]);
    blitCopyMask(s_pTiles, 32, 192 + falkonFace, s_pVpManager->pBack, falkonx * 32, falkony * 32, 32, 32, (UWORD *)s_pTilesMask->Planes[0]);
    blitCopyMask(s_pTiles, 32, 192 + falkonFace, s_pVpManager->pFront, falkonx * 32, falkony * 32, 32, 32, (UWORD *)s_pTilesMask->Planes[0]);

    waitFrames(s_pVp, falkonIdleTempo, uwPosY + FALCON_HEIGHT);
  }
}

void nextLevel(void)
{
  coal = 1;
  audioFadeIn = 0;

  switch (level)
  {

  case 4:
    bitmapDestroy(s_pBg);
    bitmapDestroy(s_pBgWithTile);
    s_pBg = bitmapCreateFromFile("data/tlo2.bm", 0);
    s_pBgWithTile = bitmapCreateFromFile("data/tlo2.bm", 0);
    break;

  case 8:
    bitmapDestroy(s_pBg);
    bitmapDestroy(s_pBgWithTile);
    s_pBg = bitmapCreateFromFile("data/tlo3.bm", 0);
    s_pBgWithTile = bitmapCreateFromFile("data/tlo3.bm", 0);
    break;

  case 12:
    bitmapDestroy(s_pBg);
    bitmapDestroy(s_pBgWithTile);
    s_pBg = bitmapCreateFromFile("data/tlo4.bm", 0);
    s_pBgWithTile = bitmapCreateFromFile("data/tlo4.bm", 0);
    break;

  case 17:
    bitmapDestroy(s_pBg);
    bitmapDestroy(s_pBgWithTile);
    s_pBg = bitmapCreateFromFile("data/tlo5.bm", 0);
    s_pBgWithTile = bitmapCreateFromFile("data/tlo5.bm", 0);
    break;
  case 22:
    bitmapDestroy(s_pBg);
    bitmapDestroy(s_pBgWithTile);
    s_pBg = bitmapCreateFromFile("data/tlo6.bm", 0);
    s_pBgWithTile = bitmapCreateFromFile("data/tlo6.bm", 0);
    break;

  case LAST_LEVEL_NUMBER - 1: // ta ma byc przedostatnia

    robboMsgNr = LAST_LEVEL_NUMBER - 1;
    break;

  case LAST_LEVEL_NUMBER: // ta ma byc ostatnia

    robboMsgNr = LAST_LEVEL_NUMBER;
    break;
  }
  clearTiles();
  blitCopy(s_pHUD, 0, 0, s_pVpManager->pBack, 0, 224, 320, 32, MINTERM_COPY);
  blitCopy(s_pHUD, 0, 0, s_pVpManager->pFront, 0, 224, 320, 32, MINTERM_COPY);
  printOnHUD();
  doubleBufferFrameControl = 2;
  drawTiles();
}

void czyRamka(void)
{
  // tu jest funkcja sprawdzajaca czy sie chcemy wypierdolic za ekran i nie pozwalajaca na to
  switch (kierunek)
  {
  case 1:
    krawedzx = krawedzx + 1;
    if (krawedzx == 10)
    {
      krawedzx = 9;
      falkonx = 9;
      frameHit = 1;
    }
    break;
  case 2:
    krawedzx = krawedzx - 1;
    if (krawedzx == -1)
    {
      krawedzx = 0;
      falkonx = 0;
      frameHit = 1;
    }
    break;
  case 3:
    krawedzy = krawedzy - 1;
    if (krawedzy == -1)
    {
      krawedzy = 0;
      falkony = 0;
      frameHit = 1;
    }
    break;
  case 4:
    krawedzy = krawedzy + 1;
    if (krawedzy == 7)
    {
      krawedzy = 6;
      falkony = 6;
      frameHit = 1;
    }
    break;
  }
}

void isThisStone(void)
{
  // funkcja sprawdzajaca przed wykonaniem ruchu czy chcemy wleciec w kamien

  BYTE stoneX = 0;
  BYTE stoneY = 0;

  switch (kierunek)
  {
  case 1:
    stoneX = falkonx + 1;
    if (kamyki[stoneX][falkony] == 3)
    {
      stoneHit = 1;
    }
    break;
  case 2:
    stoneX = falkonx - 1;
    if (kamyki[stoneX][falkony] == 3)
    {
      stoneHit = 1;
    }
    break;
  case 3:
    stoneY = falkony - 1;
    if (kamyki[falkonx][stoneY] == 3)
    {
      stoneHit = 1;
    }
    break;
  case 4:
    stoneY = falkony + 1;
    if (kamyki[falkonx][stoneY] == 3)
    {
      stoneHit = 1;
    }
    break;
  }
}

void portalAnim(void)
{
  if (portalAnimControl != 1)
  {
    return;
  }

  uwPosX = falkonx * 32;
  uwPosY = falkony * 32;

  if (portalAnimControl == 1)
  {

    if (portalAnimTick == falkonIdleTempo * 1)
    {
      portalFrame = 0;
    }
    else if (portalAnimTick == falkonIdleTempo * 2)
    {
      portalFrame = 1;
    }
    else if (portalAnimTick == falkonIdleTempo * 3)
    {
      portalFrame = 2;
    }
    else if (portalAnimTick == falkonIdleTempo * 4)
    {
      portalFrame = 3;
    }
    else if (portalAnimTick == falkonIdleTempo * 5)
    {
      portalFrame = 4;
    }
    else if (portalAnimTick == falkonIdleTempo * 6)
    {
      portalFrame = 5;
    }
    else if (portalAnimTick == falkonIdleTempo * 7)
    {
      portalFrame = 6;
    }
    else if (portalAnimTick == falkonIdleTempo * 8)
    {
      portalFrame = 7;
    }
    else if (portalAnimTick == falkonIdleTempo * 9)
    {
      portalFrame = 8;
    }
  }
  if (portalFrame == 8)
  {
    portalCloseAnim();
    endLevelFadeOut();
    portalFrame = 0;
    portalAnimTick = 0;
    portalAnimControl = 0;
    ++level;
    if (level == LAST_LEVEL_NUMBER + 1)
    {
      ptplayerStop();
      youWin = 1;
    }
    else
    {
      nextLevel();
    }
  }
  blitCopy(s_pBg, uwPosX, uwPosY, s_pVpManager->pBack, uwPosX, uwPosY, 32, 32, MINTERM_COOKIE);
  blitCopyMask(s_pTiles, portalFrame * 32, 128 + falkonFace, s_pVpManager->pBack, uwPosX, uwPosY, 32, 32, (UWORD *)s_pTilesMask->Planes[0]);
}

void robboScrollUp(void)
{
  if (robboMsgCtrl != 1)
  {
    return;
  }
  doubleBufferFrameControl = 2;

  if (hudScrollingControl == 1)
  {
    if (hudScrollingTick == 0 || hudScrollingTick == 1)
    {
      blitCopy(s_pRobbo, 0, 0, s_pVpManager->pBack, 0, 248, 320, 8, MINTERM_COOKIE);
    }
    else if (hudScrollingTick == 4 || hudScrollingTick == 5)
    {
      blitCopy(s_pRobbo, 0, 0, s_pVpManager->pBack, 0, 240, 320, 16, MINTERM_COOKIE);
    }
    else if (hudScrollingTick == 8 || hudScrollingTick == 9)
    {
      blitCopy(s_pRobbo, 0, 0, s_pVpManager->pBack, 0, 232, 320, 24, MINTERM_COOKIE);
    }
    else if (hudScrollingTick == 12 || hudScrollingTick == 13)
    {
      blitCopy(s_pRobbo, 0, 0, s_pVpManager->pBack, 0, 224, 320, 32, MINTERM_COOKIE);
    }
    else if (hudScrollingTick == 16)
    {
      blitCopy(s_pRobbo, 0, 0, s_pVpManager->pBack, 0, 224, 320, 32, MINTERM_COOKIE);
      robboMsgCtrl = 3;
      hudScrollingControl = 0;
      hudScrollingTick = 0;
    }
  }
}

void robboScrollDown(void)
{
  if (robboMsgCtrl != 2)
  {
    return;
  }
  doubleBufferFrameControl = 2;
  if (hudScrollingControl == 1)
  {

    if (hudScrollingTick == 0 || hudScrollingTick == 1)
    {
      blitCopy(s_pHUD, 0, 0, s_pVpManager->pBack, 0, 224, 320, 8, MINTERM_COOKIE);
      blitCopy(s_pRobbo, 0, 0, s_pVpManager->pBack, 0, 232, 320, 24, MINTERM_COOKIE);
    }
    else if (hudScrollingTick == 4 || hudScrollingTick == 5)
    {
      blitCopy(s_pHUD, 0, 0, s_pVpManager->pBack, 0, 224, 320, 16, MINTERM_COOKIE);
      blitCopy(s_pRobbo, 0, 0, s_pVpManager->pBack, 0, 240, 320, 16, MINTERM_COOKIE);
    }
    else if (hudScrollingTick == 8 || hudScrollingTick == 9)
    {
      blitCopy(s_pHUD, 0, 0, s_pVpManager->pBack, 0, 224, 320, 32, MINTERM_COOKIE);
      blitCopy(s_pRobbo, 0, 0, s_pVpManager->pBack, 0, 248, 320, 8, MINTERM_COOKIE);
    }
    else if (hudScrollingTick == 12 || hudScrollingTick == 13)
    {
      blitCopy(s_pHUD, 0, 0, s_pVpManager->pBack, 0, 224, 320, 32, MINTERM_COOKIE);
    }
    else if (hudScrollingTick == 16)
    {
      blitCopy(s_pHUD, 0, 0, s_pVpManager->pBack, 0, 224, 320, 32, MINTERM_COOKIE);
      robboMsgCtrl = 0;
      hudScrollingTick = 0;
      hudScrollingControl = 0;
      printOnHUD();
    }
  }
}

void robboSays(void)
{
  if (amigaMode == 2)
  {
    sprintf(szRobboMsg, "Traitor! Burn in hell!");
  }

  else if (amigaMode != 2)
  {
    switch (robboMsgNr)
    {
    case 1:
      sprintf(szRobboMsg, "Follow the Atari portal.");
      break;
    case 2:
      sprintf(szRobboMsg, "Keep an eye on your coal supplies.");
      break;
    case 3:
      sprintf(szRobboMsg, "Don't waste our coal hitting the meteorites.");
      break;
    case 4:
      sprintf(szRobboMsg, "Try to steal some red and blue capacitors.");
      break;
    case 5:
      sprintf(szRobboMsg, "Infiltrate the Amigans territory.");
      break;
    case 6:
      sprintf(szRobboMsg, "Minister Renton is counting on you, Sir.");
      break;
    case 7:
      sprintf(szRobboMsg, "Please clean up here, I found some GermZ.");
      break;
    case 8:
      sprintf(szRobboMsg, "Take me home, this place sucks!");
      break;
    case 9:
      sprintf(szRobboMsg, "Find the coal warehouse and reclaim it.");
      break;

    case 10:
      sprintf(szRobboMsg, "Have you played Aminer yet?");
      break;

    case 11:
      sprintf(szRobboMsg, "Drop me out at LK Avalon please.");
      break;

    case 12:
      sprintf(szRobboMsg, "You like Paula? I love Laura!");
      break;

    case 13:
      sprintf(szRobboMsg, "Atari has no glitches...");
      break;

    case 14:
      sprintf(szRobboMsg, "..even if played in 2077.");
      break;
    case 15:
      sprintf(szRobboMsg, "Make River Raid not Bridge Strike!");
      break;
    case 16:
      sprintf(szRobboMsg, "Cytadela is better than Doom.");
      break;
    case 17:
      sprintf(szRobboMsg, "How to double the value of your Atari...?");
      break;
    case 18:
      sprintf(szRobboMsg, "...Just insert the cassette into tape drive.");
      break;

    case LAST_LEVEL_NUMBER - 1:
      sprintf(szRobboMsg, "We're close, I feel it in my DSP.");
      break;
    case LAST_LEVEL_NUMBER:
      sprintf(szRobboMsg, "Well done! Now collect the coal and GTFO !!!");
      break;
    }
  }
  fontFillTextBitMap(s_pFont, s_pBmText, szRobbo1stLine);
  fontDrawTextBitMap(s_pVpManager->pBack, s_pBmText, 8, 230, 23, FONT_COOKIE);
  fontFillTextBitMap(s_pFont, s_pBmText, szRobboMsg);
  fontDrawTextBitMap(s_pVpManager->pBack, s_pBmText, 8, 240, 23, FONT_COOKIE);
}

void coalAndCollect(void)
{
  //funkcja do zbierania zasobu jesli jest na danym tajlu

  BYTE pickSthX = 0;
  BYTE pickSthY = 0;

  pickSthX = falkonx;
  pickSthY = falkony;

  BYTE what = kamyki[pickSthX][pickSthY];
  kamyki[pickSthX][pickSthY] = 0;
  collectiblesAnim[pickSthX][pickSthY] = 0;

  if (secondCheatEnablerWhenEqual3 == 3)
  {
    ++coal;
  }

  --coal;

  switch (what)
  {
  case 4:
    coal = coal + 2;
    break;

  case 5:
    coal = coal + 3;
    break;

  case 6:
    coal = coal + 4;
    break;

  case 7:
    coal = coal + 5;
    break;

  case 8:
    capacitors = capacitors + 2;
    blitCopy(s_pHUD, 188, 0, s_pVpManager->pBack, 188, 224, 32, 32, MINTERM_COOKIE);
    sprintf(szMsg2, "%d", capacitors);
    fontFillTextBitMap(s_pFont, s_pBmText, szMsg2);
    fontDrawTextBitMap(s_pVpManager->pBack, s_pBmText, 190, 236, HUDfontColor, FONT_COOKIE);
    break;

  case 9:
    capacitors = capacitors + 4;
    blitCopy(s_pHUD, 188, 0, s_pVpManager->pBack, 188, 224, 32, 32, MINTERM_COOKIE);
    sprintf(szMsg2, "%d", capacitors);
    fontFillTextBitMap(s_pFont, s_pBmText, szMsg2);
    fontDrawTextBitMap(s_pVpManager->pBack, s_pBmText, 190, 236, HUDfontColor, FONT_COOKIE);
    break;

  case 10:
    levelScore();
    portalAnimControl = 1;
    break;

  case 11:
    ++robboMsgNr;
    ++robboMsgCount;
    robboMsgCtrl = 1;
    hudScrollingControl = 1;
    break;

  case 12:
    amigaMode = 1;
    portalAnimControl = 0;
    ptplayerSetMasterVolume(0);
    bitmapDestroy(s_pTiles);
    bitmapDestroy(s_pTilesMask);
    s_pTiles = bitmapCreateFromFile("data/tileset2.bm", 0);
    s_pTilesMask = bitmapCreateFromFile("data/tileset_mask2.bm", 0);
    bitmapDestroy(s_pHUD);
    s_pHUD = bitmapCreateFromFile("data/amiHUD.bm", 0);
    blitCopy(s_pHUD, 0, 0, s_pVpManager->pBack, 0, 224, 320, 32, MINTERM_COOKIE);
    blitCopy(s_pHUD, 0, 0, s_pVpManager->pFront, 0, 224, 320, 32, MINTERM_COOKIE);
    blitCopy(s_pBg, 288, 0, s_pVpManager->pBack, 288, 0, 32, 32, MINTERM_COPY);
    blitCopy(s_pBg, 288, 0, s_pVpManager->pFront, 288, 0, 32, 32, MINTERM_COPY);
    blitCopyMask(s_pTiles, 64, 32, s_pBgWithTile,  288, 0, 32, 32, (UWORD *)s_pTilesMask->Planes[0]);
    blitCopyMask(s_pTiles, 64, 32, s_pBgWithTile, 288, 0, 32, 32, (UWORD *)s_pTilesMask->Planes[0]);
    blitCopy(s_pBgWithTile, 288, 0, s_pVpManager->pBack, 288, 0, 32, 32, MINTERM_COPY);
    blitCopy(s_pBgWithTile, 288, 0, s_pVpManager->pFront, 288, 0, 32, 32, MINTERM_COPY);
    amiHUDprintOnFrontOnceAfterLoad();
    printOnHUD();
    break;
  }
  printOnHUD();
}

void falkonHittingStone(void)
{
  if (stonehitAnimControl != 1)
  {
    return;
  }

  UWORD uwPosX = falkonx * 32;
  UWORD uwPosY = falkony * 32;

  if (stonehitAnimControl == 1)
  {
    if (stonehitAnimTick == falkonIdleTempo * 1)
    {
      stonehitAnimFrame = 0;
      if (oneFrameDirection == 1)
      {
        ++uwPosX;
      }
      else if (oneFrameDirection == 2)
      {
        --uwPosX;
      }
      else if (oneFrameDirection == 3)
      {
        --uwPosY;
      }
      else if (oneFrameDirection == 4)
      {
        ++uwPosY;
      }
    }
    else if (stonehitAnimTick == falkonIdleTempo * 2)
    {
      stonehitAnimFrame = 1;
    }
    else if (stonehitAnimTick == falkonIdleTempo * 3)
    {
      stonehitAnimFrame = 2;
      if (oneFrameDirection == 1)
      {
        ++uwPosX;
      }
      else if (oneFrameDirection == 2)
      {
        --uwPosX;
      }
      else if (oneFrameDirection == 3)
      {
        --uwPosY;
      }
      else if (oneFrameDirection == 4)
      {
        ++uwPosY;
      }
    }
    else if (stonehitAnimTick == falkonIdleTempo * 4)
    {
      stonehitAnimFrame = 3;
    }
    else if (stonehitAnimTick == falkonIdleTempo * 5)
    {
      stonehitAnimFrame = 4;
      if (oneFrameDirection == 1)
      {
        ++uwPosX;
      }
      else if (oneFrameDirection == 2)
      {
        --uwPosX;
      }
      else if (oneFrameDirection == 3)
      {
        --uwPosY;
      }
      else if (oneFrameDirection == 4)
      {
        ++uwPosY;
      }
    }
    else if (stonehitAnimTick == falkonIdleTempo * 6)
    {
      stonehitAnimFrame = 5;
    }
    else if (stonehitAnimTick == falkonIdleTempo * 7)
    {
      stonehitAnimFrame = 6;
      if (oneFrameDirection == 1)
      {
        ++uwPosX;
      }
      else if (oneFrameDirection == 2)
      {
        --uwPosX;
      }
      else if (oneFrameDirection == 3)
      {
        --uwPosY;
      }
      else if (oneFrameDirection == 4)
      {
        ++uwPosY;
      }
    }
    else if (stonehitAnimTick == falkonIdleTempo * 8)
    {
      stonehitAnimFrame = 7;
      stonehitAnimTick = 0;
      stonehitAnimControl = 0;
      falkonIdleControl = 1;
    }

    blitCopy(s_pBg, uwPosX, uwPosY, s_pFalconBg, 0, 0, 32, 32, MINTERM_COOKIE);
    blitCopy(s_pFalconBg, 0, 0, s_pVpManager->pBack, uwPosX, uwPosY, 32, 32, MINTERM_COOKIE);
    blitCopy(s_pVpManager->pBack, uwPosX, uwPosY, s_pFalconBg, 0, 0, 32, 32, MINTERM_COOKIE);                                                         // fragment tla wrzuca do zmiennej
    blitCopyMask(s_pTiles, pAnim[stonehitAnimFrame], 64 + falkonFace, s_pVpManager->pBack, uwPosX, uwPosY, 32, 32, (UWORD *)s_pTilesMask->Planes[0]); // rysuje falkona
  }
}

void prepareFalconFlying(void)
{
  uwPreviousX = uwPosX;
  uwPreviousY = uwPosY;
  newPosX = uwPosX;
  newPosY = uwPosY;

  switch (kierunekHold)
  {
  case 1:
    tempX = falkonx + 1;
    uwPosX = tempX * 32;
    break;

  case 2:
    tempX = falkonx - 1;
    uwPosX = tempX * 32;
    break;

  case 3:
    tempY = falkony - 1;
    uwPosY = tempY * 32;
    break;

  case 4:
    tempY = falkony + 1;
    uwPosY = tempY * 32;
    break;
  }
}

void endFalconFlying(void)
{
  blitCopy(s_pBg, newPosX, newPosY, s_pBgWithTile, newPosX, newPosY, 32, 32, MINTERM_COOKIE);

  switch (kierunekHold)
  {
  case 1:
    falkonx = falkonx + 1;
    break;

  case 2:
    falkonx = falkonx - 1;
    break;

  case 3:
    falkony = falkony - 1;
    break;

  case 4:
    falkony = falkony + 1;
    break;
  }
  // blitCopy(s_pBg, falkonx * 32, falkony * 32, s_pVpManager->pBack, falkonx * 32, falkony * 32, 32, 32, MINTERM_COOKIE);
}

void blitFlyingAnimFrame(void)
{

  blitCopy(s_pBg, uwPreviousX, uwPreviousY, s_pVpManager->pBack, uwPreviousX, uwPreviousY, 32, 32, MINTERM_COOKIE);
  if (kamyki[tempX][tempY] > 3)
  {
    blitCopy(s_pBgWithTile, newPosX, newPosY, s_pVpManager->pBack, newPosX, newPosY, 32, 32, MINTERM_COOKIE);
  }
  else if (kamyki[tempX][tempY] < 4)
  {
    blitCopy(s_pBg, newPosX, newPosY, s_pVpManager->pBack, newPosX, newPosY, 32, 32, MINTERM_COOKIE);
  }
  blitCopyMask(s_pTiles, pAnim[flyingFrame], 64 + falkonFace, s_pVpManager->pBack, newPosX, newPosY, 32, 32, (UWORD *)s_pTilesMask->Planes[0]);
}

void falkonFlying(void)
{
  if (flyingAnimControl == 0)
  {
    return;
  }

  if (flyingAnimControl == 1)
  {
    falkonIdleControl = 0;
    flyingAnimControl = 3;

    switch (kierunekHold)
    {
    case 1:
      newPosX += 1;
      break;
    case 2:
      newPosX -= 1;
      break;
    case 3:
      newPosY -= 1;
      break;
    case 4:
      newPosY += 1;
      break;
    }

    if (flyingTick == 4)
    {
      flyingFrame = 0;
    }
    else if (flyingTick == 8 || flyingTick == 12 || flyingTick == 16 || flyingTick == 20 ||
             flyingTick == 24 || flyingTick == 28 || flyingTick == 32)
    {
      ++flyingFrame;
    }

    blitFlyingAnimFrame();

    if (flyingTick == 32)
    {
      flyingTick = 0;
      flyingAnimControl = 2;
      falkonIdleControl = 1;
    }
  }

  else if (flyingAnimControl == 2)
  {
    blitCopy(s_pBg, uwPreviousX, uwPreviousY, s_pVpManager->pBack, uwPreviousX, uwPreviousY, 32, 32, MINTERM_COOKIE);

    endFalconFlying();
    doubleBufferFrameControl = 2;
    coalAndCollect();
    flyingAnimControl = 0;
    falkonIdleControl = 1;
  }

  if (robboMsgCtrl == 3)
  {
    robboMsgCtrl = 2;
    hudScrollingControl = 1;
  }
}

void falkonFlying2Db(void)
{
  flyingAnimControl = 4;
  blitFlyingAnimFrame();
}

void falconCollisionCheck(void)
{

  // jesli byl kamien to brak ruchu
  if (stoneHit == 1)
  {
    stonehitAnimControl = 1;
    falkonIdleControl = 0;

    switch (kierunek)
    {
    case 1:
      krawedzx = krawedzx - 1;
      break;
    case 2:
      krawedzx = krawedzx + 1;
      break;
    case 3:
      krawedzy = krawedzy + 1;
      break;
    case 4:
      krawedzy = krawedzy - 1;
      break;
    }

    stoneHit = 0;
    return;
  }

  if (frameHit == 1)
  {
    stonehitAnimControl = 1;
    falkonIdleControl = 0;
    frameHit = 0;
    return;
  }
  prepareFalconFlying();
  flyingAnimControl = 1;
}

void falconIdleAnimation(void)
{
  if (falkonIdleControl != 1)
  {
    return;
  }

  if (kierunek != 0 && stoneHit == 0)
  {
    return;
  }

  if (falkonIdle == falkonIdleTempo * 1)
  {
    idleFrame = 0;
  }
  else if (falkonIdle == falkonIdleTempo * 2)
  {
    idleFrame = 1;
  }
  else if (falkonIdle == falkonIdleTempo * 3)
  {
    idleFrame = 2;
  }
  else if (falkonIdle == falkonIdleTempo * 4)
  {
    idleFrame = 3;
  }
  else if (falkonIdle == falkonIdleTempo * 5)
  {
    idleFrame = 4;
  }
  else if (falkonIdle == falkonIdleTempo * 6)
  {
    idleFrame = 5;
  }
  else if (falkonIdle == falkonIdleTempo * 7)
  {
    idleFrame = 6;
  }
  else if (falkonIdle >= falkonIdleTempo * 8)
  {
    idleFrame = 7;
    falkonIdle = 0;
  }
  blitCopy(s_pBg, uwPosX, uwPosY, s_pFalconBg, 0, 0, 32, 32, MINTERM_COOKIE);
  blitCopy(s_pFalconBg, 0, 0, s_pVpManager->pBack, uwPosX, uwPosY, 32, 32, MINTERM_COOKIE);
  blitCopyMask(s_pTiles, idleFrame * 32, 192 + falkonFace, s_pVpManager->pBack, falkonx * 32, falkony * 32, 32, 32, (UWORD *)s_pTilesMask->Planes[0]);
}

void redCapacitorsAnimation(void)
{
  UBYTE i = 0, k = 0;

  if (redCapacitorsAnimTick == tickTempo)
  {

    for (i = 0; i < 10; ++i)
    {
      for (k = 0; k < 7; ++k)
      {
        if (collectiblesAnim[i][k] == 9)
        {
          blitCopy(s_pBg, i * 32, k * 32, s_pAnimBg, 0, 0, 32, 32, MINTERM_COOKIE);
          blitCopy(s_pAnimBg, 0, 0, s_pVpManager->pBack, i * 32, k * 32, 32, 32, MINTERM_COOKIE);
          blitCopyMask(s_pTiles, redCapacitorsAnimTileCheck * 32, 288, s_pVpManager->pBack, i * 32, k * 32, 32, 32, (UWORD *)s_pTilesMask->Planes[0]);
        }
      }
    }
    ++redCapacitorsAnimTileCheck;
    if (redCapacitorsAnimTileCheck == 7)
    {
      redCapacitorsAnimTileCheck = 0;
    }
  }
}

void blueCapacitorsAnimation(void)
{

  if (blueCapacitorsAnimTick == tickTempo)
  {

    for (UBYTE i = 0; i < 10; ++i)
    {
      for (UBYTE k = 0; k < 7; ++k)
      {
        if (collectiblesAnim[i][k] == 8)
        {
          blitCopy(s_pBg, i * 32, k * 32, s_pAnimBg, 0, 0, 32, 32, MINTERM_COOKIE);
          blitCopy(s_pAnimBg, 0, 0, s_pVpManager->pBack, i * 32, k * 32, 32, 32, MINTERM_COOKIE);
          blitCopyMask(s_pTiles, blueCapacitorsAnimTileCheck * 32, 256, s_pVpManager->pBack, i * 32, k * 32, 32, 32, (UWORD *)s_pTilesMask->Planes[0]);
        }
      }
    }
    ++blueCapacitorsAnimTileCheck;
    if (blueCapacitorsAnimTileCheck == 7)
    {
      blueCapacitorsAnimTileCheck = 0;
    }
  }
}

void portalGlowAnim(void){
  blitCopy(s_pBg, portalGlowX * 32, portalGlowY * 32, s_pBgPortalGlow, 0, 0, 32, 32, MINTERM_COOKIE);
  blitCopyMask(s_pTiles, portalGlowFrame * 32, 352, s_pBgPortalGlow, 0, 0, 32, 32, (UWORD *)s_pTilesMask->Planes[0]);
  //blitCopyMask(s_pTiles, portalGlowFrame * 32, 352, s_pBgWithTile, x * 32, y * 32, 32, 32, (UWORD *)s_pTilesMask->Planes[0]);

  blitCopy(s_pBgPortalGlow, 0, 0, s_pVpManager->pBack, portalGlowX * 32, portalGlowY * 32, 32, 32, MINTERM_COPY);
  //blitCopy(s_pBgWithTile, 0, 0, s_pVpManager->pFront, portalGlowX * 32, portalGlowY * 32, 32, 32, MINTERM_COPY);

}

void gameOverCoalBlinkingOnHUD(void)
{
  flyingAnimControl = 5;
  for (UBYTE i = 0; i < 5; ++i)
  {
    blitCopy(s_pHUD, 32, 0, s_pVpManager->pBack, 32, 224, 32, 32, MINTERM_COOKIE);
    blitCopy(s_pHUD, 32, 0, s_pVpManager->pFront, 32, 224, 32, 32, MINTERM_COOKIE);
    sprintf(szMsg, "%d", coal);
    fontFillTextBitMap(s_pFont, s_pBmText, szMsg);
    fontDrawTextBitMap(s_pVpManager->pBack, s_pBmText, 42, 232, HUDfontColor, FONT_COOKIE);
    fontDrawTextBitMap(s_pVpManager->pFront, s_pBmText, 42, 232, HUDfontColor, FONT_COOKIE);
    waitFrames(s_pVp, 50, uwPosY + FALCON_HEIGHT);
    blitCopy(s_pHUD, 32, 0, s_pVpManager->pBack, 32, 224, 32, 32, MINTERM_COOKIE);
    blitCopy(s_pHUD, 32, 0, s_pVpManager->pFront, 32, 224, 32, 32, MINTERM_COOKIE);
    sprintf(szMsg, "%d", coal);
    fontFillTextBitMap(s_pFont, s_pBmText, szMsg);
    fontDrawTextBitMap(s_pVpManager->pBack, s_pBmText, 42, 232, 15, FONT_COOKIE);
    fontDrawTextBitMap(s_pVpManager->pFront, s_pBmText, 42, 232, 15, FONT_COOKIE);
    waitFrames(s_pVp, 50, uwPosY + FALCON_HEIGHT);
  }
}

void stateGameCreate(void)
{
  // Here goes your startup code
  //-------------------------------------------------------------- gdzie� w create
  s_pView = viewCreate(0,
                       TAG_VIEW_COPLIST_MODE, COPPER_MODE_BLOCK, // spos�b u�ywania coppera - ten jest najprostszy bo nic z nim nie musisz wi�cej robi�
                       TAG_VIEW_GLOBAL_CLUT, 1,                  // globalna paleta dla wszystkich viewport�w
                       TAG_END                                   // wi�cej argument�w nie ma
  );

  s_pVp = vPortCreate(0,
                      TAG_VPORT_VIEW, s_pView, // parent view
                      TAG_VPORT_BPP, 5,        // bits per pixel: 4bpp = 16col, 5pp = 32col, etc.
                      TAG_END);

  ptplayerCreate(1);
  s_pMod = ptplayerModCreate("data/mod.falkon");
  ptplayerLoadMod(s_pMod, 0, 0);

  // Paleta z falkona
  paletteLoad("data/falkon.plt", s_pVp->pPalette, 32);
  paletteLoad("data/falkon.plt", s_pPalette, 32);

  g_pCustom->color[0] = 0x0FFF; // zmie� kolor zero aktualnie u�ywanej palety na 15,15,15

  s_pTiles = bitmapCreateFromFile("data/tileset.bm", 0);          // z pliku tileset.bm, nie lokuj bitmapy w pami�ci FAST
  s_pTilesMask = bitmapCreateFromFile("data/tileset_mask.bm", 0); // z pliku tileset_mask.bm, nie lokuj bitmapy w pami�ci FAST
  s_pBg = bitmapCreateFromFile("data/tlo1.bm", 0);
  s_pBgWithTile = bitmapCreateFromFile("data/tlo1.bm", 0); // fragmenty tla do podstawiania po ruchu
  s_pHUD = bitmapCreateFromFile("data/HUD.bm", 0);
  s_pRobbo = bitmapCreateFromFile("data/falkon_robbo.bm", 0);

  s_pFalconBg = bitmapCreate(48, 32, 5, BMF_INTERLEAVED);
  s_pAnimBg = bitmapCreate(48, 32, 5, BMF_INTERLEAVED);
  s_pBgPortalGlow = bitmapCreate(48, 32, 5, BMF_INTERLEAVED);

  s_pFont = fontCreate("data/topaz.fnt");
  s_pBmText = fontCreateTextBitMap(300, s_pFont->uwHeight); // bitmapa robocza długa na 200px, wysoka na jedną linię tekstu

  // proste wy�wietlanie bitmapy na viewporcie
  s_pVpManager = simpleBufferCreate(0,
                                    TAG_SIMPLEBUFFER_VPORT, s_pVp,                              // parent viewport
                                    TAG_SIMPLEBUFFER_BITMAP_FLAGS, BMF_CLEAR | BMF_INTERLEAVED, // wst�pne czyszczenie bitmapy, przyspieszenie rysowania grafiki
                                    TAG_SIMPLEBUFFER_IS_DBLBUF, 1,                              // nie potrzebujemy double buffering
                                    TAG_SIMPLEBUFFER_BOUND_WIDTH, 320 + 16,
                                    TAG_END);

  cameraSetCoord(s_pVpManager->pCamera, 0, 0);

  // po zrobieniu simpleBufferCreate()
  bitmapLoadFromFile(s_pVpManager->pBack, "data/tlo1.bm", 0, 0); // wczytaj zawarto�� bg1.bm bezpo�rednio do bitmapy bufora ekranu, zaczynaj�c od pozycji 0,0
  bitmapLoadFromFile(s_pVpManager->pFront, "data/tlo1.bm", 0, 0);
  blitCopy(s_pHUD, 0, 0, s_pVpManager->pBack, 0, 224, 320, 32, MINTERM_COOKIE);
  blitCopy(s_pHUD, 0, 0, s_pVpManager->pFront, 0, 224, 320, 32, MINTERM_COOKIE);
  joyOpen(); // b�dziemy u�ywa� d�oja w grze
  keyCreate();
  // na koniec create:

  randInit(1337);

  printOnHUD();
  doubleBufferFrameControl = 2;

  viewProcessManagers(s_pView);
  viewLoad(s_pView);

  blitCopy(s_pBg, 0, 0, s_pFalconBg, 0, 0, 48, 32, MINTERM_COOKIE);
  blitCopy(s_pFalconBg, 0, 0, s_pVpManager->pBack, 0, 0, 33, 32, MINTERM_COOKIE);

  drawTiles();
  ptplayerEnableMusic(1);
  systemUnuse(); // system w trakcie loop nie jest nam potrzebny
}

void stateGameLoop(void)
{
  // Here goes code done each game frame
  if (musicPlay == 1 && audioFadeIn < 64)
  {
    ++audioFadeIn;
    ptplayerSetMasterVolume(audioFadeIn);
  }

  if (portalGlowDB == 1)
  {
    portalGlowAnim();
    portalGlowDB = 0;
  }

  ++portalGlowTick;
  if (portalGlowTick > portalTickTempo)
  {
    ++portalGlowFrame;
    portalGlowAnim();
    portalGlowTick = 0;
    portalGlowDB = 1;
    if (portalGlowFrame == 7)
    {
      portalGlowFrame = 0;
    }
  }

  ++redCapacitorsAnimTick;
  if (redCapacitorsAnimTick > tickTempo)
  {
    redCapacitorsAnimTick = 0;
  }
  ++blueCapacitorsAnimTick;
  if (blueCapacitorsAnimTick > tickTempo)
  {
    blueCapacitorsAnimTick = 0;
  }

  if (falkonIdleControl == 1)
  {
    ++falkonIdle;
  }

  if (flyingAnimControl == 1)
  {
    ++flyingTick;
  }

  if (flyingAnimControl == 2)
  {
    falkonFlying();
    flyingAnimControl = 0;
    if (coal == 0)
    {
      gameOverCoalBlinkingOnHUD();
      portalAnimControl = 0;
      coal = 1;
      youWin = 2;

      clean();
      ptplayerStop();
      return;
    }
  }

  falconIdleAnimation();

  falkonHittingStone();
  redCapacitorsAnimation();
  blueCapacitorsAnimation();

  robboScrollUp();
  robboScrollDown();
  portalAnim();

  if (flyingAnimControl == 3)
  {
    falkonFlying2Db();
  }

  falkonFlying();

  if (flyingAnimControl == 4)
  {
    flyingAnimControl = 1;
  }

  if (hudScrollingControl == 1)
  {
    ++hudScrollingTick;
  }
  if (portalAnimControl == 1)
  {
    ++portalAnimTick;
  }
  if (stonehitAnimControl == 1)
  {
    ++stonehitAnimTick;
  }

  joyProcess();
  keyProcess();

  kierunek = 0;
  if (joyUse(JOY1_RIGHT) || keyUse(KEY_D) || keyUse(KEY_RIGHT))
  {
    kierunek = 1;
    falkonFace = 0;
  }
  else if (joyUse(JOY1_LEFT) || keyUse(KEY_A) || keyUse(KEY_LEFT))
  {
    kierunek = 2;
    falkonFace = 32;
  }
  else if (joyUse(JOY1_UP) || keyUse(KEY_W) || keyUse(KEY_UP))
  {
    kierunek = 3;
  }
  else if (joyUse(JOY1_DOWN) || keyUse(KEY_S) || keyUse(KEY_DOWN))
  {
    kierunek = 4;
  }
  else if (keyUse(KEY_ESCAPE))
  {
    ptplayerStop();
    clearTiles();
    clean();
    stateChange(g_pStateMachineGame, &g_sStateMenu);
    return;
  }
  else if (keyUse(KEY_N))
  {
    if (cheatmodeEnablerWhenEqual3 != 3)
    {
      return;
    }
    ++level;
    nextLevel();
    return;
  }
  else if (keyUse(KEY_M))
  {
    if (musicPlay == 1)
    {
      musicPlay = 0;
      ptplayerSetMasterVolume(0);
    }
    else if (musicPlay == 0)
    {
      musicPlay = 1;
      ptplayerSetMasterVolume(64);
    }
  }

  if (kierunek != 0)
  {
    if (flyingAnimControl != 0)
    {
      return;
    }
    kierunekHold = kierunek;

    isThisStone();
    czyRamka();
    falconCollisionCheck();
  }

  if (doubleBufferFrameControl > 0)
  {
    if (robboMsgCtrl == 0)
    {
      printOnHUD();
    }
    else if (robboMsgCtrl == 1)
    {
      robboScrollUp();
    }
    else if (robboMsgCtrl == 2)
    {
      robboScrollDown();
    }

    if (robboMsgCtrl == 3)
    {
      robboSays();
    }

    --doubleBufferFrameControl;
  }

  if (amigaMode == 1)
  {                // jesli zebralem ami-kondka to wyswietlam ekran z plot twistem
    amigaMode = 2; // ustawiam dla sprawdzenia na koniec czy bedzie alternatywne zakonczenie

    statePush(g_pStateMachineGame, &g_sStateGuruMastah);
    return;
  }

  if (youWin == 1) // sprawdzenie ktore zakonczenie uruchomic
  {
    youWin = 0;
    if (amigaMode == 0)
    {
      stateChange(g_pStateMachineGame, &g_sStateScore); // atari ending
      return;
    }
    else if (amigaMode == 2)
    {
      stateChange(g_pStateMachineGame, &g_sStateScoreAmi); // amiga ending
      return;
    }
  }
  else if (youWin == 2)
  {
    youWin = 0;
    stateChange(g_pStateMachineGame, &g_sStateGameOver);
    return;
  }

  viewProcessManagers(s_pView); // obliczenia niezb�dne do poprawnego dzia�ania viewport�w

  copProcessBlocks();     // obliczenia niezb�dne do poprawnego dzia�ania coppera
  vPortWaitForEnd(s_pVp); // r�wnowa�ne amosowemu wait vbl
}

void stateGameDestroy(void)
{
  // Here goes your cleanup code
  systemUse(); // w��cz grzecznie system
  ptplayerModDestroy(s_pMod);
  ptplayerDestroy();
  bitmapDestroy(s_pTiles);
  bitmapDestroy(s_pTilesMask);
  bitmapDestroy(s_pBg);
  bitmapDestroy(s_pBgWithTile);
  bitmapDestroy(s_pHUD);
  bitmapDestroy(s_pFalconBg);
  bitmapDestroy(s_pAnimBg);
  bitmapDestroy(s_pBgPortalGlow);
  bitmapDestroy(s_pRobbo);

  fontDestroy(s_pFont);
  fontDestroyTextBitMap(s_pBmText);

  viewDestroy(s_pView); // zwolnij z pami�ci view, wszystkie do��czone do niego viewporty i wszystkie do��czone do nich mened�ery
  joyClose();
  keyDestroy();
}

// pod funkcjami bo kompilator czyta od g�ry do do�u i musi najpierw napotka� funkcje by wiedzie� �e istniej�
tState g_sStateGame = {
    .cbCreate = stateGameCreate,
    .cbLoop = stateGameLoop,
    .cbDestroy = stateGameDestroy,
    .cbSuspend = 0,
    .cbResume = gameOnResume,
    .pPrev = 0};
