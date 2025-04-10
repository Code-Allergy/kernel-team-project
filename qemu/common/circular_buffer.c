#include <circular_buffer.h>
#include <types.h>

void circular_char_buffer_init(circular_char_buffer_t* buffer)
{
    buffer->head  = 0;
    buffer->tail  = 0;
    buffer->size  = 0;
    buffer->lines = 0;
}

bool circular_char_buffer_push(circular_char_buffer_t* buffer, char c)
{
    if (buffer->size == CHAR_BUFFER_SIZE)
    {
        return false;
    }

    buffer->buffer[buffer->tail] = c;
    buffer->tail                 = (buffer->tail + 1) % CHAR_BUFFER_SIZE;
    buffer->size++;
    if (c == '\n')
    {
        buffer->lines++;
    }
    return true;
}

bool circular_char_buffer_pop(circular_char_buffer_t* buffer, char* c)
{
    if (buffer->size == 0)
    {
        return false;
    }

    *c           = buffer->buffer[buffer->head];
    buffer->head = (buffer->head + 1) % CHAR_BUFFER_SIZE;
    buffer->size--;
    if (*c == '\n')
    {
        buffer->lines--;
    }
    return true;
}

void generic_circular_buffer_init(generic_circular_buffer_t* buffer)
{
    buffer->head = 0;
    buffer->tail = 0;
    buffer->size = 0;
}

bool generic_circular_buffer_push(generic_circular_buffer_t* buffer, void* data)
{
    if (buffer->size == GENERIC_BUFFER_SIZE)
    {
        return false;
    }

    buffer->buffer[buffer->tail] = data;
    buffer->tail                 = (buffer->tail + 1) % GENERIC_BUFFER_SIZE;
    buffer->size++;
    return true;
}

bool generic_circular_buffer_pop(generic_circular_buffer_t* buffer, void** data)
{
    if (buffer->size == 0)
    {
        return false;
    }

    *data        = buffer->buffer[buffer->head];
    buffer->head = (buffer->head + 1) % GENERIC_BUFFER_SIZE;
    buffer->size--;
    return true;
}