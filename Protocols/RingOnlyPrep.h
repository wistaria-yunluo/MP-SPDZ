/*
 * RingOnlyPrep.h
 *
 */

#ifndef PROTOCOLS_RINGONLYPREP_H_
#define PROTOCOLS_RINGONLYPREP_H_

#include "ReplicatedPrep.h"

template<class T>
class RingOnlyPrep : public virtual RingPrep<T>
{
protected:
    RingOnlyPrep<T>(SubProcessor<T>* proc, DataPositions& usage) :
            BufferPrep<T>(usage), RingPrep<T>(proc, usage)
    {
    }

    void buffer_dabits_from_bits_without_check(vector<dabit<T>>& dabits,
            int buffer_size, ThreadQueues* queues);
};

#endif /* PROTOCOLS_RINGONLYPREP_H_ */
