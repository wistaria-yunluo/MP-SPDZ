/*
 * Rep2kPrep.h
 *
 */

#ifndef PROTOCOLS_REPLICATEDPREP2K_H_
#define PROTOCOLS_REPLICATEDPREP2K_H_

#include "ReplicatedPrep.h"

template<class T>
class ReplicatedPrep2k : public ReplicatedRingPrep<T>
{
public:
    ReplicatedPrep2k(SubProcessor<T>* proc, DataPositions& usage) :
            BufferPrep<T>(usage), RingPrep<T>(proc, usage),
            ReplicatedRingPrep<T>(proc, usage)
    {
    }

    void get_dabit_no_count(T& a, typename T::bit_type& b)
    {
        this->get_one_no_count(DATA_BIT, a);
        b = a & 1;
    }
};

#endif /* PROTOCOLS_REPLICATEDPREP2K_H_ */
