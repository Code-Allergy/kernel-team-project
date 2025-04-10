#pragma once

/* OOP is bad, proceeds to write OOP */
typedef struct
{
    /* constructors */
    int (*Ctor)(void*);
    int (*Mov)(void*);
    int (*Cpy)(void*);
    int (*Dtor)(void*);
    /* functionality */
    int (*Open)(void*);
    int (*Close)(void*);
    int (*Read)(void*);
    int (*Write)(void*);
    void (*InterruptHandler)(void*);
    void* (*IOCtrl)(void*);
} Driver;
