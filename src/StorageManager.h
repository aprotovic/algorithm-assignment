#ifndef STORAGEMANAGER_H
#define STORAGEMANAGER_H

#include "LinkedList.h"
#include "Models.h"
#include "Queue.h"

class StorageManager {
public:
    virtual ~StorageManager() {}
    virtual bool saveListings(LinkedList<Listing*>& list) = 0;
    virtual bool loadListings(LinkedList<Listing*>& list) = 0;
    virtual bool saveAccounts(LinkedList<Account*>& list) = 0;
    virtual bool loadAccounts(LinkedList<Account*>& list) = 0;
    virtual bool saveOrders(Queue<Inquiry*>& queue) = 0;
    virtual bool loadOrders(Queue<Inquiry*>& queue) = 0;
};

#endif
