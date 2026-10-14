#ifndef SERIALTERMIO_H
#define SERIALTERMIO_H

void termioInit(char* pipeName);
int termioRead(char* buff, int size);
void termioClose();

#endif /* SERIALTERMIO_H */
