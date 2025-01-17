/*
 * ChaiGearPrep.h
 *
 */

#ifndef PROTOCOLS_CHAIGEARPREP_H_
#define PROTOCOLS_CHAIGEARPREP_H_

#include "FHEOffline/SimpleGenerator.h"

template<class T>
class ChaiGearPrep : public MaliciousRingPrep<T>
{
    typedef typename T::mac_key_type mac_key_type;
    typedef typename T::clear::FD FD;
    typedef SimpleGenerator<SummingEncCommit, FD> Generator;

    static MultiplicativeMachine* machine;
    static Lock lock;

    Generator* generator;
    SquareProducer<FD>* square_producer;
    InputProducer<FD>* input_producer;

    Generator& get_generator();

public:
    static void basic_setup(Player& P);
    static void key_setup(Player& P, mac_key_type alphai);
    static void teardown();

    ChaiGearPrep(SubProcessor<T>* proc, DataPositions& usage) :
            BufferPrep<T>(usage), RingPrep<T>(proc, usage),
            MaliciousRingPrep<T>(proc, usage), generator(0), square_producer(0),
            input_producer(0)
    {
    }
    ~ChaiGearPrep();

    void buffer_triples();
    void buffer_squares();
    void buffer_inverses();
    void buffer_bits();
    void buffer_inputs(int player);
};

#endif /* PROTOCOLS_CHAIGEARPREP_H_ */
