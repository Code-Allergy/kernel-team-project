#ifndef __CIRCULAR_BUFFER_H__
#define __CIRCULAR_BUFFER_H__

#include <types.h>

// Making the decision to make all char circular buffers 512 characters long
#define CHAR_BUFFER_SIZE 512
typedef struct _circular_char_buffer{
    char buffer[CHAR_BUFFER_SIZE];
    uint16_t head;
    uint16_t tail;
    uint16_t size;
} circular_char_buffer_t;

// Generic circular buffer
// Making the size 512 as well
#define GENERIC_BUFFER_SIZE 512
typedef struct _generic_circular_buffer{
    void* buffer[GENERIC_BUFFER_SIZE];
    uint16_t head;
    uint16_t tail;
    uint16_t size;
} generic_circular_buffer_t;

void circular_char_buffer_init(circular_char_buffer_t* buffer);
bool circular_char_buffer_push(circular_char_buffer_t* buffer, char c);
bool circular_char_buffer_pop(circular_char_buffer_t* buffer, char* c);

void generic_circular_buffer_init(generic_circular_buffer_t* buffer);
bool generic_circular_buffer_push(generic_circular_buffer_t* buffer, void* data);
bool generic_circular_buffer_pop(generic_circular_buffer_t* buffer, void** data);

#endif /* __CIRCULAR_BUFFER_H__ */