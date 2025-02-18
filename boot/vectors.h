#ifndef VECTORS_H
#define VECTORS_H

void reset_handler(void);
void undefined_handler(void);
void svc_handler(void);
void prefetch_abort_handler(void);
void data_abort_handler(void);
void irq_handler(void);
void fiq_handler(void);

#endif  /* VECTORS_H */