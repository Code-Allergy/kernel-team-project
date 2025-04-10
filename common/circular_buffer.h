#ifndef __CIRCULAR_BUFFER_H__
#define __CIRCULAR_BUFFER_H__

#include <types.h>

// Making the decision to make all char circular buffers 512 characters long
#define CHAR_BUFFER_SIZE 512
typedef struct _circular_char_buffer
{
    char buffer[CHAR_BUFFER_SIZE];
    uint16_t head;
    uint16_t tail;
    uint16_t size;
    uint16_t lines;
} circular_char_buffer_t;

// Generic circular buffer
// Making the size 512 as well
#define GENERIC_BUFFER_SIZE 512
typedef struct _generic_circular_buffer
{
    void* buffer[GENERIC_BUFFER_SIZE];
    uint16_t head;
    uint16_t tail;
    uint16_t size;
} generic_circular_buffer_t;

/**
 * @brief Initializes a circular buffer for characters.
 *
 * @param buffer Pointer to the circular buffer to initialize.
 */
void circular_char_buffer_init(circular_char_buffer_t* buffer);

/**
 * @brief Pushes a character into the circular buffer.
 *
 * @param buffer Pointer to the circular buffer.
 * @param c Character to push into the buffer.
 * @return true if the character was successfully pushed, false if the buffer is
 * full.
 */
bool circular_char_buffer_push(circular_char_buffer_t* buffer, char c);

/**
 * @brief Pops a character from the circular buffer.
 *
 * @param buffer Pointer to the circular buffer.
 * @param c Pointer to the character variable where the popped character will be
 * stored.
 * @return true if the character was successfully popped, false if the buffer is
 * empty.
 */
bool circular_char_buffer_pop(circular_char_buffer_t* buffer, char* c);

/**
 * @brief Initializes a generic circular buffer.
 *
 * @param buffer Pointer to the generic circular buffer to initialize.
 */
void generic_circular_buffer_init(generic_circular_buffer_t* buffer);

/**
 * @brief Pushes data into the generic circular buffer.
 *
 * @param buffer Pointer to the generic circular buffer.
 * @param data Pointer to the data to push into the buffer.
 * @return true if the data was successfully pushed, false if the buffer is
 * full.
 */
bool generic_circular_buffer_push(generic_circular_buffer_t* buffer,
                                  void* data);

/**
 * @brief Pops data from the generic circular buffer.
 *
 * @param buffer Pointer to the generic circular buffer.
 * @param data Pointer to the variable where the popped data will be stored.
 * @return true if the data was successfully popped, false if the buffer is
 * empty.
 */
bool generic_circular_buffer_pop(generic_circular_buffer_t* buffer,
                                 void** data);

#endif /* __CIRCULAR_BUFFER_H__ */
