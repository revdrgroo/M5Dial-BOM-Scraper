#pragma once
#ifndef _MAIN_H_
#define _MAIN_H_

#include <LittleFS.h>
#include <M5Dial.h>
#include "BomParse.h"
#include "LocalBomParse.h"
#include "BomUI.h"

#define LITTLEFS_FILE_PATH "/test.xml"

bool connect_wifi();
#endif