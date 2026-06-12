#ifndef RUNTIME_RESIDENT_DOS_H
#define RUNTIME_RESIDENT_DOS_H

#include "types.h"

int ResidentDosOpenRead(const char *path);
int ResidentDosOpenAppend(const char *path);
int ResidentDosCreate(const char *path);
u16 ResidentDosRead(u16 handle, void *buffer, u16 length);
u16 ResidentDosWrite(u16 handle, const void *buffer, u16 length);
void ResidentDosClose(u16 handle);

#endif
