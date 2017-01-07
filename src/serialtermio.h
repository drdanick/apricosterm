#ifndef SERIALTERMIO_H
#define SERIALTERMIO_H

void termioInit(char* pipeName);
int termioRead();
void termioClose();

#endif /* SERIALTERMIO_H */
