/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef NXT_OTA_PACKAGE_H
#define NXT_OTA_PACKAGE_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define NXT_OTA_PACKAGE "vn.nguyenxuanthong.nxtretromusic"
#define NXT_OTA_DEVICE "trimui-brick-pro"
#if defined(NXT_ROM_STOCK) && (defined(NXT_ROM_NEXTUI) || defined(NXT_ROM_NXREDUX))
#error "Choose exactly one OTA ROM target"
#endif
#ifdef NXT_ROM_NXREDUX
#define NXT_OTA_ROM "nxredux"
#define NXT_OTA_DEFAULT_FILE "ota-source-nxredux.url"
#define NXT_OTA_DATA_DIR "ota/nxredux"
#define NXT_OTA_APP_BASENAME "NXT Retro Music.pak"
#define NXT_OTA_ZIP_PREFIX "Tools/NXT Retro Music.pak/"
#define NXT_OTA_BACKUP_NAME ".NXT_Retro_Music.previous.pak"
#elif defined(NXT_ROM_STOCK)
#define NXT_OTA_ROM "stock"
#define NXT_OTA_DEFAULT_FILE "ota-source-stock.url"
#define NXT_OTA_DATA_DIR "ota/stock"
#define NXT_OTA_APP_BASENAME "NXT_Retro_Music"
#define NXT_OTA_ZIP_PREFIX "Apps/NXT_Retro_Music/"
#define NXT_OTA_BACKUP_NAME ".NXT_Retro_Music.previous"
#else
/* The upstream Makefile builds NextUI without the Stock override. */
#define NXT_OTA_ROM "nextui"
#define NXT_OTA_DEFAULT_FILE "ota-source-nextui.url"
#define NXT_OTA_DATA_DIR "ota/nextui"
#define NXT_OTA_APP_BASENAME "NXT Retro Music.pak"
#define NXT_OTA_ZIP_PREFIX "Tools/tg5040/NXT Retro Music.pak/"
#define NXT_OTA_BACKUP_NAME ".NXT_Retro_Music.previous.pak"
#endif
typedef struct{char version[48],url[2048],sha256[65],notes[8192];uint64_t size;} NxtOTAManifest;
int NxtOTA_compareVersion(const char *,const char *);
bool NxtOTA_https(const char *);
bool NxtOTA_parseManifest(const char *,NxtOTAManifest *,char *,size_t);
bool NxtOTA_verifyZip(const char *,const NxtOTAManifest *,const char *,char *,size_t);
bool NxtOTA_install(const char *,const char *,char *,size_t);
void NxtOTA_removeTree(const char *);
#endif
