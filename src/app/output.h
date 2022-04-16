#ifndef _OUTPUT_H_
#define _OUTPUT_H_

#include "app.h"
#include "communication.h"

int onOutputBegin(TAppContext* appContext);

void onOutputResult(TAppContext* appContext, unsigned int workerId, const TWorkerResult* result, const char* filepath);

void onOutputEnd(TAppContext* appContext);

#endif