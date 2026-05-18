#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include "Models.h"
#include "LinkedList.h"

class Algorithms {
public:
    // Linear Search by Location
    static LinkedList<Listing*> searchByLocation(LinkedList<Listing*>& list, const string& location) {
        LinkedList<Listing*> results;
        for (int i = 0; i < list.getSize(); i++) {
            Listing** item = list.get(i);
            if (item && *item && (*item)->location.find(location) != string::npos) {
                results.add(*item);
            }
        }
        return results;
    }

    // Bubble Sort by Price
    static void bubbleSortByPrice(LinkedList<Listing*>& list) {
        int n = list.getSize();
        for (int i = 0; i < n - 1; i++) {
            for (int j = 0; j < n - i - 1; j++) {
                Listing** item1 = list.get(j);
                Listing** item2 = list.get(j + 1);
                if (item1 && item2 && *item1 && *item2) {
                    if ((*item1)->price > (*item2)->price) {
                        Listing* temp = *item1;
                        *item1 = *item2;
                        *item2 = temp;
                    }
                }
            }
        }
    }
};

#endif
