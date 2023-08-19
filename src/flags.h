#ifndef __FLAGS_H__
#define __FLAGS_H__

#define ON(allFlags, someFlags) (allFlags | someFlags)
#define OFF(allFlags, someFlags) (allFlags & ~someFlags)
#define FLIP(allFlags, someFlags) (allFlags ^ someFlags)

#define ISON(allFlags, someFlags) ((allFlags & someFlags) == someFlags)
#define ISOFF(allFlags, someFlags) ((allFlags & ~someFlags) == allFlags)

#endif